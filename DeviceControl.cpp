/*
 * DeviceControl.cpp
 *
 *  Created on: Aug 23, 2020
 *      Author: tsugua
 */

#include "DeviceControl.h"

DeviceControl* DeviceControl::_instance = NULL;
TSIC* DeviceControl::tsicBoiler = NULL;
TSIC* DeviceControl::tsicBU = NULL;
TSIC* DeviceControl::tsicTube = NULL;


DeviceControl::DeviceControl() {
	init();
}

DeviceControl::~DeviceControl() {
}

/// initializes all pins and interrupts etc. necessary
void DeviceControl::init(){
	srData = 0x0000;
	pumpTickToVolumeFactor = 1;
	bypassTickToVolumeFactor = 1;
	pumpTicks = 0;
	bypassTicks = 0;
	pinMode(BUTTON_RIGHT_PIN, INPUT_PULLDOWN);
	pinMode(BUTTON_LEFT_PIN, INPUT_PULLDOWN);
	pinMode(BREW_MAN_PIN, INPUT_PULLDOWN);
	pinMode(BREW_VOL_PIN, INPUT_PULLDOWN);
	pinMode(TEMP_BOILER_PIN, INPUT);
	pinMode(TEMP_BU_PIN, INPUT);
	pinMode(TEMP_TUBE_PIN, INPUT);
	pinMode(FLOW_PUMP_PIN, INPUT);
	pinMode(FLOW_RET_PIN, INPUT);
	pinMode(SOUND_DIGITAL_PIN, INPUT);
	pinMode(SR_RCK, OUTPUT);
	pinMode(SR_SCK, OUTPUT);
	pinMode(SR_SER, OUTPUT);
	updateSR();
	tsicBoiler = new TSIC(TEMP_BOILER_PIN);
	tsicBU = new TSIC(TEMP_BU_PIN);
	tsicTube = new TSIC(TEMP_TUBE_PIN);
	attachInterrupt(TEMP_BOILER_PIN, tsicBoilerWrapper, CHANGE);
	attachInterrupt(TEMP_BU_PIN, tsicBUWrapper, CHANGE);
	attachInterrupt(TEMP_TUBE_PIN, tsicTubeWrapper, CHANGE);
}

/// update routine, that should be called regularly. Some values are only updated through this routine.
void DeviceControl::update(){
	//update boiler heater
	if(boilerLevel == 0){
		srData &= ~(1<<BOILER_HEATER);
	}
	else if(boilerLevel == 100){
		srData |= (1<<BOILER_HEATER);
	}
	else{
		if(millis() >= boilerPeriodStartTime + SSR_PERIOD_TIME){
			boilerPeriodStartTime = millis();
			srData |= (1<<BOILER_HEATER);
		}
		else if(millis() < boilerPeriodStartTime + boilerLevel/100.0*SSR_PERIOD_TIME){
			srData |= (1<<BOILER_HEATER);
		}
		else{
			srData &= ~(1<<BOILER_HEATER);
		}
	}
	//update BU heater
	if(BULevel == 0){
		srData &= ~(1<<BU_HEATER);
	}
	else if(BULevel == 100){
		srData |= (1<<BU_HEATER);
	}
	else{
		if(millis() >= BUPeriodStartTime + SSR_PERIOD_TIME){
			BUPeriodStartTime = millis();
			srData |= (1<<BU_HEATER);
		}
		else if(millis() < BUPeriodStartTime + BULevel/100.0*SSR_PERIOD_TIME){
			srData |= (1<<BU_HEATER);
		}
		else{
			srData &= ~(1<<BU_HEATER);
		}
	}
	updateSR();
}

/// enables the boiler heater
// @param level: heater level between 0 (off) and 100 (full power)
void DeviceControl::enableBoilerHeater(int level){
	if(level < 0) level = 0;
	else if (level > 100) level = 100;
	boilerLevel = level;
//	srData |= (1<<BOILER_HEATER);
//	updateSR();
}

