#include "user.h"
#include "adc_pulse_freq.h"
#include "CircularBuffer.h"
#include "fsm.h"
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc;
FrequencyMeter_t freq;
CircularBuffer cb;

void USER_Init() {
	cb.size=20;
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
	FSM_Init();
}

void USER_Loop() {
	FSM_Process();
}

