#include <zephyr/drivers/i2c.h>
#include <ctype.h>
#include <string.h>

#include "display.h"
#include "alpha_num_font.h"

static int display_index_is_valid(const struct display_config *config, uint8_t index) {
    return index < (config->num_displays * DIGITS_PER_DISPLAY);
}

static void enable_osc(const struct display_config *config) {
    uint8_t msg = VK16K33_OSC_ON;
    int ret;

    for (size_t i = 0; i < config->num_displays; i++) {
        ret = i2c_write_dt(&config->display[i], &msg, 1);
        if (ret != 0) {
            printk("Failed to enable OSC on display %zu: %d\n", i, ret);
        }
    }
}

static void enable_display(const struct display_config *config) {
    uint8_t msg = VK16K33_DISPLAY_ON;
    int ret;

    for (size_t i = 0; i < config->num_displays; i++) {
        ret = i2c_write_dt(&config->display[i], &msg, 1);
        if (ret != 0) {
            printk("Failed to enable display %zu: %d\n", i, ret);
        }
    }
}

void display_init(const struct display_config *config) {
    if (config == NULL || config->display == NULL || config->display_ram == NULL) {
        printk("Display config is invalid\n");
        return;
    }

    for (size_t i = 0; i < config->num_displays; i++) {
        if (!device_is_ready(config->display[i].bus)) {
            printk("I2C bus %zu not ready\n", i);
            return;
        }
    }

    enable_osc(config);
    enable_display(config);
    display_clear(config);
    display_update(config);
}

void set_digits(const struct display_config *config, uint8_t start, size_t len, char *text) {
    uint8_t total_digits = config->num_displays * DIGITS_PER_DISPLAY;

    if (config == NULL || config->display_ram == NULL || text == NULL) {
        return;
    }

    if (start >= total_digits) {
        return;
    }

    if (len > (size_t)(total_digits - start)) {
        len = total_digits - start;
    }

    for (size_t index = 0; index < len; index++) {
        set_digit(config, (uint8_t)(start + index), text[index]);
    }
}

void display_clear(const struct display_config *config) {
    if (config == NULL || config->display_ram == NULL) {
        return;
    }

    memset(config->display_ram, 0, config->num_displays * VK16K33_RAM_SIZE);
}

void clear_digit(const struct display_config *config, uint8_t index) {
    if (config == NULL || config->display_ram == NULL || !display_index_is_valid(config, index)) {
        return;
    }

    uint8_t current_display = index / DIGITS_PER_DISPLAY;
    for (uint8_t j = 0; j < SEGMENTS_PER_DIGIT; j++) {
        config->display_ram[digit_map[j].byte + current_display * VK16K33_RAM_SIZE] &=
            ~(1u << (index % DIGITS_PER_DISPLAY + digit_map[j].offset));
    }
}

void set_digit(const struct display_config *config, uint8_t index, char character) {
    if (config == NULL || config->display_ram == NULL || !display_index_is_valid(config, index)) {
        return;
    }

    uint8_t c = (uint8_t)toupper((unsigned char)character);
    if (alpha_num_font[c] == 0 && c != ' ') {
        c = '-';
    }

    clear_digit(config, index);
    uint8_t current_display = index / DIGITS_PER_DISPLAY;
    for (uint8_t j = 0; j < SEGMENTS_PER_DIGIT; j++) {
        uint8_t display_byte = (alpha_num_font[c] >> j) & 0x01U;
        config->display_ram[digit_map[j].byte + current_display * VK16K33_RAM_SIZE] |=
            display_byte << (index % DIGITS_PER_DISPLAY + digit_map[j].offset);
    }
}

void display_update(const struct display_config *config) {
    if (config == NULL || config->display == NULL || config->display_ram == NULL) {
        return;
    }

    int ret = 0;
    uint8_t tx_buf[1 + VK16K33_RAM_SIZE];
    tx_buf[0] = VK16K33_RAM_START_ADDR;

    for (size_t n = 0; n < config->num_displays; n++) {
        memcpy(&tx_buf[1], &config->display_ram[n * VK16K33_RAM_SIZE], VK16K33_RAM_SIZE);
        ret = i2c_write_dt(&config->display[n], tx_buf, 1 + VK16K33_RAM_SIZE);
        if (ret != 0) {
            printk("Failed to update display %zu: %d\n", n, ret);
        }
    }
}