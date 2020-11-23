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
	dev = DeviceControl::instance();
}

MachineStatusStateMachine::~MachineStatusStateMachine() {
}

void MachineStatusStateMachine::update(){
	bool useraction = false;
	if(dev->getButton1ShortPressed() || dev->getButton1LongPressed() ||
			dev->getButton2ShortPressed() || dev->getButton2LongPressed() ||
			dev->getManualDistribution() || dev->getVolumetricDistribution()){
		lastUserActionTime = millis();
		useraction = true;
	}
	switch (state){
	case standby:
	{
		//set outputs
		dev->disableLEDPower();
		dev->disableLEDLeft();
		dev->disableLEDRight();
		//check transitions
		//wakeup automatically
		if((DataManager::getStandbyWakeupEnabled() &&
				(millis() > DataManager::getStandbyWakeupTime() &&
						(millis() < DataManager::getStandbyWakeupTime() + DataManager::getStandbyStartTime() ||
								DataManager::getStandbyStartTime() == 0)))){
			DataManager::incStandbyWakeupTimeByOneDay();
			state = running;
			lastUserActionTime = millis();
		}
		//wakeup by useraction
		else if(useraction){
			state = running;
			lastUserActionTime = millis();
		}
	}
		break;
	case running:
		//set outputs
		if(dev->getBoilerFull())
			dev->enableLEDLeft();
		else
			dev->disableLEDLeft();
		dev->enableLEDPower();
//		dev->enableLEDLeft();
		dev->enableLEDRight();
		//check transitions
		if(dev->getButton1ShortPressed() ||
				(millis() > lastUserActionTime + DataManager::getStandbyStartTime() &&
						DataManager::getStandbyStartTime() != 0)){
			state = standby;
		}
		break;
	}

}

bool MachineStatusStateMachine::inStandbye(){
	return standby == state;
}
