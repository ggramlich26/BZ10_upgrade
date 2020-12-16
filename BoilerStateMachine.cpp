/*
 * BoilerStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BoilerStateMachine.h"
#include "DataManager.h"

BoilerStateMachine::BoilerStateMachine() {
	state = disabled;
#ifdef QUICKSTART
	quickStart = true;
#else
	quickStart = false;
#endif
	dev = DeviceControl::instance();
	machStat = MachineStatusStateMachine::instance();
}

BoilerStateMachine::~BoilerStateMachine() {
}

void BoilerStateMachine::update(){
	switch (state){
	case disabled:
		//set outputs
		dev->disableBoilerHeater();
		//check transitions
		if(dev->getBoilerFull() && !machStat->inStandbye() &&
				!dev->getBoilerTempSensorError() && !dev->getBoilerFillSensorError()){
			state = enabled;
		}
		break;
	case enabled:
		//set outputs
		if(quickStart){
			dev->enableBoilerHeater(100);
			if(dev->getBoilerTemp() >= DataManager::getTargetTempBoiler()){
				quickStart = false;
			}
		}
		else{
			double diff = DataManager::getTargetTempBoiler() - dev->getBoilerTemp();
			if(diff <= 0){
				dev->enableBoilerHeater(0);
			}
			else{
				int heaterValue = DataManager::getBoilerControllerP()*diff;
				heaterValue = heaterValue>100?100:heaterValue;
				heaterValue = heaterValue<0?0:heaterValue;
				dev->enableBoilerHeater(heaterValue);
			}
		}
		//check transitions
		if(!dev->getBoilerFull() || machStat->inStandbye() ||
				dev->getBoilerFillSensorError() || dev->getBoilerTempSensorError()){
#ifdef QUICKSTART
			// reenable faststart after standbye
			if(machStat->inStandbye()){
				quickStart = true;
			}
#endif
			state = disabled;
		}
		break;
	}

}

