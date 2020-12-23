/*
 * Button.cpp
 *
 *  Created on: 23.12.2020
 *      Author: ag4716
 */

#include "Button.h"

Button::Button(uint8_t pin) {
	mPin = pin;
	externalReadButtonFunction = NULL;
	init();
}

Button::Button(bool (*readButtonFunction)()){
	mPin = 0xFF;
	externalReadButtonFunction = readButtonFunction;
	init();
}

Button::~Button() {
}

void Button::init(){
	state = false;
	longPressed = false;
	shortPressed = false;
	lastState = false;
	lastChangeTime = 0;
	longPressTime = BUTTON_LONG_PRESS_TIME;
	samplePointer = 0;
	for(int i = 0; i < BUTTON_NUMBER_SAMPLES; i++){
		samples[i] = false;
	}
	lastSampleTime = 0;
}

bool Button::readButton(){
	if(NULL != externalReadButtonFunction){
		return externalReadButtonFunction();
	}
	else{
		return digitalRead(mPin);
	}
}

void Button::update(){
	longPressed = false;
	shortPressed = false;
	if(millis() >= lastSampleTime){
		lastSampleTime = millis();
		samples[samplePointer] = readButton();
		samplePointer = (samplePointer+1)%BUTTON_NUMBER_SAMPLES;
		bool stateChange = true;
		for(int i = 0; i < BUTTON_NUMBER_SAMPLES; i++){
			if(samples[i] == state){
				stateChange = false;
				break;
			}
		}
		if(stateChange){
			Serial.println("State changed");
			//falling edge
			if(state){
				if(millis() >= lastChangeTime + longPressTime){
					longPressed = true;
				}
				else{
					shortPressed = true;
				}
			}
			state = !state;
			lastChangeTime = millis();
		}
	}
}

bool Button::isPressed(){
	return state;
}

bool Button::isLongPressed(){
	return longPressed;
}

bool Button::isShortPressed(){
	return shortPressed;
}
