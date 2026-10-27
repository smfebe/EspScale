#include <math.h>
// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// user includes
#include "scale.h"

// HX711 Definitions
#define HX711_NODE DT_NODELABEL(hx711)
#define HX711_GAIN_128_NPULSES 1
#define HX711_GAIN_32_NPULSES 2
#define HX711_GAIN_64_NPULSES 3
#define HX711_CLOCK_DELAY_US 1

// logging
LOG_MODULE_REGISTER(scale, LOG_LEVEL_INF);

static struct scale_context ctx = 
{   // scale parameters
    .last_weight_raw = 0,
    .last_weight_dezi_grams = 0,
    .offset_raw = 0,
    .calibration_weight_raw = 0,
    .calibration_factor = 0.0f,
    // buffers
    .weight_dezi_grams = {0},
    .offset_raw_buffer = {0},
    .calibration_weight_raw_buffer = {0},
    // status
    .n_measurements = 0,
    .calculating_offset = false,
    .calibrated = false,
    .calibrating = false,
    // driver settings
    .gain_pulses = HX711_GAIN_128_NPULSES
};

static const struct gpio_dt_spec hx711_sck  = GPIO_DT_SPEC_GET(HX711_NODE, sck_gpios);
static const struct gpio_dt_spec hx711_dout = GPIO_DT_SPEC_GET(HX711_NODE, dout_gpios);
static struct k_work_delayable load_cell_work;



static void scale_calculate_offset(int32_t weight_raw) {
    static uint8_t offset_counter = 0;
    if (offset_counter < RAW_OFFSET_N_SAMPLES) {
        ctx.offset_raw_buffer[offset_counter] = weight_raw;
        offset_counter++;
    } else {
        int64_t sum = 0;
        for(uint8_t i = 0; i < RAW_OFFSET_N_SAMPLES; i++) {
            sum += ctx.offset_raw_buffer[i];
        }
        ctx.offset_raw = sum / RAW_OFFSET_N_SAMPLES;
        offset_counter = 0;
        memset(ctx.offset_raw_buffer, 0, sizeof(ctx.offset_raw_buffer));
        LOG_INF("Offset calculation complete. Offset: %d", ctx.offset_raw);
        ctx.calculating_offset = false;
    }
}

 static void scale_calculate_calibration(int32_t weight_raw) {
    static uint8_t calibration_counter = 0;
    if (calibration_counter < CALIBRATION_N_SAMPLES) {
        ctx.calibration_weight_raw_buffer[calibration_counter] = weight_raw;
        calibration_counter++;
    } else {
        int64_t sum = 0;
        for(uint8_t i = 0; i < CALIBRATION_N_SAMPLES; i++) {
            sum += ctx.calibration_weight_raw_buffer[i];
        }
        ctx.calibration_weight_raw = sum / CALIBRATION_N_SAMPLES;
        ctx.calibration_factor = (float)(CALIBRATION_WEIGHT_GRAMS * 10.0f) /(ctx.calibration_weight_raw - ctx.offset_raw);
        calibration_counter = 0;
        memset(ctx.calibration_weight_raw_buffer, 0, sizeof(ctx.calibration_weight_raw_buffer));
        LOG_INF("Calibration raw weight calculation complete. Raw weight: %d", ctx.calibration_weight_raw);
        ctx.calibrating = false;
        ctx.calibrated = true;
    }
}

static void scale_calculate_weight_dezigrams(int32_t weight_raw) {
    static uint8_t weight_buffer_index = 0;
    // ctx.last_weight_dezi_grams = (int32_t)(((weight_raw - ctx.offset_raw) * CALIBRATION_WEIGHT_GRAMS * 10.0f) / (ctx.calibration_weight_raw - ctx.offset_raw));
    ctx.last_weight_dezi_grams = (int32_t)(((weight_raw - ctx.offset_raw) * ctx.calibration_factor));

    ctx.weight_dezi_grams[weight_buffer_index] = ctx.last_weight_dezi_grams;
    weight_buffer_index = (weight_buffer_index + 1) % FILTER_SIZE;
}

