/*
 * ADSTempSensor.h
 *
 *  Created on: Mar 29, 2021
 *      Author: tsugua
 */

#ifndef ADSTEMPSENSOR_H_
#define ADSTEMPSENSOR_H_
#include "HW_config.h"
#include "ADS1115_WE.h"

#define ADS_MIN_UPDATE_INTERVAL	5000	//the sensor will be in error state if no new measurements will have been received
										//after this time
//plausibility checks:
#define ADS_PLAUSIBILITY_MAX_BOILER_TEMP		140
#define ADS_PLAUSIBILITY_MIN_BOILER_TEMP		-1
#define ADS_PLAUSIBILITY_MAX_BOILER_TEMP_JUMP	10
#define ADS_PLAUSIBILITY_STAY_IN_ERROR_TIME		30000	//time in ms after which error will be reset

class ADSTempSensor {
	enum adsStates{IDLE, measureBoiler, measureVCC};
public:
	ADSTempSensor();
	virtual ~ADSTempSensor();
	void init();
	void update();
	double getBoilerTemp();
	bool getBoilerTempSensorError();

private:
	void calculateBoilerTemp();
	void checkBoilerTempSensorForError();
	double boilerTemp;
	bool boilerTempSensorError;
	long lastUpdateTime;
	adsStates state;
	float boilerVoltage;
	float supplyVoltage;
};

#endif /* ADSTEMPSENSOR_H_ */
