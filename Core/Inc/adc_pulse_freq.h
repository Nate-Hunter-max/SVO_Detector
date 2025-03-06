/**
 * @file adc_pulse_freq.h
 * @brief Header file for ADC pulse frequency measurement library.
 *
 * This library provides functions to measure the frequency of pulses on ADC_IN0.
 */

#ifndef ADC_PULSE_FREQ_H
#define ADC_PULSE_FREQ_H

#include "main.h"

/**
 * @brief Structure for frequency measurement.
 */
typedef struct {
    ADC_HandleTypeDef hadc;
    TIM_HandleTypeDef htim;
    uint32_t threshold_high;
    uint32_t threshold_low;
    uint32_t last_time;
    uint32_t frequency;
    uint8_t triggered;
    uint32_t timeout;

} FrequencyMeter_t;

void FREQ_InitAll(void);

#endif // ADC_PULSE_FREQ_H
