/**
 * @file adc_pulse_freq.h
 * @brief Header file for ADC pulse frequency measurement library.
 *
 * This library provides functions to measure the frequency of pulses on ADC_IN0.
 */

#ifndef ADC_PULSE_FREQ_H
#define ADC_PULSE_FREQ_H

#include "main.h"
#include "CircularBuffer.h"
/**
 * @brief Structure for frequency measurement.
 */
typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t adcChannel;
    TIM_HandleTypeDef* htim;
    uint8_t threshold_high;
    uint8_t threshold_low;
    CircularBuffer* frequency;
    uint32_t _last_time;
    uint8_t _triggered;
    uint32_t _timeout;

} FrequencyMeter_t;

void FREQ_Init(FrequencyMeter_t* freq_meter);

void FREQ_Start(FrequencyMeter_t* freq_meter);

void FREQ_Stop(FrequencyMeter_t* freq_meter);

#endif // ADC_PULSE_FREQ_H
