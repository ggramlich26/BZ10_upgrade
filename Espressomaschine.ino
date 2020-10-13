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

void setup()
{
	Serial.begin(115200);
	dev = DeviceControl::instance();
	DataManager::init();
	machineStatSM = MachineStatusStateMachine::instance();
	boilerSM = new BoilerStateMachine();
	brewingUnitSM = new BUStateMachine();
	brewSM = new BrewStateMachine();
	dispSM = new DisplayStateMachine(brewSM);
	Serial.println("Espresso machine initialized");
}

void loop()
{
	machineStatSM->update();
	boilerSM->update();
	brewingUnitSM->update();
	brewSM->update();
	dispSM->update();
	DataManager::update();
	dev->update();

	if(dev->getButton1LongPressed()){
		DataManager::setBlynkEnabled(!DataManager::getBlynkEnabled());
	}
}
