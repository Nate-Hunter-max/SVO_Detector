/**
 * @file fsm_channel_search.c
 * @brief FSM for automatic channel search control using two buttons, with debounce and pulse generation.
 */

#include "fsm.h"
#include "adc_pulse_freq.h"
#include <stdbool.h>

extern FrequencyMeter_t freq;
extern TIM_HandleTypeDef htim1;

/**
 * @defgroup SearchTiming Search Timing Parameters
 * @brief Timing parameters for channel search pulses
 * @{
 */
#define PULSE_PERIOD_MS      1000  ///< Time between search pulses in milliseconds
#define PULSE_DURATION_MS    500   ///< Duration of each search pulse in milliseconds
/** @} */

/**
 * @defgroup AlarmTiming Alarm Timing Parameters
 * @brief Timing parameters for alarm indication
 * @{
 */
#define ALARM_INTERVAL_MS    100   ///< Time between alarm pulses in milliseconds
#define ALARM_PULSE_MS       50    ///< Duration of each alarm pulse in milliseconds
/** @} */

/**
 * @defgroup FrequencySettings Frequency Channel Settings
 * @brief Parameters for frequency channel detection
 * @{
 */
#define FREQ_CH_MIN 14       ///< Minimum valid channel frequency value
#define FREQ_CH_MAX 18       ///< Maximum valid channel frequency value
#define FREQ_CH_THR 10       ///< Maximum allowed out-of-range samples before channel is considered invalid
/** @} */

/**
 * @defgroup ButtonSettings Button Settings
 * @brief Parameters for button debouncing
 * @{
 */
#define DEBOUNCE_TIME_MS     10    ///< Debounce time for button presses in milliseconds
/** @} */

/**
 * @defgroup HardwareControl Hardware Control Macros
 * @brief Macros for controlling hardware components
 * @{
 */
#define STOP_SEARCH() do { \
    HAL_GPIO_WritePin(CTRL_UP_GPIO_Port, CTRL_UP_Pin, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, GPIO_PIN_SET); \
} while(0) ///< Stops both search directions by setting control pins high

#define LED_ON()    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2)  ///< Turns LED on by starting PWM
#define LED_OFF()   HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2)   ///< Turns LED off by stopping PWM

#define BUZZER_ON() HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1)  ///< Turns buzzer on by starting complementary PWM
#define BUZZER_OFF() HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1)  ///< Turns buzzer off by stopping complementary PWM
/** @} */

/**
 * @brief Available FSM states.
 */
typedef enum {
    IDLE,         /**< System is idle, waiting for button press */
    SEARCH_UP,    /**< Searching upward direction */
    SEARCH_DOWN,  /**< Searching downward direction */
    ALARM         /**< Alarm state, channel found */
} State_t;

/**
 * @brief FSM runtime context.
 */
typedef struct {
    State_t current;            /**< Current FSM state */
    State_t last;               /**< Last processed FSM state */
    uint32_t pulseTick;         /**< Timestamp for pulse generation */
    bool pulseActive;           /**< Flag indicating active pulse */
    uint32_t alarmTick;         /**< Timestamp for alarm blinking */
    bool alarmOn;               /**< Alarm blinking state flag */
    bool waitForRelease;        /**< Prevents immediate state transition due to held button */
} FSM_Context_t;

static FSM_Context_t fsm = {0};

/**
 * @brief Debounced GPIO input checker.
 *
 * @param port GPIO port.
 * @param pin GPIO pin.
 * @return true if button is pressed and stable, false otherwise.
 */
