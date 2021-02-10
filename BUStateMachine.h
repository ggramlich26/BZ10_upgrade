/*
 * BUStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BUSTATEMACHINE_H_
#define BUSTATEMACHINE_H_

#include "DeviceControl.h"
#include "MachineStatusStateMachine.h"
#include <PID_v1.h>

//#define BU_QUICKSTART

class BUStateMachine {
	enum BUStates{enabled, disabled};

public:
	BUStateMachine();
	virtual ~BUStateMachine();
	void update();
private:
	BUStates state;
	DeviceControl *dev;
	MachineStatusStateMachine *machStat;
	bool quickStart;
	double pid_input;
	double pid_output;
	double pid_setpoint;
	PID *BUpid;
};

#endif /* BUSTATEMACHINE_H_ */
