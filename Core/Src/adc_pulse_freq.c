#include "adc_pulse_freq.h"
#define SAMPLING_TIME ((uint32_t)1e5) // 10 µs
FrequencyMeter_t *_freq_meter;

/**
 * @brief Initializes the ADC & TIM for signal sampling.
 */
void FREQ_Init(FrequencyMeter_t *freq_meter) {
	freq_meter->hadc->Init.Resolution = ADC_RESOLUTION_8B;
	freq_meter->hadc->Init.ContinuousConvMode = ENABLE;
	freq_meter->hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
	freq_meter->hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	freq_meter->hadc->Init.ScanConvMode = DISABLE;
	HAL_ADC_Init(freq_meter->hadc);

	ADC_ChannelConfTypeDef sConfig = { 0 };
	sConfig.Channel = freq_meter->adcChannel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
	HAL_ADC_ConfigChannel(freq_meter->hadc, &sConfig);

	freq_meter->htim->Init.Prescaler = (SystemCoreClock / SAMPLING_TIME) - 1;
	freq_meter->htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	freq_meter->htim->Init.Period = 0xFFFFFFFF;
	HAL_TIM_Base_Init(freq_meter->htim);

	_freq_meter = freq_meter;
}

void FREQ_Start(FrequencyMeter_t *freq_meter) {
	HAL_ADC_Start_IT(freq_meter->hadc);
	HAL_TIM_Base_Start(freq_meter->htim);
}

void FREQ_Stop(FrequencyMeter_t *freq_meter) {
	HAL_ADC_Stop(freq_meter->hadc);
	HAL_TIM_Base_Stop(freq_meter->htim);
}

/**
 * @brief ADC conversion complete callback with hysteresis.
 * @param hadc ADC handle pointer.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {
		uint8_t value = hadc->Instance->DR;
		uint32_t current_time = __HAL_TIM_GET_COUNTER(_freq_meter->htim);

		if (!_freq_meter->_triggered) {
			if (value >= _freq_meter->threshold_high) {
				_freq_meter->_triggered = 1; // Фиксируем срабатывание

				if (_freq_meter->_last_time != 0) {
					value = SAMPLING_TIME / (current_time - _freq_meter->_last_time) / 1000; //In KHz
					CB_Add(_freq_meter->frequency, (void*) &value);

				}
				_freq_meter->_last_time = current_time;
			}
		} else {
			if (value <= _freq_meter->threshold_low) {
				_freq_meter->_triggered = 0; // Сброс триггера, ждем нового фронта
			}
		}

		if ((current_time - _freq_meter->_last_time) > _freq_meter->_timeout) {
			value = 0;
			//CB_Add(_freq_meter->frequency, (void*) &value);
		}

		HAL_ADC_Start_IT(hadc);
	}
}
