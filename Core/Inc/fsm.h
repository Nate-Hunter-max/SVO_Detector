/**
 * @file fsm_channel_search.h
 * @brief FSM for automatic channel search control using two buttons (header).
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

#include "main.h"

/**
 * @brief Initialize the FSM and ensure all outputs are off.
 */
void FSM_Init(void);

/**
 * @brief Main FSM process function.
 */
void FSM_Process(void);

#endif /* INC_FSM_H_ */
