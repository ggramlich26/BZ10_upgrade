/*
 * BoilerStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "MachineStatusStateMachine.h"
#include "DataManager.h"


MachineStatusStateMachine* MachineStatusStateMachine::_instance = NULL;

MachineStatusStateMachine::MachineStatusStateMachine() {
	state = running;
	lastUserActionTime = millis();
	wakeupTime = 0;	//todo: set through dataManager
	dev = DeviceControl::instance();
	standbyeStartTime = 90*60*1000;	//todo: set through dataManager
}

MachineStatusStateMachine::~MachineStatusStateMachine() {
}

void MachineStatusStateMachine::update(){
	if(dev->getButton1() || dev->getButton2() ||
			dev->getManualDistribution() || dev->getVolumetricDistribution()){
		lastUserActionTime = millis();
	}
	switch (state){
	case standbye:
		//set outputs
		dev->disableLEDPower();
		dev->disableLEDLeft();
		dev->disableLEDRight();
		//check transitions
		if(millis() < lastUserActionTime + standbyeStartTime ||
				(wakeupTime != 0 && (millis() > wakeupTime && millis() < wakeupTime + standbyeStartTime))){
			state = running;
			lastUserActionTime = millis();
		}
		break;
	case running:
		//set outputs
		dev->enableLEDPower();
		dev->enableLEDLeft();
		dev->enableLEDRight();
		//check transitions
		if(dev->getButton1LongPressed() || millis() > lastUserActionTime + standbyeStartTime){
			state = standbye;
		}
		break;
	}

}

bool MachineStatusStateMachine::inStandbye(){
	return standbye == state;
}
