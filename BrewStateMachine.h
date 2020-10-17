/*
 * BrewStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BREWSTATEMACHINE_H_
#define BREWSTATEMACHINE_H_

#include "DeviceControl.h"
#include "MachineStatusStateMachine.h"
#include "Arduino.h"

class BrewStateMachine {
	enum BrewStates {idle, dist_man, refill_boiler, dist_vol, dist_vol_finished, error_tank_empty, error_sonde,
		preinf_buildup, preinf_wait, cleaning_buildup, cleaning_flushing};
public:
	BrewStateMachine();
	bool isBrewing();
	bool isPreinfusing();
	bool isCleaning();
	int getCurrentCleaningCycle();
	virtual ~BrewStateMachine();
	void update();
private:
	BrewStates state;
	DeviceControl *dev;
	MachineStatusStateMachine *machStat;
	unsigned long cleaningBuildupTime;
	unsigned long cleaningFlushingTime;
	int numberCleaningCycles;
	int currentCleaningCycle;
};

#endif /* BREWSTATEMACHINE_H_ */
