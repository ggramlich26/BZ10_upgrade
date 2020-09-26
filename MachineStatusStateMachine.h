/*
 * BoilerStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BOILERSTATEMACHINE_H_
#define BOILERSTATEMACHINE_H_

#include "DeviceControl.h"

class MachineStatusStateMachine {
	enum MachineStatusState {running, standbye};

public:
	MachineStatusStateMachine();
	virtual ~MachineStatusStateMachine();
	void update();
	bool inStandbye();
private:
	MachineStatusState state;
	DeviceControl *dev;
	unsigned long lastUserActionTime;
	unsigned long wakeupTime;	//time in ms after machine start when the machine will wake up from standbye. 0 to disable
								//automated wakeup function
	unsigned long standbyeStartTime; //time in ms after which standbye mode will be entered if no user interaction has occured
};

#endif /* BOILERSTATEMACHINE_H_ */
