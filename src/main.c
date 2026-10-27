
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "scale.h"
#include "display.h"
#include "touch.h"

// logging
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

// Displays
#define NUM_DISPLAYS 2
#define DISPLAY1_NODE DT_NODELABEL(display1)
#define DISPLAY2_NODE DT_NODELABEL(display2)

static const struct i2c_dt_spec display_specs[] = {
    I2C_DT_SPEC_GET(DISPLAY1_NODE),
    I2C_DT_SPEC_GET(DISPLAY2_NODE),
};

static uint8_t display_ram[NUM_DISPLAYS * VK16K33_RAM_SIZE];

static const struct display_config my_display = {
    .display = display_specs,
    .num_displays = NUM_DISPLAYS,
    .display_ram = display_ram,
};

static struct k_work_delayable display_work;
uint8_t index = 0;
static void display_work_handler(struct k_work *work) {
    static int32_t last_weight_dezi_grams = 0;
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    char text[5] = {0};
    int32_t weight_dezi_grams = scale_get_weight_dezi_grams();
    snprintf(text, sizeof(text), "%04d", weight_dezi_grams);
    display_clear(&my_display);
    set_digits(&my_display, index, 4, text);
    // index = (index + 1) % (NUM_DISPLAYS * DIGITS_PER_DISPLAY);
    display_update(&my_display);
    last_weight_dezi_grams = weight_dezi_grams;
    k_work_schedule(dwork, K_MSEC(100));
}


int main(void)
{
    touch_init();
    // Initialize the display
    display_init(&my_display);
    k_msleep(1000);

    k_work_init_delayable(&display_work, display_work_handler);
    k_work_schedule(&display_work, K_NO_WAIT);
    
    scale_init();
    // scale_calc_offset();
    // k_msleep(3000);
    // LOG_INF("Start Calibration");
    // k_msleep(3000);
    // scale_calibrate();
    
    // k_msleep(1000);
    // printk("Hello World! %s\n", CONFIG_BOARD_TARGET);
    
    while(1) {
        int32_t weight = scale_get_weight_dezi_grams();
        printk("Weight: %ld decigrams\n", (long)weight);
        k_msleep(100);
    }
        
    return 0;
    
}


