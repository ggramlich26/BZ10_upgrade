/*
 * BUStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BUStateMachine.h"

BUStateMachine::BUStateMachine() {
	state = enabled;
	dev = DeviceControl::instance();
}

BUStateMachine::~BUStateMachine() {
}

void BUStateMachine::update(){
	switch (state){
	case enabled:
		dev->enableBUHeater(100);
		break;
	case disabled:
		dev->disableBUHeater();
		state = enabled;
		break;
	}
}