void DeviceControl::disableBoilerHeater(){
	boilerLevel = 0;
	srData &= ~(1<<BOILER_HEATER);
	updateSR();
}

/// enables the brewing unit heater
// @param level: heater level between 0 (off) and 100 (full power)
void DeviceControl::enableBUHeater(int level){
	if(level < 0) level = 0;
	else if (level > 100) level = 100;
	BULevel = level;
//	srData |= (1<<BU_HEATER);
//	updateSR();
}

void DeviceControl::disableBUHeater(){
	BULevel = 0;
	srData &= ~(1<<BU_HEATER);
	updateSR();
}

void DeviceControl::enablePump(){
	srData |= (1<<PUMP);
	updateSR();
}

void DeviceControl::disablePump(){
	srData &= ~(1<<PUMP);
	updateSR();
}

//make water flow to brewing unit instead of sink
void DeviceControl::enableBrewingValve(){
	srData |= (1<<BU_VALVE);
	updateSR();
}

//make water flow to sink instead of brewing unit
void DeviceControl::disableBrewingValve(){
	srData &= ~(1<<BU_VALVE);
	updateSR();
}

//make water flow into boiler
void DeviceControl::enableBoilerValve(){
	srData |= (1<<BOILER_VALVE);
	updateSR();
}

//make water flow to brewing heater
void DeviceControl::disableBoilerValve(){
	srData &= ~(1<<BOILER_VALVE);
	updateSR();
}

void DeviceControl::enableLEDLeft(){
	srData |= (1<<LED_LEFT);
	updateSR();
}

void DeviceControl::disableLEDLeft(){
	srData &= ~(1<<LED_LEFT);
	updateSR();
}

void DeviceControl::enableLEDRight(){
	srData |= (1<<LED_RIGHT);
	updateSR();
}

void DeviceControl::disableLEDRight(){
	srData &= ~(1<<LED_RIGHT);
	updateSR();
}

/// enable the water tank empty indicator LED
void DeviceControl::enableLEDTank(){
	srData |= (1<<LED_POWER);
	updateSR();
}

/// disable the water tank empty indicator LED
void DeviceControl::disableLEDTank(){
	srData &= ~(1<<LED_TANK);
	updateSR();
}

/// enable the power LED
void DeviceControl::enableLEDPower(){
	srData |= (1<<LED_POWER);
	updateSR();
}

/// disable the power LED
void DeviceControl::disableLEDPower(){
	srData &= ~(1<<LED_POWER);
	updateSR();
}

bool DeviceControl::getBoilerFull(){
	return true; //todo delete
	return digitalRead(SOUND_DIGITAL_PIN);
}

bool DeviceControl::getTankFull(){
	//todo
	return true;
}

bool DeviceControl::getManualDistribution(){
	return digitalRead(BREW_MAN_PIN);
}

bool DeviceControl::getVolumetricDistribution(){
	return digitalRead(BREW_VOL_PIN);
	return false;
}

bool DeviceControl::getButton1(){
	return digitalRead(BUTTON_LEFT_PIN);
}

bool DeviceControl::getButton2(){
	return digitalRead(BUTTON_RIGHT_PIN);
}

double DeviceControl::getPumpVolume(){
	return pumpTicks * pumpTickToVolumeFactor;
}

double DeviceControl::getBypassVolume(){
	return bypassTicks * bypassTickToVolumeFactor;
}

double	DeviceControl::getBoilerTemp(){
	return tsicBoiler->getTemperature();
}

double DeviceControl::getBUTemp(){
	return tsicBU->getTemperature();
}

double DeviceControl::getTubeTemp(){
	return tsicTube->getTemperature();
}

void DeviceControl::updateSR(){
	digitalWrite(SR_RCK, LOW);
	shiftOut(SR_SER, SR_SCK, MSBFIRST, (srData >> 8));
	shiftOut(SR_SER, SR_SCK, MSBFIRST, srData);
	digitalWrite(SR_RCK, HIGH);
}
