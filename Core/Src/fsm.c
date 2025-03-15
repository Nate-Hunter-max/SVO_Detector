/**
 * @file fsm_channel_search.c
 * @brief FSM for automatic channel search control using two buttons.
 */

#include "fsm.h"
#include "adc_pulse_freq.h"
#include <stdbool.h>

extern FrequencyMeter_t freq;
extern TIM_HandleTypeDef htim1;

#define CHANNEL_DELAY 250
#define FREQ_CH_MIN 14
#define FREQ_CH_MAX 18
#define FREQ_CH_THR 10

#define IS_PRESS(GPIOx, GPIO_Pin)   ((GPIOx->IDR & GPIO_Pin) ? 0 : 1)

#define STOP_SEARCH() do { \
    HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_SET); \
} while(0)

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
 * @brief Checks if the number of out-of-range values in a circular buffer exceeds a given threshold.
 *
 * @param buffer Pointer to the circular buffer.
 * @param min_val Minimum acceptable value (inclusive).
 * @param max_val Maximum acceptable value (inclusive).
 * @param threshold Maximum allowed number of out-of-range elements.
 * @return bool Returns false if the number of out-of-range elements exceeds the threshold, true otherwise.
 */
bool CheckForChannel(const CircularBuffer *buffer, uint8_t min_val, uint8_t max_val, uint8_t threshold) {
    uint8_t out_of_range_count = 0;
    uint8_t *data = (uint8_t *)buffer->data;
    for (uint8_t i=0;i<buffer->size;i++){
        if (data[i] < min_val || data[i] > max_val) {
            out_of_range_count++;
            if (out_of_range_count > threshold) {
                return false;
            }
        }
    }

    return true;
}

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
		HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_SET);
		lastTick = HAL_GetTick();
	}

	if (CheckForChannel(freq.frequency,FREQ_CH_MIN,FREQ_CH_MAX,FREQ_CH_THR)) currentState = ALARM;
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
		HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_SET);
		lastTick = HAL_GetTick();
	}

	if (CheckForChannel(freq.frequency,FREQ_CH_MIN,FREQ_CH_MAX,FREQ_CH_THR)) currentState = ALARM;
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