static inline bool IsButtonPressed(GPIO_TypeDef *port, uint16_t pin) {
    if ((port->IDR & pin) == 0) {
        uint32_t t_start = HAL_GetTick();
        while ((HAL_GetTick() - t_start) < DEBOUNCE_TIME_MS) {
            if ((port->IDR & pin) != 0) return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief Checks if frequency buffer meets channel presence conditions.
 *
 * @param buffer Pointer to circular buffer.
 * @param min_val Minimum valid frequency.
 * @param max_val Maximum valid frequency.
 * @param threshold Max allowed number of out-of-range values.
 * @return true if channel is detected, false otherwise.
 */
bool CheckForChannel(const CircularBuffer *buffer, uint8_t min_val, uint8_t max_val, uint8_t threshold) {
    uint8_t out_of_range_count = 0;
    uint8_t *data = (uint8_t *)buffer->data;
    for (uint8_t i = 0; i < buffer->size; i++) {
        if (data[i] < min_val || data[i] > max_val) {
            out_of_range_count++;
            if (out_of_range_count > threshold) return false;
        }
    }
    return true;
}

/**
 * @brief Initializes FSM state and resets output controls.
 */
void FSM_Init(void) {
    STOP_SEARCH();
    fsm.current = IDLE;
    fsm.last = IDLE;
    fsm.pulseActive = false;
    fsm.alarmOn = false;
    fsm.waitForRelease = false;
}

/**
 * @brief IDLE state: wait for debounced button press.
 */
static void idle_state(void) {
    if (fsm.current != fsm.last) {
        fsm.last = fsm.current;
        STOP_SEARCH();
        fsm.waitForRelease = true;
    }

    // Hold in IDLE until all buttons are released
    if (fsm.waitForRelease) {
        if (!IsButtonPressed(BTN_P_GPIO_Port, BTN_P_Pin) &&
            !IsButtonPressed(BTN_M_GPIO_Port, BTN_M_Pin)) {
            fsm.waitForRelease = false;
        } else {
            return;
        }
    }

    // Start appropriate search on button press
    if (IsButtonPressed(BTN_P_GPIO_Port, BTN_P_Pin)) {
        fsm.current = SEARCH_UP;
    } else if (IsButtonPressed(BTN_M_GPIO_Port, BTN_M_Pin)) {
        fsm.current = SEARCH_DOWN;
    }
}

/**
 * @brief Common logic for SEARCH states with timed pulse and channel detection.
 *
 * @param port GPIO port to control.
 * @param pin GPIO pin to pulse.
 * @param opposite_pressed true if the opposite direction button is pressed.
 */
static void handle_search(GPIO_TypeDef* port, uint16_t pin, bool opposite_pressed) {
    uint32_t now = HAL_GetTick();

    if (!fsm.pulseActive && (now - fsm.pulseTick >= PULSE_PERIOD_MS)) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        fsm.pulseTick = now;
        fsm.pulseActive = true;
    }

    if (fsm.pulseActive && (now - fsm.pulseTick >= PULSE_DURATION_MS)) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        fsm.pulseActive = false;
    }

    if (CheckForChannel(freq.frequency, FREQ_CH_MIN, FREQ_CH_MAX, FREQ_CH_THR)) {
        fsm.current = ALARM;
    } else if (opposite_pressed) {
        fsm.current = IDLE;
    }
}

/**
 * @brief SEARCH_UP state: perform timed pulses and check for channel.
 */
static void search_up_state(void) {
    if (fsm.current != fsm.last) {
        fsm.last = fsm.current;
        STOP_SEARCH();
        fsm.pulseTick = HAL_GetTick();
        fsm.pulseActive = false;
    }

    handle_search(CTRL_UP_GPIO_Port, CTRL_UP_Pin, IsButtonPressed(BTN_M_GPIO_Port, BTN_M_Pin));
}

/**
 * @brief SEARCH_DOWN state: perform timed pulses and check for channel.
 */
static void search_down_state(void) {
    if (fsm.current != fsm.last) {
        fsm.last = fsm.current;
        STOP_SEARCH();
        fsm.pulseTick = HAL_GetTick();
        fsm.pulseActive = false;
    }

    handle_search(CTRL_DWN_GPIO_Port, CTRL_DWN_Pin, IsButtonPressed(BTN_P_GPIO_Port, BTN_P_Pin));
}

/**
 * @brief ALARM state: blink LED and buzzer until any button pressed.
 */
static void alarm_state(void) {
    uint32_t now = HAL_GetTick();

    if (fsm.current != fsm.last) {
        fsm.last = fsm.current;
        fsm.alarmTick = now;
        fsm.alarmOn = false;
        LED_OFF();
        BUZZER_OFF();
    }

    if (!fsm.alarmOn && (now - fsm.alarmTick >= ALARM_INTERVAL_MS)) {
        LED_ON();
        BUZZER_ON();
        fsm.alarmTick = now;
        fsm.alarmOn = true;
    } else if (fsm.alarmOn && (now - fsm.alarmTick >= ALARM_PULSE_MS)) {
        LED_OFF();
        BUZZER_OFF();
        fsm.alarmOn = false;
    }

    if (IsButtonPressed(BTN_P_GPIO_Port, BTN_P_Pin) || IsButtonPressed(BTN_M_GPIO_Port, BTN_M_Pin)) {
        fsm.current = IDLE;
    }
}

/**
 * @brief Main FSM step function. Should be called periodically.
 */
void FSM_Process(void) {
    switch (fsm.current) {
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
