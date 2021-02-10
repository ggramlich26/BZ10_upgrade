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
#ifdef BOILER_QUICKSTART
	quickStart = true;
#else
	quickStart = false;
#endif
	dev = DeviceControl::instance();
	machStat = MachineStatusStateMachine::instance();
	pid_input = dev->getBoilerTemp();
	pid_output = 0;
	pid_setpoint = DataManager::getTargetTempBoiler();
	boilerPID = new PID(&pid_input, &pid_output, &pid_setpoint, DataManager::getBoilerControllerP(),
			DataManager::getBoilerControllerI(), DataManager::getBoilerControllerD(), DIRECT);
	boilerPID->SetOutputLimits(0, 100);
	boilerPID->SetMode(AUTOMATIC);
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
		pid_input = dev->getBoilerTemp();
		pid_setpoint = DataManager::getTargetTempBoiler();
		boilerPID->SetTunings(DataManager::getBoilerControllerP(), DataManager::getBoilerControllerI(),
				DataManager::getBoilerControllerD());
		boilerPID->Compute();
		//set outputs
		if(quickStart){
			dev->enableBoilerHeater(100);
			if(dev->getBoilerTemp() >= DataManager::getTargetTempBoiler()){
				quickStart = false;
			}
		}
		else{
//			double diff = DataManager::getTargetTempBoiler() - dev->getBoilerTemp();
//			if(diff <= 0){
//				dev->enableBoilerHeater(0);
//			}
//			else{
//				int heaterValue = DataManager::getBoilerControllerP()*diff;
//				heaterValue = heaterValue>100?100:heaterValue;
//				heaterValue = heaterValue<0?0:heaterValue;
//				dev->enableBoilerHeater(heaterValue);
//			}
			dev->enableBoilerHeater(pid_output);
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

