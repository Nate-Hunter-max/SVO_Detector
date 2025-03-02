#include "user.h"
#include "adc_pulse_freq.h"

extern FrequencyMeter_t freq_meter;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

void USER_Init() {
	FREQ_InitAll();
}

void USER_Loop() {

	if (!(BTN_M_GPIO_Port->IDR & BTN_M_Pin))
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	else
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

	if (!(BTN_P_GPIO_Port->IDR & BTN_P_Pin))
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}

