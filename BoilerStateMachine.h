/*
 * BoilerStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BOILERSTATEMACHINE_H_
#define BOILERSTATEMACHINE_H_

#include "DeviceControl.h"
#include "MachineStatusStateMachine.h"

//#define QUICKSTART

class BoilerStateMachine {
	enum BoilerStates {enabled, disabled};

public:
	BoilerStateMachine();
	virtual ~BoilerStateMachine();
	void update();
private:
	BoilerStates state;
	DeviceControl *dev;
	MachineStatusStateMachine *machStat;
	bool quickStart;
};

#endif /* BOILERSTATEMACHINE_H_ */