static int32_t filter_weight(void) {
    int32_t sorted[FILTER_SIZE];
    memcpy(sorted, ctx.weight_dezi_grams, sizeof(ctx.weight_dezi_grams));

    for (int i = 1; i < FILTER_SIZE; i++) {
        int32_t key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    return sorted[FILTER_SIZE / 2];
}

int scale_read_raw(void) {
    if (gpio_pin_get_dt(&hx711_dout) != 0) {
        printk("Data not ready, DOUT is high\n");
        return -EBUSY; 
    }
    uint32_t raw_data = 0;
    unsigned int key = irq_lock();

    for (uint8_t i = 0; i < 24; i++) {
        gpio_pin_set_dt(&hx711_sck, 1);
        k_busy_wait(HX711_CLOCK_DELAY_US);
        uint8_t bit = gpio_pin_get_dt(&hx711_dout);
        raw_data = (raw_data << 1);
        if (bit > 0) {
            raw_data |= 1;
        }        
        gpio_pin_set_dt(&hx711_sck, 0);
        k_busy_wait(HX711_CLOCK_DELAY_US);
    }
    for (uint8_t i = 0; i < ctx.gain_pulses; i++) {
        gpio_pin_set_dt(&hx711_sck, 1);
        k_busy_wait(HX711_CLOCK_DELAY_US);
        gpio_pin_set_dt(&hx711_sck, 0);
        k_busy_wait(HX711_CLOCK_DELAY_US);
    }
    irq_unlock(key);

    if (raw_data & 0x800000) {
        raw_data |= 0xFF000000;
    }
    ctx.last_weight_raw = (int32_t)raw_data;
    ctx.n_measurements++;
    // calculate offset
    if (ctx.calculating_offset) {
        scale_calculate_offset(ctx.last_weight_raw);
    }
    // calibration
    if (ctx.calibrating) {
        scale_calculate_calibration(ctx.last_weight_raw);
    }
    // push weight dezi grams to buffer
    if (ctx.calibrated) {
        scale_calculate_weight_dezigrams(ctx.last_weight_raw);
    }

    return 0;  
} 




static void load_cell_work_handler(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    int ret = scale_read_raw();
    if (ret == 0) {
        /* Erfolgreich gelesen -> Nächste Messung in 1 Sekunde */
        k_work_schedule(dwork, K_MSEC(100));
    } else if (ret == -EBUSY) {
        /* Noch keine neuen Daten -> Schnell erneut prüfen (z. B. nach 20 ms) */
        k_work_schedule(dwork, K_MSEC(10));
    } else {
        LOG_ERR("Failed to read from HX711: %d\n", ret);
        k_work_schedule(dwork, K_MSEC(500));
    }
}

// API Functions
int scale_init(void) {
    if (!gpio_is_ready_dt(&hx711_sck) || !gpio_is_ready_dt(&hx711_dout)) {
        return -ENODEV;
    }
    if (gpio_pin_configure_dt(&hx711_sck, GPIO_OUTPUT_INACTIVE) < 0) {
        return -EIO;
    }
    if (gpio_pin_configure_dt(&hx711_dout, GPIO_INPUT) < 0) {
        return -EIO;
    }
    
    k_work_init_delayable(&load_cell_work, load_cell_work_handler);
    k_work_schedule(&load_cell_work, K_NO_WAIT);
    
    return 0;
}

void scale_calc_offset(void) {
    LOG_INF("Starting offset calculation with %d samples...", RAW_OFFSET_N_SAMPLES);
    ctx.calculating_offset = true;
}

void scale_calibrate(void) {
    LOG_INF("Starting calibration process with %d samples...", CALIBRATION_N_SAMPLES);
    ctx.calibrating = true;
}

int scale_set_gain(uint8_t gain_pulses) {
    if (gain_pulses < 1 || gain_pulses > 3) {
        return -EINVAL;
    }
    ctx.gain_pulses = gain_pulses;
    return 0;
}

int32_t scale_get_weight_dezi_grams(void) {
    if (!ctx.calibrated) {
        LOG_ERR("Scale is not calibrated. Weight measurement may be inaccurate.");
    }
    return filter_weight();
}


