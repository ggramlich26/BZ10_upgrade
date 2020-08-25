/*
 * BrewStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BREWSTATEMACHINE_H_
#define BREWSTATEMACHINE_H_

#include "DeviceControl.h"
#include "Arduino.h"

class BrewStateMachine {
	enum BrewStates {idle, dist_man, refill_boiler, dist_vol, dist_vol_finished, error_tank_empty, error_sonde};
public:
	BrewStateMachine();
	bool isBrewing();
	virtual ~BrewStateMachine();
	void update();
private:
	BrewStates state;
	DeviceControl *dev;
};

#endif /* BREWSTATEMACHINE_H_ */
