/**
 * @file adc_pulse_freq.c
 * @brief Measure pulse frequency on ADC_IN0 with configurable threshold.
 *
 * This library uses ADC and Timer to measure the frequency of pulses
 * on ADC_IN0. A configurable threshold defines when the signal is considered high.
 */

#include "adc_pulse_freq.h"

/**
 * @brief Initialize ADC and Timer for pulse frequency measurement.
 *
 * @param dev Pointer to ADC frequency measurement device structure.
 */
void ADC_PulseFreq_Init(ADC_PulseFreq_t *dev) {

	// Configure ADC (single conversion mode, channel 0)
	dev->adc->CHSELR = dev->channel;
	dev->adc->CFGR1 &= ~ADC_CFGR1_CONT; // Single conversion mode
	dev->adc->SMPR |= ADC_SMPR_SMP_2;   // Sampling time selection
	dev->adc->CR |= ADC_CR_ADEN;        // Enable ADC
	while (!(ADC1->ISR & ADC_ISR_ADRDY))
		; // Wait for ADC ready

	// Configure Timer (input capture mode)
	dev->tim->PSC = SystemCoreClock / 1000000 - 1; // Prescaler for 1us time base
	dev->tim->ARR = 0xFFFF; // Max auto-reload value
	dev->tim->CCMR1 |= TIM_CCMR1_CC1S_0; // Set CH1 as input
	dev->tim->CCER |= TIM_CCER_CC1E; // Enable capture
	dev->tim->CR1 |= TIM_CR1_CEN; // Start timer
}

/**
 * @brief Measure pulse frequency.
 *
 * @param dev Pointer to ADC frequency measurement device structure.
 * @return Measured frequency in Hz.
 */
uint32_t ADC_PulseFreq_Measure(ADC_PulseFreq_t *dev) {
	uint32_t lastTime = 0, currentTime = 0;
	uint32_t pulseCount = 0;
	uint32_t timeout = 1000000; // Timeout counter

	while (pulseCount < 2 && timeout--) {
		// Start ADC conversion
		dev->adc->CR |= ADC_CR_ADSTART;
		while (!(dev->adc->ISR & ADC_ISR_EOC))
			; // Wait for conversion complete
		uint16_t adcValue = dev->adc->DR;

		// Check threshold condition
		if (adcValue > dev->threshold) {
			currentTime = dev->tim->CNT;
			if (pulseCount > 0) {
				dev->freq = 1000000 / (currentTime - lastTime); // Calculate frequency in Hz
				return dev->freq;
			}
			lastTime = currentTime;
			pulseCount++;
		}
	}
	dev->freq = 0;
	return dev->freq; // Return 0 if no valid measurement
}
