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


// Global variables for asynchronous counting
PIO pio_global;
volatile uint sm_global;
volatile uint offset_global;
volatile int dma_chan_global;
volatile uint32_t dma_initial_count_global = 0;
volatile int pps_mode_target = 0;
volatile int pps_mode_counter = 0;
volatile bool pulse_counter_initialized = false;
volatile bool start_count_request = false;
volatile bool stop_count_request = false;

// Initialize PIO+DMA for pulse counting on `pin`. This prepares SM and DMA but does not start them.
void init_pulse_counter(uint pin) {
    if (pulse_counter_initialized) return;
    pio_global = pio0;
    sm_global = pio_claim_unused_sm(pio_global, true);
    if (sm_global == (uint)-1) return;
    offset_global = pio_add_program(pio_global, &pulse_counter_program);
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset_global, offset_global + pulse_counter_program.length - 1);
    sm_config_set_in_pins(&c, pin);
    pio_gpio_init(pio_global, pin);
    pio_sm_set_consecutive_pindirs(pio_global, sm_global, pin, 1, false);
    pio_sm_init(pio_global, sm_global, offset_global, &c);

    dma_chan_global = dma_claim_unused_channel(true);
    if (dma_chan_global == -1) {
        pio_remove_program(pio_global, &pulse_counter_program, offset_global);
        pio_sm_unclaim(pio_global, sm_global);
        sm_global = (uint)-1;
        return;
    }
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan_global);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio_global, sm_global, false));
    static uint32_t dummy;
    // Use very large transfer count; we'll compute actual transfers via initial-remaining
    const uint32_t async_transfer_count = 0xFFFFFFFFu;
    dma_initial_count_global = async_transfer_count;
    dma_channel_configure(dma_chan_global, &dma_config, &dummy, &pio_global->rxf[sm_global], async_transfer_count, false);

    pulse_counter_initialized = true;
}

void start_pulse_count(uint pin) {
    if (!pulse_counter_initialized) init_pulse_counter(pin);
    if (!pulse_counter_initialized) return;
    // Reconfigure DMA each start to ensure transfer_count and addresses are set
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan_global);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio_global, sm_global, false));
    static uint32_t dummy;
    const uint32_t async_transfer_count = 0xFFFFFFFFu;
    dma_initial_count_global = async_transfer_count;
    dma_channel_configure(dma_chan_global, &dma_config, &dummy, &pio_global->rxf[sm_global], async_transfer_count, false);

    // Clear any prior RX FIFO content
    while (pio_sm_get_rx_fifo_level(pio_global, sm_global)) {
        (void)pio_sm_get(pio_global, sm_global);
    }

    // Start DMA and SM as quickly as possible
    dma_channel_start(dma_chan_global);
    pio_sm_set_enabled(pio_global, sm_global, true);
}

uint32_t stop_pulse_count() {
    pio_sm_set_enabled(pio_global, sm_global, false);
    // Read remaining transfers before aborting
    uint32_t remaining = dma_hw->ch[dma_chan_global].transfer_count;
    dma_channel_abort(dma_chan_global);
    uint32_t transferred = dma_initial_count_global - remaining;
    // Keep SM/program/DMA claimed so subsequent measurements start immediately
    return transferred;
}

// Read the current pulse count without stopping counting.
// Returns the number of pulses counted so far.
uint32_t read_pulse_count() {
    if (!pulse_counter_initialized || dma_chan_global == -1) return 0;
    uint32_t remaining = dma_hw->ch[dma_chan_global].transfer_count;
    uint32_t transferred = dma_initial_count_global - remaining;
    return transferred;
}

// Stop counting and perform cleanup (unclaim DMA, remove PIO program, unclaim SM).
// Returns the final pulse count.
uint32_t stop_and_cleanup_pulse_count() {
    if (!pulse_counter_initialized) return 0;
    // Disable SM
    pio_sm_set_enabled(pio_global, sm_global, false);
    // Read remaining before aborting
    uint32_t remaining = dma_hw->ch[dma_chan_global].transfer_count;
    dma_channel_abort(dma_chan_global);
    uint32_t transferred = dma_initial_count_global - remaining;
    // Cleanup
    dma_channel_unclaim(dma_chan_global);
    pio_remove_program(pio_global, &pulse_counter_program, offset_global);
    pio_sm_unclaim(pio_global, sm_global);
    // Reset state
    dma_chan_global = -1;
    sm_global = (uint)-1;
    offset_global = 0;
    dma_initial_count_global = 0;
    pulse_counter_initialized = false;
    return transferred;
}

// Disable the interrupt callback on INTERRUPT_PIN
void CalibrationOff() {
    // Disable rising-edge IRQ and clear the callback
    gpio_set_irq_enabled_with_callback(INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, false, NULL);
    pps_mode_counter = 0;
    stop_and_cleanup_pulse_count();
   
    si5351_calibrate_off();
}



void PPSinterruptPico()
{
  // Calibration function that counts SI5351 clock 2  for 10 seconds
  // Called on every pulse per second after gps sat lock
    if (CalibrationDone == true) return;
    tcount++;
    // Request start at first PPS after entry
    if (tcount == 1) {
        //start_count_request = true;
        //start_pulse_count(COUNTER_PIN);
        pps_mode_target = 0;
        pps_mode_counter = 0;
        return;
    }
        else if (tcount == 4)
        {
        //start_count_request = true;
        start_pulse_count(COUNTER_PIN);
        pps_mode_target = 20;
        pps_mode_counter = 0;
        //zeroCount = read_pulse_count();
        return;
        }

    // Track PPS counts and request stop when target reached
    if (pps_mode_target > 0) 
    {
        pps_mode_counter++;
        if (pps_mode_counter == pps_mode_target) 
        {
        uint32_t XtalFreq = stop_pulse_count() - zeroCount;
        if (XtalFreq != 0) 
            {
            correction = (2.*25000000.0f )/ (float)XtalFreq;
            SiCnt = (float)XtalFreq;
            pps_mode_counter = 0;
            CalibrationDone = true;
            }
        }
        
    }
}



// GPIO interrupt ISR triggered every 1 seconds by a pulse on INTERRUPT_PIN
void pinInterruptISR(uint gpio, uint32_t events) {
    // Clear the interrupt
    gpio_acknowledge_irq(gpio, events);
    
    PPSinterruptPico();
}

// Function to set up GPIO interrupt on INTERRUPT_PIN for rising edge
void setupPinInterrupt() {
    gpio_init(INTERRUPT_PIN);
    gpio_set_dir(INTERRUPT_PIN, GPIO_IN);
    gpio_pull_up(INTERRUPT_PIN);  // Assuming pull-up, adjust if needed
     tcount = 0;
    // Pre-initialize the pulse counter so starting it from the ISR is fast and safe
    init_pulse_counter(COUNTER_PIN);
    gpio_set_irq_enabled_with_callback(INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, true, &pinInterruptISR);
}


// Re-enable the interrupt callback on INTERRUPT_PIN
void attachPinInterrupt() {
    gpio_init(INTERRUPT_PIN);
    gpio_set_dir(INTERRUPT_PIN, GPIO_IN);
    gpio_pull_up(INTERRUPT_PIN);
    gpio_set_irq_enabled_with_callback(INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, true, &pinInterruptISR);
}
