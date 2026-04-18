#include <hardware/pio.h>
#include <hardware/dma.h>
#include <pico/time.h>



// PIO program for counting pulses on pin 0 (relative)
static const uint16_t pulse_counter_program_instructions[] = {
    0x2020, // wait 0 gpio 0
    0x20a0, // wait 1 gpio 0
    0x8020, // push noblock
};

static const struct pio_program pulse_counter_program = {
    .instructions = pulse_counter_program_instructions,
    .length = 3,
    .origin = -1,
};

// Function to count rising edge pulses on a GPIO pin for a specified timeout in milliseconds
uint32_t count_pulses_pio(uint pin, uint32_t timeout_ms) {
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    if (sm == (uint)-1) return 0;

    uint offset = pio_add_program(pio, &pulse_counter_program);
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset, offset + pulse_counter_program.length - 1);
    sm_config_set_in_pins(&c, pin);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_sm_init(pio, sm, offset, &c);

    // DMA setup
    int dma_chan = dma_claim_unused_channel(true);
    if (dma_chan == -1) {
        pio_remove_program(pio, &pulse_counter_program, offset);
        pio_sm_unclaim(pio, sm);
        return 0;
    }
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, sm, false));
    static uint32_t dummy;
    dma_channel_configure(dma_chan, &dma_config, &dummy, &pio->txf[sm], 0x7FFFFFFF, false);

    // Start counting
    dma_channel_start(dma_chan);
    pio_sm_set_enabled(pio, sm, true);

    // Wait for timeout
    sleep_ms(timeout_ms);

    // Stop counting
    pio_sm_set_enabled(pio, sm, false);
    dma_channel_abort(dma_chan);

    // Calculate transferred pulses
    uint32_t transferred = 0x7FFFFFFF - dma_hw->ch[dma_chan].transfer_count;

    // Clean up
    dma_channel_unclaim(dma_chan);
    pio_remove_program(pio, &pulse_counter_program, offset);
    pio_sm_unclaim(pio, sm);

    return transferred;
}

// Global variables for asynchronous counting
PIO pio_global;
volatile uint sm_global;
volatile uint offset_global;
volatile int dma_chan_global;

void start_pulse_count(uint pin) {
    pio_global = pio0;
    sm_global = pio_claim_unused_sm(pio_global, true);
    offset_global = pio_add_program(pio_global, &pulse_counter_program);
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset_global, offset_global + pulse_counter_program.length - 1);
    sm_config_set_in_pins(&c, pin);
    pio_gpio_init(pio_global, pin);
    pio_sm_set_consecutive_pindirs(pio_global, sm_global, pin, 1, false);
    pio_sm_init(pio_global, sm_global, offset_global, &c);

    dma_chan_global = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan_global);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio_global, sm_global, false));
    static uint32_t dummy;
    dma_channel_configure(dma_chan_global, &dma_config, &dummy, &pio_global->txf[sm_global], 0x7FFFFFFF, false);

    dma_channel_start(dma_chan_global);
    pio_sm_set_enabled(pio_global, sm_global, true);
}

uint32_t stop_pulse_count() {
    pio_sm_set_enabled(pio_global, sm_global, false);
    dma_channel_abort(dma_chan_global);
    uint32_t transferred = 0x7FFFFFFF - dma_hw->ch[dma_chan_global].transfer_count;
    dma_channel_unclaim(dma_chan_global);
    pio_remove_program(pio_global, &pulse_counter_program, offset_global);
    pio_sm_unclaim(pio_global, sm_global);
    return transferred;
}

// Function to set up GPIO interrupt on INTERRUPT_PIN for rising edge
void setupPinInterrupt() {
    gpio_init(INTERRUPT_PIN);
    gpio_set_dir(INTERRUPT_PIN, GPIO_IN);
    gpio_pull_up(INTERRUPT_PIN);  // Assuming pull-up, adjust if needed
    
    gpio_set_irq_enabled_with_callback(INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, true, &pinInterruptISR);
}

// GPIO interrupt ISR triggered every 1 seconds by a pulse on INTERRUPT_PIN
void __isr pinInterruptISR(uint gpio, uint32_t events) {
    // Clear the interrupt
    gpio_acknowledge_irq(gpio, events);
    
    PPSinterruptPico();
}

void PPSinterruptPico()
{
  // Calibration function that counts SI5351 clock 2  for 10 seconds
  // Called on every pulse per second after gps sat lock
  if (CalibrationDone == true) return;
  tcount++;

  if (tcount == 4)  // Start counting the 2.5 MHz signal from Si5351A CLK2
  {
    start_pulse_count(COUNTER_PIN);
  }
  else if (tcount == 14)  //The 40 second counting time has elapsed - stop counting
  {     
    uint32_t total_pulses = stop_pulse_count();
    XtalFreq = total_pulses  // Actual crystal frequency
    correction = 25000000./(float)XtalFreq;
    // I found that adjusting the transmit freq gives a cleaner signal
    si5351_calibrate_off();
    tcount = 0;                              //Reset the seconds counter
    CalibrationDone = true;                  
  }
}