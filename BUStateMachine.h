/*
 * BUStateMachine.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef BUSTATEMACHINE_H_
#define BUSTATEMACHINE_H_

#include "DeviceControl.h"

class BUStateMachine {
	enum BUStates{enabled, disabled};

public:
	BUStateMachine();
	virtual ~BUStateMachine();
	void update();
private:
	BUStates state;
	DeviceControl *dev;
};

#endif /* BUSTATEMACHINE_H_ */
