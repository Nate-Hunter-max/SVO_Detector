/**
 * @file fsm_channel_search.c
 * @brief FSM for automatic channel search control using two buttons.
 */

#include "fsm.h"
#include "adc_pulse_freq.h"

extern FrequencyMeter_t freq;
extern TIM_HandleTypeDef htim1;

#define CHANNEL_DELAY 1000
#define FREQ_CH_MIN 10
#define FREQ_CH_MAX 20

#define IS_PRESS(GPIOx, GPIO_Pin)   ((GPIOx->IDR & GPIO_Pin) ? 0 : 1)

#define STOP_SEARCH() do { \
    HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_RESET); \
} while(0)

#define IS_CHANNEL_FOUND() (((uint8_t*)freq.frequency->data)[0] > FREQ_CH_MIN && \
                            ((uint8_t*)freq.frequency->data)[0] < FREQ_CH_MAX)

#define LED_ON()    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2)
#define LED_OFF()   HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2)

#define BUZZER_ON() HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1)
#define BUZZER_OFF() HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1)


// Enum of FSM states
typedef enum {
	IDLE, SEARCH_UP, SEARCH_DOWN, ALARM,
} State_t;

State_t currentState = IDLE;
State_t lastState = IDLE;

static inline void idle_state(void);
static inline void search_up_state(void);
static inline void search_down_state(void);

/**
 * @brief Initialize the FSM and ensure all outputs are off.
 */
void FSM_Init(void) {
	STOP_SEARCH();
	currentState = IDLE;
	lastState = IDLE;
}

/**
 * @brief IDLE state - waiting for button press.
 */
static inline void idle_state(void) {
	if (currentState != lastState) {
		lastState = currentState;
		STOP_SEARCH();
		while (IS_PRESS(BTN_P_GPIO_Port, BTN_P_Pin)||IS_PRESS(BTN_M_GPIO_Port, BTN_M_Pin)){};
	}

	if (IS_PRESS(BTN_P_GPIO_Port, BTN_P_Pin)) {
		currentState = SEARCH_UP;
	} else if (IS_PRESS(BTN_M_GPIO_Port, BTN_M_Pin)) {
		currentState = SEARCH_DOWN;
	}
}

/**
 * @brief SEARCH_UP state - send pulse every second to search forward.
 */
static inline void search_up_state(void) {
	static uint32_t lastTick = 0;
	if (currentState != lastState) {
		lastState = currentState;
		STOP_SEARCH();
	}

	// Generate 500 ms pulse every second
	if (HAL_GetTick() - lastTick >= CHANNEL_DELAY) {
		HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_RESET);
		lastTick = HAL_GetTick();
	}

	if (IS_CHANNEL_FOUND()) currentState = ALARM;
	else if (IS_PRESS(BTN_M_GPIO_Port, BTN_M_Pin)) currentState = IDLE;
}

/**
 * @brief SEARCH_DOWN state - send pulse every second to search backward.
 */
static inline void search_down_state(void) {
	static uint32_t lastTick = 0;
	if (currentState != lastState) {
		lastState = currentState;
		STOP_SEARCH();
	}

	// Generate 500 ms pulse every second
	if (HAL_GetTick() - lastTick >= CHANNEL_DELAY) {
		HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_RESET);
		lastTick = HAL_GetTick();
	}

	if (IS_CHANNEL_FOUND()) currentState = ALARM;
	else if (IS_PRESS(BTN_P_GPIO_Port, BTN_P_Pin)) currentState = IDLE;
}

/**
 * @brief ALARM state - enables buzzer & LED if channel found.
 */
static inline void alarm_state(void) {
	static uint32_t lastTick = 0;
	if (currentState != lastState) {
		lastState = currentState;
	}

	if (HAL_GetTick() - lastTick >= 100) {
		LED_ON();
		BUZZER_ON();
		HAL_Delay(50);
		LED_OFF();
		BUZZER_OFF();
		lastTick = HAL_GetTick();
	}

	if (IS_PRESS(BTN_P_GPIO_Port, BTN_P_Pin)||IS_PRESS(BTN_M_GPIO_Port, BTN_M_Pin))
		currentState = IDLE;
}

/**
 * @brief Main FSM process function.
 */
void FSM_Process(void) {
	switch (currentState) {
		case IDLE:
			idle_state();
			break;
		case SEARCH_UP:
			search_up_state();
			break;
		case SEARCH_DOWN:
			search_down_state();
			break;
		case ALARM:
			alarm_state();
			break;
	}
}
