#ifndef DISPLAY_H
#define DISPLAY_H

#include <zephyr/drivers/i2c.h>

#define VK16K33_OSC_ON          0x21
#define VK16K33_OSC_OFF         0x20
#define VK16K33_DISPLAY_ON      0x81
#define VK16K33_DISPLAY_OFF     0x80

#define VK16K33_RAM_START_ADDR  0x00
#define VK16K33_RAM_SIZE        16

#define DIGITS_PER_DISPLAY      4
#define SEGMENTS_PER_DIGIT      14

struct display_config {
    const struct i2c_dt_spec *display;
    size_t num_displays;
    uint8_t *display_ram;
};

void display_init(const struct display_config *config);
void display_clear(const struct display_config *config);
void set_digits(const struct display_config *config, uint8_t start, size_t len, char * text);
void display_update(const struct display_config *config);
void set_digit(const struct display_config *config, uint8_t index, char character);
void clear_digit(const struct display_config *config, uint8_t index);

#endif