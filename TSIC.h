/*
 * TSIC.h
 *
 *  Created on: Aug 23, 2020
 *      Author: tsugua
 */

#ifndef TSIC_H_
#define TSIC_H_

#include "Arduino.h"

#define	TSIC_START_MIN_TIME		47
#define	TSIC_START_MAX_TIME		78
#define	TSIC_ONE_MAX_TIME		47
#define	TSIC_ZERO_MIN_TIME		78
#define	TSIC_BREAK_MIN_TIME		1000
#define TSIC_MIN_UPDATE_INTERVAL	5000	//the sensor will be in error state if
											// no valid temperature has been read for that time (in ms)

class TSIC {
	enum TSICStates {idle, startBit, byte1, byte2};
public:
	TSIC(int pin);
	virtual ~TSIC();
//	static void enable(TSIC t);
//	void disable();
	double getTemperature();
	void TSIC_ISR();
	bool sensorError();
private:
	void calcTemp();
	int TSICPin;
	unsigned long lastEdgeTime;
	unsigned long lastUpdateTime;	//last time a temperature value has been successfully read
	TSICStates state;
	double temperature;
	uint8_t data1;
	uint8_t data2;
	uint8_t parity1;
	uint8_t parity2;
	int bitCounter;
	unsigned long halfBitTime;
};

#endif /* TSIC_H_ */
