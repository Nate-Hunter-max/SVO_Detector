#include "user.h"
#include "adc_pulse_freq.h"
#include "CircularBuffer.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc;
FrequencyMeter_t freq;
CircularBuffer cb;

void USER_Init() {
	cb.size=5;
	cb.item_size = sizeof(uint8_t);
	CB_Init(&cb);

	freq.hadc=&hadc;
	freq.adcChannel = ADC_CHANNEL_0;
	freq.htim=&htim3;
	freq.threshold_high = 180;
	freq.threshold_low = 100;
	freq.frequency = &cb;
	freq._timeout = 1e3;
	FREQ_Init(&freq);
	FREQ_Start(&freq);
}
uint32_t val;
void USER_Loop() {
	val = ((uint32_t*)(cb.data))[0];
	if (!(BTN_M_GPIO_Port->IDR & BTN_M_Pin))
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);

	if (!(BTN_P_GPIO_Port->IDR & BTN_P_Pin))
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	else
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
}

