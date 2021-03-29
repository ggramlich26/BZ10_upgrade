/*
 * DeviceControl.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef DEVICECONTROL_H_
#define DEVICECONTROL_H_

#include "HW_config.h"
#include "Arduino.h"
#include "TSIC.h"
#include "MCP23017.h"
#include "ADSTempSensor.h"
#include "Wire.h"
#include "Button.h"

#define SSR_PERIOD_TIME			5000
#define	BUTTON_DEPRELL_TIME		300
#define DIST_SWITCH_DEPRELL_TIME	100
#define	BUTTON_LONG_PRESS_TIME	3000

class DeviceControl {
public:
	static DeviceControl *instance(){
		if(!_instance)
			_instance = new DeviceControl();
		return _instance;
	}
	void update();

	void enableBoilerHeater(int level);
	void disableBoilerHeater();

	void enableBUHeater(int level);
	void disableBUHeater();

	void enablePump();
	void disablePump();
	void enableBrewingValve();		//makes water go to brewing unit
	void disableBrewingValve();		//makes water go to sink
	void enableBoilerValve();		//makes water go to boiler
	void disableBoilerValve();		//makes water go to brewing water heater

	bool getBoilerFull();			//true if the boiler is sufficiently filled
	bool getTankFull();	//true if the cold water tank in the back of the machine is full
	bool getBoilerFillSensorError();	//returns true, if an error for the boiler sensor has been detected, see BrewStateMachine
	void setBoilerFillSensorError(bool error);

	bool getManualDistribution();
	bool getVolumetricDistribution();
	bool readButton1();
	bool readButton2();
	bool readButtonVolDist();
	bool readButtonManDist();
	bool getButton1State();
	bool getButton2State();
	bool getButton1LongPressed(){return button1->isLongPressed();}
	bool getButton1ShortPressed(){return button1->isShortPressed();}
	bool getButton2LongPressed(){return button2->isLongPressed();}
	bool getButton2ShortPressed(){return button2->isShortPressed();}

	double getPumpVolume();
	double getBypassVolume();

	double getBoilerTemp();
	double getBUTemp();
	double getTubeTemp();
	bool getBoilerTempSensorError();
	bool getBUTempSensorError();
	bool getTubeTempSensorError();

	void enableLEDLeft();
	void disableLEDLeft();
	void enableLEDRight();
	void disableLEDRight();
	void enableLEDPower();
	void disableLEDPower();
	void enableLEDTank();
	void disableLEDTank();
	void enableDisplayBacklight();
	void disableDisplayBacklight();


private:
	static DeviceControl *_instance;
	DeviceControl();
	DeviceControl (const DeviceControl& );
	virtual ~DeviceControl();

	void init();
	void boilerHeaterOn(bool send);
	void boilerHeaterOff(bool send);
	void BUHeaterOn(bool send);
	void BUHeaterOff(bool send);

	uint16_t mcpWriteBuffer;
	uint16_t mcpReadBuffer;

	unsigned long boilerPeriodStartTime;
	int boilerLevel;
	unsigned long BUPeriodStartTime;
	int BULevel;

	bool boilerFillSensorError;

	Button *button1;
	Button *button2;
	Button *buttonVolDist;
	Button *buttonManDist;

};

#endif /* DEVICECONTROL_H_ */

