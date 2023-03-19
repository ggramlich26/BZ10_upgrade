#include "SPIFFS.h"
#include "Arduino.h"
#include "BoilerStateMachine.h"
#include "BUStateMachine.h"
#include "BrewStateMachine.h"
#include "DisplayStateMachine.h"
#include "MachineStatusStateMachine.h"
#include "DeviceControl.h"
#include "DataManager.h"
#include "Webserver.h"
#include "WifiManager.h"

//The setup function is called once at startup of the sketch

BoilerStateMachine *boilerSM;
BUStateMachine *brewingUnitSM;
BrewStateMachine *brewSM;
DisplayStateMachine *dispSM;
MachineStatusStateMachine *machineStatSM;
DeviceControl *dev;
WifiManager *wifiMan;

void setup()
{
	Serial.begin(115200);
	if(!SPIFFS.begin(true)){
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
	dev = DeviceControl::instance();
	DataManager::init();
	machineStatSM = MachineStatusStateMachine::instance();
	boilerSM = new BoilerStateMachine();
	brewingUnitSM = new BUStateMachine();
	brewSM = new BrewStateMachine();
	dispSM = new DisplayStateMachine(brewSM);
	wifiMan = WifiManager::instance();

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
	wifiMan->update();

	static bool webserverInitialized = false;
	if(DataManager::getWifiEnabled() && !webserverInitialized){
		//webserver only has to be initialized and then runs on its own
		Webserver::instance()->init();
		webserverInitialized = true;
	}
	if(dev->getButton1LongPressed()){
		DataManager::setWifiEnabled(!DataManager::getWifiEnabled());
	}
}
