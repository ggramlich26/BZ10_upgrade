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
	machStat = MachineStatusStateMachine::instance();
#ifdef BU_QUICKSTART
	quickStart = true;
#else
	quickStart = false;
#endif
	pid_input = dev->getBUTemp();
	pid_output = 0;
	pid_setpoint = DataManager::getTargetTempBU();
	BUpid = new PID(&pid_input, &pid_output, &pid_setpoint, DataManager::getBUControllerP(),
			DataManager::getBUControllerI(), DataManager::getBUControllerD(), DIRECT);
	BUpid->SetOutputLimits(0, 100);
	BUpid->SetMode(AUTOMATIC);
}

BUStateMachine::~BUStateMachine() {
}

void BUStateMachine::update(){
	switch (state){
	case enabled:
		pid_input = dev->getBUTemp();
		pid_setpoint = DataManager::getTargetTempBU();
		BUpid->SetTunings(DataManager::getBUControllerP(), DataManager::getBUControllerI(),
				DataManager::getBUControllerD());
		BUpid->Compute();
		//outputs
		if(quickStart){
			dev->enableBUHeater(100);
			if(dev->getBUTemp() >= DataManager::getTargetTempBU()){
				quickStart = false;
			}
		}
		else{
//			double diff = DataManager::getTargetTempBU() - dev->getBUTemp();
//			if(diff <= 0){
//				dev->enableBUHeater(0);
//			}
//			else{	//calculate BU P controller value
//				int heaterValue = DataManager::getBUControllerP() * diff;
//				heaterValue = heaterValue>100?100:heaterValue;
//				heaterValue = heaterValue<0?0:heaterValue;
//				dev->enableBUHeater(heaterValue);
//			}
			dev->enableBUHeater(pid_output);
		}
		//transitions
		if(machStat->inStandbye() || dev->getBUTempSensorError()){
			//reenable quick start after standbye
			if(machStat->inStandbye()){
				quickStart = true;
			}
			state = disabled;
		}
		break;
	case disabled:
		//outputs
		dev->disableBUHeater();
		//transitions
		if(!machStat->inStandbye() && !dev->getBUTempSensorError()){
			state = enabled;
		}
		break;
	}
}

