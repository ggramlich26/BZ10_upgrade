/*
 * BrewStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BrewStateMachine.h"
#include "DataManager.h"

BrewStateMachine::BrewStateMachine() {
	cleaningBuildupTime = 20000;
	cleaningFlushingTime = 20000;
	numberCleaningCycles = 10;
	currentCleaningCycle = 0;
	state = idle;
	dev = DeviceControl::instance();
	machStat = MachineStatusStateMachine::instance();
}

BrewStateMachine::~BrewStateMachine() {
}

void BrewStateMachine::update(){
	static long refillBoilerStartTime = 0;
	static long boilerFullStartTime = 0;
	static unsigned long preinfusionStartTime = 0;
	static unsigned long cleaningStartTime = 0; //used for both cleaning states as start time
	switch (state){
	case idle:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		dev->disableLEDTank();
		//transitions
		if(machStat->inStandbye()){
			state = idle;	//stay here
		}
		else if(!dev->getTankFull()){
			state = error_tank_empty;
		}
		else if(!dev->getBoilerFull()){
			state = refill_boiler;
		}
		else if(dev->getManualDistribution()){
			if(DataManager::getPreinfusionBuildupTime() > 0){
				state = preinf_buildup;
			}
			else{
				state = dist_man;
			}
		}
		else if(dev->getVolumetricDistribution()){
			if(DataManager::getPreinfusionBuildupTime() > 0){
				state = preinf_buildup;
			}
			else{
				state = dist_vol;
			}
		}
		else if(dev->getButton2LongPressed()){
			state = cleaning_buildup;
		}
		break;

	case dist_man:
		dev->disableBoilerValve();
		dev->enableBrewingValve();
		dev->enablePump();
		//transitions
		if(machStat->inStandbye()){
			state = idle;
		}
		else if(!dev->getTankFull()){
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
		if(machStat->inStandbye()){
			startPumpVolume = 0;
			startBypassVolume = 0;
			state = idle;
		}
		else if(!dev->getTankFull()){
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
		if(machStat->inStandbye()){
			state = idle;
		}
		else if(!dev->getTankFull()){
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
			boilerFullStartTime = 0;	// this happens when entering the state. Make sure it is initialized correctly
										// in case tank was empty while filling over probe level
		}
		if(machStat->inStandbye()){
			refillBoilerStartTime = 0;
			state = idle;
		}
		else if(!dev->getTankFull()){
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
		else if(millis() >= refillBoilerStartTime + DataManager::getBoilerMaxFillTime()){
			state = error_sonde;
		}
		break;
	case error_sonde:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		dev->setBoilerFillSensorError(true);
		//transitions
		//none, this state can only be left by restarting the machine
		break;
	case error_tank_empty:
		dev->disablePump();
		dev->disableBoilerValve();
		dev->disableBrewingValve();
		dev->enableLEDTank();
		//transitions
		if(dev->getTankFull()){
			state = idle;
		}
		break;
	case preinf_buildup:
		dev->enablePump();
		dev->enableBrewingValve();
		dev->disableBoilerValve();
		//transitions
		if(preinfusionStartTime == 0){
			preinfusionStartTime = millis();
		}
		if(machStat->inStandbye()){
			preinfusionStartTime = 0;
			state = idle;
		}
		else if(!dev->getManualDistribution() && !dev->getVolumetricDistribution()){
			preinfusionStartTime = 0;
			state = idle;
		}
		else if(!dev->getTankFull()){
			preinfusionStartTime = 0;
			state = error_tank_empty;
		}
		else if(millis() >= preinfusionStartTime + DataManager::getPreinfusionBuildupTime()){
			preinfusionStartTime = 0;
			state = preinf_wait;
		}
		break;
	case preinf_wait:
		dev->disablePump();
		dev->enableBrewingValve();
		dev->disableBoilerValve();
		//transitions
		if(preinfusionStartTime == 0){
			preinfusionStartTime = millis();
		}
		if(machStat->inStandbye()){
			preinfusionStartTime = 0;
			state = idle;
		}
		else if(!dev->getManualDistribution() && !dev->getVolumetricDistribution()){
			preinfusionStartTime = 0;
			state = idle;
		}
		else if(!dev->getTankFull()){
			preinfusionStartTime = 0;
			state = error_tank_empty;
		}
		else if(millis() >= preinfusionStartTime + DataManager::getPreinfusionWaitTime()){
			preinfusionStartTime = 0;
			if(dev->getManualDistribution()){
				state = dist_man;
			}
			else{
				state = dist_vol;
			}
		}
		break;
	case cleaning_buildup:
		dev->enablePump();
		dev->enableBrewingValve();
		dev->disableBoilerValve();
		//transitions
		if(cleaningStartTime == 0){
			cleaningStartTime = millis();
		}
		if(machStat->inStandbye()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = idle;
		}
		else if(dev->getButton2ShortPressed() || dev->getButton2LongPressed()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = error_tank_empty;
		}
		else if(!dev->getTankFull()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = error_tank_empty;
		}
		else if(millis() >= cleaningStartTime + cleaningBuildupTime){
			cleaningStartTime = 0;
			state = cleaning_flushing;
		}
		break;
	case cleaning_flushing:
		dev->disablePump();
		dev->disableBrewingValve();
		dev->disableBoilerValve();
		//transitions
		if(cleaningStartTime == 0){
			cleaningStartTime = millis();
		}
		if(machStat->inStandbye()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = idle;
		}
		else if(dev->getButton2ShortPressed() || dev->getButton2LongPressed()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = error_tank_empty;
		}
		else if(!dev->getTankFull()){
			cleaningStartTime = 0;
			currentCleaningCycle = 0;
			state = error_tank_empty;
		}
		else if(millis() >= cleaningStartTime + cleaningFlushingTime){
			cleaningStartTime = 0;
			if(currentCleaningCycle < numberCleaningCycles){
				currentCleaningCycle++;
				state = cleaning_buildup;
			}
			else{
				currentCleaningCycle = 0;
				state = idle;
			}
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

bool BrewStateMachine::isPreinfusing(){
	if(preinf_buildup == state || preinf_wait == state){
		return true;
	}
	return false;
}

bool BrewStateMachine::isCleaning(){
	if(cleaning_buildup == state || cleaning_flushing == state){
		return true;
	}
	return false;
}

int BrewStateMachine::getCurrentCleaningCycle(){
	return currentCleaningCycle;
}
