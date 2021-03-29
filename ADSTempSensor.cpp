/*
 * ADSTempSensor.cpp
 *
 *  Created on: Mar 29, 2021
 *      Author: tsugua
 */

#include "ADSTempSensor.h"

ADS1115_WE adc(ADS_ADDR);

ADSTempSensor::ADSTempSensor() {
	boilerTempSensorError = false;
	boilerTemp = -1;
	lastUpdateTime = 0;
	state = IDLE;
	boilerVoltage = 0;
	supplyVoltage = 0;
}

ADSTempSensor::~ADSTempSensor() {
}


void ADSTempSensor::init(){
	pinMode(ADS_RDY, INPUT_PULLUP);
	if(!adc.init()){
	    Serial.println("ADS1115 not connected!");
	    boilerTempSensorError = true;
	}
	adc.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
	adc.setMeasureMode(ADS1115_SINGLE);
	adc.setAlertPol(ADS1115_ACT_LOW);
	adc.setAlertPinToConversionReady();
}

void ADSTempSensor::update(){
	if(millis() >= lastUpdateTime + ADS_SAMPLE_INTERVAL){
		switch (state){
		case IDLE:
			adc.setVoltageRange_mV(ADS_BOILER_MAX_VOLT);
			adc.setCompareChannels(ADS_BOILER_INPUT);
			adc.startSingleMeasurement();
			state = measureBoiler;
			break;
		case measureBoiler:
			if(!digitalRead(ADS_RDY)){
				boilerVoltage = adc.getResult_mV();
				adc.setVoltageRange_mV(ADS_VCC_MAX_VOLT);
				adc.setCompareChannels(ADS_VCC_INPUT);
				adc.startSingleMeasurement();
				state = measureVCC;
			}
			break;
		case measureVCC:
			if(!digitalRead(ADS_RDY)){
				supplyVoltage = adc.getResult_mV();
				calculateBoilerTemp();
				state = IDLE;
				lastUpdateTime = millis();
			}
			break;
		}
	}
	checkBoilerTempSensorForError();
}

double ADSTempSensor::getBoilerTemp(){
	return boilerTemp;
}

bool ADSTempSensor::getBoilerTempSensorError(){
	return boilerTempSensorError;
}

void ADSTempSensor::calculateBoilerTemp(){
	double vcc = supplyVoltage * ADS_VCC_CONV_FACTOR;
	double boilerR = ADS_BOILER_REF_R * boilerVoltage / (vcc - boilerVoltage);
	if(boilerR <= 0){
		boilerTemp = -2;
		return;
	}
	// 1/T = 1/TN + 1/B*ln(RT/RN) with
	double oneOverT = (1/ADS_BOILER_TN + 1/(double)ADS_BOILER_B*log(boilerR/ADS_BOILER_RN));
	if(oneOverT == 0){
		boilerTemp = -3;
		return;
	}
	boilerTemp = 1/oneOverT-ADS_T0;
}

void ADSTempSensor::checkBoilerTempSensorForError(){
	bool error = false;
	static long lastErrorFoundTime = 0;
	if(millis() > lastErrorFoundTime + ADS_PLAUSIBILITY_STAY_IN_ERROR_TIME){
		boilerTempSensorError = false;
	}
	if(millis() >= lastUpdateTime + ADS_MIN_UPDATE_INTERVAL)
		error = true;
	if(boilerTemp > ADS_PLAUSIBILITY_MAX_BOILER_TEMP)
		error = true;
	if(boilerTemp < ADS_PLAUSIBILITY_MIN_BOILER_TEMP)
		error = true;
	static double lastBoilerTemp = -1;
	if(lastBoilerTemp != -1 && abs(boilerTemp-lastBoilerTemp) > ADS_PLAUSIBILITY_MAX_BOILER_TEMP_JUMP)
		error = true;
	lastBoilerTemp = boilerTemp;
	if(error){
		lastErrorFoundTime = millis();
		boilerTempSensorError = true;
	}
}


