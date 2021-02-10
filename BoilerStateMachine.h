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
#include <PID_v1.h>

//#define BOILER_QUICKSTART

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
	double pid_input;
	double pid_output;
	double pid_setpoint;
	PID *boilerPID;
};

#endif /* BOILERSTATEMACHINE_H_ */
