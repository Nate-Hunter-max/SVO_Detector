#include "adc_pulse_freq.h"

FrequencyMeter_t freq_meter;

/**
 * @brief Initializes the ADC for signal sampling.
 */
void ADC_Init(void) {
	__HAL_RCC_ADC1_CLK_ENABLE();

	freq_meter.hadc.Instance = ADC1;
	freq_meter.hadc.Init.Resolution = ADC_RESOLUTION_12B;
	freq_meter.hadc.Init.ContinuousConvMode = ENABLE;
	freq_meter.hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	freq_meter.hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	freq_meter.hadc.Init.ScanConvMode = DISABLE;

	HAL_ADC_Init(&freq_meter.hadc);

	ADC_ChannelConfTypeDef sConfig = { 0 };
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

	HAL_ADC_ConfigChannel(&freq_meter.hadc, &sConfig);
	HAL_ADC_Start_IT(&freq_meter.hadc);
}

/**
 * @brief Initializes the timer for timestamping edges.
 */
void TIM_Init(void) {
	__HAL_RCC_TIM3_CLK_ENABLE();
	freq_meter.htim.Instance = TIM3;
	freq_meter.htim.Init.Prescaler = (SystemCoreClock / 1000000) - 1; // 1 µs
	freq_meter.htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	freq_meter.htim.Init.Period = 0xFFFFFFFF;
	HAL_TIM_Base_Init(&freq_meter.htim);
	HAL_TIM_Base_Start(&freq_meter.htim);
}

/**
 * @brief ADC conversion complete callback with hysteresis.
 * @param hadc ADC handle pointer.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {
		uint32_t value = HAL_ADC_GetValue(hadc);
		uint32_t current_time = __HAL_TIM_GET_COUNTER(&freq_meter.htim);

		if (!freq_meter.triggered) {
			if (value >= freq_meter.threshold_high) {
				freq_meter.triggered = 1; // Фиксируем срабатывание

				if (freq_meter.last_time != 0) {
					freq_meter.frequency = 1000000 / (current_time - freq_meter.last_time);
				}
				freq_meter.last_time = current_time;
			}
		} else {
			if (value <= freq_meter.threshold_low) {
				freq_meter.triggered = 0; // Сброс триггера, ждем нового фронта
			}
		}

    // Проверяем таймаут (если слишком долго не было сигналов — сбрасываем частоту)
    if ((current_time - freq_meter.last_time) > freq_meter.timeout) {
        freq_meter.frequency = 0;
    }

		HAL_ADC_Start_IT(hadc);
	}
}

void FREQ_InitAll(void) {
	TIM_Init();
	ADC_Init();
	freq_meter.threshold_high = 3000;
	freq_meter.threshold_low = 2500;
	freq_meter.timeout  = 1e3;
}
