/*
 * BoilerStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef MACHINESTATUSSTATEMACHINE_H_
#define MACHINESTATUSSTATEMACHINE_H_

#include "DeviceControl.h"

class MachineStatusStateMachine {
	enum MachineStatusState {running, standby};

public:
	static MachineStatusStateMachine *instance(){
		if(!_instance)
			_instance = new MachineStatusStateMachine();
		return _instance;
	}
	void update();
	bool inStandbye();
private:
	static MachineStatusStateMachine *_instance;
	MachineStatusStateMachine();
	MachineStatusStateMachine (const DeviceControl& );
	virtual ~MachineStatusStateMachine();
	MachineStatusState state;
	DeviceControl *dev;
	unsigned long lastUserActionTime;
//	unsigned long wakeupTime;	//time in ms after machine start when the machine will wake up from standby. 0 to disable
								//automated wakeup function
};

#endif /* MACHINESTATUSSTATEMACHINE_H_ */
