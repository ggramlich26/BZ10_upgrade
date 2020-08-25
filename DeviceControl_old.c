/*
 * DeviceControl.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

//#include "DeviceControl.h"
//
//uint16_t srData;
//double pumpTickToVolumeFactor;
//double bypassTickToVolumeFactor;
//long pumpTicks;
//long bypassTicks;
//
//
//
//void dev_init(){
//	srData = 0x0000;
//	pumpTickToVolumeFactor = 1;
//	bypassTickToVolumeFactor = 1;
//	pumpTicks = 0;
//	bypassTicks = 0;
//	pinMode(BUTTON_RIGHT_PIN, INPUT_PULLDOWN);
//	pinMode(BUTTON_LEFT_PIN, INPUT_PULLDOWN);
//	pinMode(BREW_MAN_PIN, INPUT_PULLDOWN);
//	pinMode(BREW_VOL_PIN, INPUT_PULLDOWN);
//	pinMode(TEMP_BOILER_PIN, INPUT);
//	pinMode(TEMP_BU_PIN, INPUT);
//	pinMode(TEMP_TUBE_PIN, INPUT);
//	pinMode(FLOW_PUMP_PIN, INPUT);
//	pinMode(FLOW_RET_PIN, INPUT);
//	pinMode(SOUND_DIGITAL_PIN, INPUT);
//	pinMode(SR_RCK, OUTPUT);
//	pinMode(SR_SCK, OUTPUT);
//	pinMode(SR_SER, OUTPUT);
//	updateSR();
//}
//
//void enableBoilerHeater(){
//	srData |= (1<<BOILER_HEATER);
//	updateSR();
//}
//
//void disableBoilerHeater(){
//	srData &= ~(1<<BOILER_HEATER);
//	updateSR();
//}
//
//void enableBUHeater(){
//	srData |= (1<<BU_HEATER);
//	updateSR();
//}
//
//void disableBUHeater(){
//	srData &= ~(1<<BU_HEATER);
//	updateSR();
//}
//
//void enablePump(){
//	srData |= (1<<PUMP);
//	updateSR();
//}
//
//void disablePump(){
//	srData &= ~(1<<PUMP);
//	updateSR();
//}
//
////make water flow to brewing unit instead of sink
//void enableBrewingValve(){
//	srData |= (1<<BU_VALVE);
//	updateSR();
//}
//
////make water flow to sink instead of brewing unit
//void disableBrewingValve(){
//	srData &= ~(1<<BU_VALVE);
//	updateSR();
//}
//
////make water flow into boiler
//void enableBoilerValve(){
//	srData |= (1<<BOILER_VALVE);
//	updateSR();
//}
//
////make water flow to brewing heater
//void disableBoilerValve(){
//	srData &= ~(1<<BOILER_VALVE);
//	updateSR();
//}
//
//void enableLEDLeft(){
//	srData |= (1<<LED_LEFT);
//	updateSR();
//}
//
//void disableLEDLeft(){
//	srData &= ~(1<<LED_LEFT);
//	updateSR();
//}
//
//void enableLEDRight(){
//	srData |= (1<<LED_RIGHT);
//	updateSR();
//}
//
//void disableLEDRight(){
//	srData &= ~(1<<LED_RIGHT);
//	updateSR();
//}
//
//void enableLEDTank(){
//	srData |= (1<<LED_POWER);
//	updateSR();
//}
//
//void disableLEDTank(){
//	srData &= ~(1<<LED_TANK);
//	updateSR();
//}
//
//void enableLEDPower(){
//	srData |= (1<<LED_POWER);
//	updateSR();
//}
//
//void disableLEDPower(){
//	srData &= ~(1<<LED_POWER);
//	updateSR();
//}
//
//bool getBoilerFull(){
//	return true; //todo delete
//	return digitalRead(SOUND_DIGITAL_PIN);
//}
//
//bool getTankFull(){
//	//todo
//	return true;
//}
//
//bool getManualDistribution(){
//	return digitalRead(BREW_MAN_PIN);
//}
//
//bool getVolumetricDistribution(){
//	return digitalRead(BREW_VOL_PIN);
//	return false;
//}
//
//bool getButton1(){
//	return digitalRead(BUTTON_LEFT_PIN);
//}
//
//bool getButton2(){
//	return digitalRead(BUTTON_RIGHT_PIN);
//}
//
//double getPumpVolume(){
//	return pumpTicks * pumpTickToVolumeFactor;
//}
//
//double getBypassVolume(){
//	return bypassTicks * bypassTickToVolumeFactor;
//}
//
//double	getBoilerTemp(){
//	return 110.2;
//}
//
//double getBUTemp(){
//	return 94.3;
//}
//
//double getTubeTemp(){
//	return 89.0;
//}
//
//void updateSR(){
//	digitalWrite(SR_RCK, LOW);
//	shiftOut(SR_SER, SR_SCK, MSBFIRST, (srData >> 8));
//	shiftOut(SR_SER, SR_SCK, MSBFIRST, srData);
//	digitalWrite(SR_RCK, HIGH);
//}
//
//
