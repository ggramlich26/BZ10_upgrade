/*
 * BrewStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BrewStateMachine.h"
#include "DataManager.h"

BrewStateMachine::BrewStateMachine() {
	state = idle;
	dev = DeviceControl::instance();
}

BrewStateMachine::~BrewStateMachine() {
}

void BrewStateMachine::update(){
	static long refillBoilerStartTime = 0;
	static long boilerFullStartTime = 0;
	switch (state){
	case idle:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		//transitions
		if(!dev->getTankFull()){
			state = error_tank_empty;
		}
		else if(!dev->getBoilerFull()){
			state = refill_boiler;
		}
		else if(dev->getManualDistribution()){
			state = dist_man;
		}
		else if(dev->getVolumetricDistribution()){
			state = dist_vol;
		}
		break;

	case dist_man:
		dev->disableBoilerValve();
		dev->enableBrewingValve();
		dev->enablePump();
		//transitions
		if(!dev->getTankFull()){
			state = error_tank_empty;
		}
		else if(!dev->getManualDistribution()){
			state = idle;
		}
		break;

	case dist_vol:
		dev->disableBoilerValve();
		dev->enableBrewingValve();
		dev->enablePump();
		static double startPumpVolume = 0;
		static double startBypassVolume = 0;
		if(0 == startPumpVolume){
			startPumpVolume = dev->getPumpVolume();
		}
		if(0 == startBypassVolume){
			startBypassVolume = dev->getBypassVolume();
		}
		//transitions
		if(!dev->getTankFull()){
			startPumpVolume = 0;
			startBypassVolume = 0;
			state = error_tank_empty;
		}
		else if(!dev->getVolumetricDistribution()){
			startPumpVolume = 0;
			startBypassVolume = 0;
			state = idle;
		}
		else if((dev->getPumpVolume() - startPumpVolume) - (dev->getBypassVolume() - startBypassVolume) >=
				DataManager::getDistributionVolume() + DataManager::getVolumeOffset()){
			startPumpVolume = 0;
			startBypassVolume = 0;
			state = dist_vol_finished;
		}
		break;

	case dist_vol_finished:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		//transitions
		if(!dev->getTankFull()){
			state = error_tank_empty;
		}
		else if(!dev->getVolumetricDistribution()){
			state = idle;
		}
		break;

	case refill_boiler:
		dev->disableBrewingValve();
		dev->enableBoilerValve();
		dev->enablePump();
		//transitions
		if(refillBoilerStartTime == 0){
			refillBoilerStartTime = millis();
		}
		if(!dev->getTankFull()){
			refillBoilerStartTime = 0;
			state = error_tank_empty;
		}
		else if(dev->getBoilerFull()){
			if(boilerFullStartTime == 0){
				boilerFullStartTime = millis();
			}
			if(millis() >= boilerFullStartTime + DataManager::getFillBoilerOverSondeTime()){
				boilerFullStartTime = 0;
				refillBoilerStartTime = 0;
				state = idle;
			}
		}
		else if(dev->getManualDistribution()){
			refillBoilerStartTime = 0;
			state = dist_man;
		}
		else if(dev->getVolumetricDistribution()){
			refillBoilerStartTime = 0;
			state = dist_vol;
		}
		else if(millis() >= refillBoilerStartTime + DataManager::getBoilerMaxFillTime()){
			state = error_sonde;
		}
		break;
	case error_sonde:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		//transitions
		//none, this state can only be left by restarting the machine
		break;
	case error_tank_empty:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		//transitions
		if(dev->getTankFull()){
			state = idle;
		}
		break;
	}
}

bool BrewStateMachine::isBrewing(){
	if(dist_man == state || dist_vol == state){
		return true;
	}
	return false;
}
