#ifndef SCALE_H_
#define SCALE_H_

#include <zephyr/kernel.h>

#define FILTER_SIZE 3
#define RAW_OFFSET_N_SAMPLES 20
#define CALIBRATION_N_SAMPLES 20
#define TARING_N_SAMPLES 20
#define CALIBRATION_WEIGHT_GRAMS 46

struct scale_context {
    // scale parameters
	int32_t last_weight_raw;
    int32_t last_weight_dezi_grams;
	int32_t offset_raw;
    int32_t calibration_weight_raw;
    int32_t tare_offset;
    float calibration_factor;
    // buffers
	int32_t weight_dezi_grams[FILTER_SIZE];
    int32_t offset_raw_buffer[RAW_OFFSET_N_SAMPLES];
    int32_t calibration_weight_raw_buffer[CALIBRATION_N_SAMPLES];
    int32_t taring_dezi_grams_buffer[TARING_N_SAMPLES];
    //status
	uint64_t n_measurements;
    bool calculating_offset;
    bool calibrated;
    bool calibrating;
    bool taring;
    // driver settings
    uint8_t gain_pulses;
};

int scale_init(void);
void scale_calc_offset(void);
void scale_calibrate(void);
void scale_tare(void);

int scale_set_gain(uint8_t gain_pulses);
int32_t scale_get_weight_dezi_grams(void);
    
#endif /* SCALE_H_ */