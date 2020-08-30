/*
 * BUStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BUStateMachine.h"
#include "DataManager.h"

BUStateMachine::BUStateMachine() {
	state = enabled;
	dev = DeviceControl::instance();
	quickStart = true;
}

BUStateMachine::~BUStateMachine() {
}

void BUStateMachine::update(){
	switch (state){
	case enabled:
		//outputs
		if(quickStart){
			dev->enableBUHeater(100);
			if(dev->getBUTemp() >= DataManager::getTargetTempBU()){
				quickStart = false;
			}
		}
		else{
			double diff = DataManager::getTargetTempBU() - dev->getBUTemp();
			if(diff <= 0){
				dev->enableBUHeater(0);
			}
			else{	//calculate BU P controller value
				int heaterValue = DataManager::getBUControllerP() * diff;
				heaterValue = heaterValue>100?100:heaterValue;
				heaterValue = heaterValue<0?0:heaterValue;
				dev->enableBUHeater(heaterValue);
			}
		}
		//transitions
		//there are no transitions here sind BU is always turned on
		break;
	case disabled:
		//outputs
		dev->disableBUHeater();
		//transitions
		state = enabled;
		break;
	}
}

