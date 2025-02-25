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
 * @brief ADC pulse frequency measurement structure.
 */
typedef struct {
	uint16_t threshold; /**< ADC threshold for pulse detection */
	TIM_TypeDef *tim;
	ADC_TypeDef *adc;
	uint32_t channel;
	uint32_t freq;
} ADC_PulseFreq_t;

/**
 * @brief Initialize ADC and Timer for pulse frequency measurement.
 *
 * @param dev Pointer to ADC frequency measurement device structure.
 */
void ADC_PulseFreq_Init(ADC_PulseFreq_t *dev);

/**
 * @brief Measure pulse frequency.
 *
 * @param dev Pointer to ADC frequency measurement device structure.
 * @return Measured frequency in Hz.
 */
uint32_t ADC_PulseFreq_Measure(ADC_PulseFreq_t *dev);

#endif // ADC_PULSE_FREQ_H
