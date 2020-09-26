#include "Arduino.h"
#include "BoilerStateMachine.h"
#include "BUStateMachine.h"
#include "BrewStateMachine.h"
#include "DisplayStateMachine.h"
#include "MachineStatusStateMachine.h"
#include "DeviceControl.h"
#include "DataManager.h"

//The setup function is called once at startup of the sketch

BoilerStateMachine *boilerSM;
BUStateMachine *brewingUnitSM;
BrewStateMachine *brewSM;
DisplayStateMachine *dispSM;
MachineStatusStateMachine *machineStatSM;
DeviceControl *dev;
//TSIC *tsic;

//void interruptWrapper(){
//	tsic->TSIC_ISR();
//}

void setup()
{
	Serial.begin(115200);
	dev = DeviceControl::instance();
	DataManager::init();
	machineStatSM = new MachineStatusStateMachine();
	boilerSM = new BoilerStateMachine();
	brewingUnitSM = new BUStateMachine();
	brewSM = new BrewStateMachine();
	dispSM = new DisplayStateMachine(brewSM);
//	tsic = new TSIC(18);
//	pinMode(18, INPUT);
//	attachInterrupt(18, interruptWrapper, CHANGE);
}

void loop()
{
//	static long lastTempTime = 0;
	machineStatSM->update();
	boilerSM->update();
	brewingUnitSM->update();
	brewSM->update();
	dispSM->update();
	DataManager::update();
	dev->update();
//	if(millis()-lastTempTime > 500){
//		lastTempTime = millis();
//		Serial.println("Temp: " + String(dev->getTubeTemp()));
//	}
}
