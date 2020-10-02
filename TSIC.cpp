/*
 * TSIC.cpp
 *
 *  Created on: Aug 23, 2020
 *      Author: tsugua
 */

#include "TSIC.h"
#include "Arduino.h"

TSIC::TSIC(int pin) {
	TSICPin = pin;
	pinMode(pin, INPUT);
	state = idle;
	lastEdgeTime = 0;
	temperature = -1;
	data1=0;
	data2=0;
	parity1=0;
	parity2=0;
	bitCounter=0;
	halfBitTime = 62;
	lastUpdateTime = millis();
}

TSIC::~TSIC() {
}

/// This interrupt service routine has to be called every time the level of the TSIC data pin changes.
//	The main program is responsible for attaching this interrupt and calling the ISR. Do so by creating a wrapper function
//	and using attachInterrupts(PIN, wrapperFunction(), CHANGE).
void TSIC::TSIC_ISR(){
	long time = micros();
	switch (state){
	case idle:
		if(!digitalRead(TSICPin)){
			if(time-lastEdgeTime > TSIC_BREAK_MIN_TIME){
				state = startBit;
			}
		}
		break;
	case startBit:
		if(digitalRead(TSICPin)){
			state = byte1;
			data1=0;
			bitCounter=0;
			halfBitTime = time-lastEdgeTime;
		}
		else{
			state = idle;
		}
		break;
	case byte1:
		if(digitalRead(TSICPin)){
			if(time-lastEdgeTime < halfBitTime){
				if(bitCounter < 8){
					data1 = data1 << 1;
					data1 |= 0x01;
					bitCounter++;
				}
				else{
					parity1 = 0x01;
					bitCounter = -1;
					state = byte2;
					data2 = 0x00;
				}
			}
			else{
				if(bitCounter < 8){
					data1 = data1 << 1;
					bitCounter++;
				}
				else{
					parity1 = 0x00;
					bitCounter = -1;
					state = byte2;
					data2 = 0x00;
				}
			}
		}
		break;
	case byte2:
		if(digitalRead(TSICPin)){
			//skip new strobe bit
			if(bitCounter < 0){
				bitCounter = 0;
				break;
			}
			if(time-lastEdgeTime < halfBitTime){
				if(bitCounter < 8){
					data2 = data2 << 1;
					data2 |= 0x01;
					bitCounter++;
				}
				else{
					parity2 = 0x01;
					state = idle;
					calcTemp();
				}
			}
			else{
				if(bitCounter < 8){
					data2 = data2 << 1;
					bitCounter++;
				}
				else{
					parity2 = 0x00;
					state = idle;
					calcTemp();
				}
			}
		}
		break;
	}
	lastEdgeTime = time;
}

bool TSIC::sensorError(){
	return millis() >= lastUpdateTime + TSIC_MIN_UPDATE_INTERVAL;
}

/// calculates the temperature from the data recieved
void TSIC::calcTemp(){
	uint8_t par = 0;;
	for(uint8_t i = 0; i < 8; i++){
		par ^= (data1>>i)&0x01;
	}
	if((par^parity1)){
		return;
	}
	par = 0;
	for(uint8_t i = 0; i < 8; i++){
		par ^= (data2>>i)&0x01;
	}
	if((par^parity2)){
		return;
	}
	uint16_t temp_value = (((uint16_t)data1)<<8) | data2;
	temperature = ((double)temp_value / 2047 * 200) - 50;
	lastUpdateTime = millis();
}

/// returns the last read temperature from the TSIC
double TSIC::getTemperature(){
	return temperature;
}
