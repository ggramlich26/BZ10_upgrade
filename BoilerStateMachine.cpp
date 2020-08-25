/*
 * BoilerStateMachine.cpp
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#include "BoilerStateMachine.h"

BoilerStateMachine::BoilerStateMachine() {
	state = disabled;
	dev = DeviceControl::instance();
}

BoilerStateMachine::~BoilerStateMachine() {
}

void BoilerStateMachine::update(){
	switch (state){
	case disabled:
		//set outputs
		dev->disableBoilerHeater();
		//check transitions
		if(dev->getBoilerFull()){
			state = enabled;
		}
		break;
	case enabled:
		//set outputs
		dev->enableBoilerHeater(100);
		//check transitions
		if(!dev->getBoilerFull()){
			state = disabled;
		}
		break;
	}

}

