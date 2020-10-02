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

#define SSR_PERIOD_TIME			5000
#define	BUTTON_DEPRELL_TIME		300
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
	bool getButton1();
	bool getButton2();
	bool getButton1LongPressed(){return button1LongPressed;}
	bool getButton1ShortPressed(){return button1ShortPressed;}
	bool getButton2LongPressed(){return button2LongPressed;}
	bool getButton2ShortPressed(){return button2ShortPressed;}

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


private:
	static DeviceControl *_instance;
	DeviceControl();
	DeviceControl (const DeviceControl& );
	virtual ~DeviceControl();

	void init();
	void updateSR();

	uint16_t srData;
	double pumpTickToVolumeFactor;
	double bypassTickToVolumeFactor;
	static long pumpTicks;
	static long bypassTicks;

	unsigned long boilerPeriodStartTime;
	int boilerLevel;
	unsigned long BUPeriodStartTime;
	int BULevel;

	bool boilerFillSensorError;

	bool button1LastState;
	bool button2LastState;
	unsigned long button1LastChangeTime;
	unsigned long button2LastChangeTime;
	bool button1LongPressed;
	bool button2LongPressed;
	bool button1ShortPressed;
	bool button2ShortPressed;

	static TSIC *tsicBoiler;
	static TSIC *tsicBU;
	static TSIC *tsicTube;
	static void tsicBoilerWrapper(){tsicBoiler->TSIC_ISR();}
	static void tsicBUWrapper(){tsicBU->TSIC_ISR();}
	static void tsicTubeWrapper(){tsicTube->TSIC_ISR();}
	static void pumpFlowmeterISR(){pumpTicks++;}
	static void bypassFlowmeterISR(){bypassTicks++;}
};

#endif /* DEVICECONTROL_H_ */

