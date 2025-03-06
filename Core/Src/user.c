#include "user.h"
#include "adc_pulse_freq.h"

extern FrequencyMeter_t freq_meter;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim3;

/**
 * @brief  Configure PWM frequency and duty cycle.
 * @param  htim Pointer to TIM handle.
 * @param  channel TIM channel (e.g., TIM_CHANNEL_1).
 * @param  frequency Desired PWM frequency in Hz.
 * @param  duty_cycle Duty cycle in percentage (0-100).
 * @retval HAL status
 */
HAL_StatusTypeDef PWM_SetFrequencyAndDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t frequency, float duty_cycle)
{
    uint32_t timer_clk = HAL_RCC_GetPCLK1Freq(); // Get timer clock frequency
    uint32_t prescaler = 0;
    uint32_t arr = 0;
    uint32_t ccr = 0;

    // Avoid zero division
    if (frequency == 0 || duty_cycle < 0.0f || duty_cycle > 100.0f)
    	return HAL_ERROR;


    // Calculate the best prescaler and ARR to achieve the desired frequency
    prescaler = (timer_clk / (frequency * 65536)) + 1;
    arr = (timer_clk / (frequency * prescaler)) - 1;

    // Calculate CCR value for duty cycle
    ccr = (uint32_t)((arr + 1) * (duty_cycle / 100.0f));

    // Stop PWM before changing values
    HAL_TIM_PWM_Stop(htim, channel);

    // Apply new prescaler and ARR
    htim->Instance->PSC = prescaler - 1;
    htim->Instance->ARR = arr;

    // Apply new CCR value
    switch (channel)
    {
        case TIM_CHANNEL_1: htim->Instance->CCR1 = ccr; break;
        case TIM_CHANNEL_2: htim->Instance->CCR2 = ccr; break;
        case TIM_CHANNEL_3: htim->Instance->CCR3 = ccr; break;
        case TIM_CHANNEL_4: htim->Instance->CCR4 = ccr; break;
        default: return HAL_ERROR;
    }

    // Reload registers
    __HAL_TIM_SET_COUNTER(htim, 0);

    // Restart PWM
    return HAL_TIM_PWM_Start(htim, channel);
}



void USER_Init() {
	FREQ_InitAll();
	PWM_SetFrequencyAndDuty(&htim14, TIM_CHANNEL_1, 2730, 50.0);
}

void USER_Loop() {
	if (!(BTN_M_GPIO_Port->IDR & BTN_M_Pin))
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);

	if (!(BTN_P_GPIO_Port->IDR & BTN_P_Pin))
		HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
	else
		HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);
}

