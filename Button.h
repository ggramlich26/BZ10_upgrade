/*
 * Button.h
 *
 *  Created on: 23.12.2020
 *      Author: ag4716
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "Arduino.h"

#define BUTTON_DEBOUNCE_TIME	200
#define BUTTON_LONG_PRESS_TIME	3000
#define BUTTON_SAMPLE_PERIOD	20
#define BUTTON_NUMBER_SAMPLES	((int)(BUTTON_DEBOUNCE_TIME/BUTTON_SAMPLE_PERIOD))

class Button {
public:
	Button(uint8_t pin);
	Button(bool (*readButtonFunction)());
	virtual ~Button();
	void update();
	bool isPressed();
	bool isLongPressed();
	bool isShortPressed();
private:
	void init();
	uint8_t mPin;
	bool state;
	bool shortPressed;
	bool longPressed;
	bool lastState;
	unsigned long lastChangeTime;
	unsigned long longPressTime;
	bool readButton();
	bool (*externalReadButtonFunction)();
	bool samples[BUTTON_NUMBER_SAMPLES];
	int samplePointer;
	unsigned long lastSampleTime;
};

#endif /* BUTTON_H_ */
