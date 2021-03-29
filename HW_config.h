/*
 * device_config.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

#include "ADS1115_WE.h"

#define SR_SER				22
#define	SR_SCK				19
#define	SR_RCK				21

#define MCP_SCL				22
#define MCP_SDA				21
#define MCP_ADDR			0x21
#define ADS_ADDR			0x48
#define ADS_RDY				12
#define ADS_SAMPLE_INTERVAL	100		//sample interval in ms

#define	BU_VALVE			9
#define BU_VALVE_LOW_LEVEL_TRIGGER
#define	BOILER_VALVE		8
#define BOILER_VALVE_LOW_LEVEL_TRIGGER
#define	PUMP				10
#define PUMP_LOW_LEVEL_TRIGGER
#define	BOILER_HEATER		2
//#define BOILER_HEATER_LOW_LEVEL_TRIGGER
#define	BU_HEATER			11
#define BU_HEATER_LOW_LEVEL_TRIGGER
#define	EXP_VALVE			1
#define EXP_VALVE_LOW_LEVEL_TRIGGER

#define	LED_LEFT			4
#define	LED_POWER			5
#define	LED_TANK			6
#define	LED_RIGHT			7

#define BOILER_TEMP_ON_ADS	//comment to use TSIC for boiler temperature
#define ADS_BOILER_INPUT	ADS1115_COMP_0_GND
#define ADS_BOILER_MAX_VOLT	ADS1115_RANGE_4096
#define ADS_VCC_INPUT		ADS1115_COMP_1_GND
#define ADS_VCC_MAX_VOLT	ADS1115_RANGE_2048
#define ADS_VCC_CONV_FACTOR	2	//(R27+R28)/R28
#define ADS_BOILER_REF_R	22e3	//R29
//calculate NTC temperature:	(see Wikipedia)
// 1/T = 1/TN + 1/B*ln(RT/RN) with
// T:	temperature
// TN:	reference temperature
// B:	conversion factor
// RT:	NTC resistance at T
// RN:	NTC resistance at TN
#define ADS_BOILER_B		4458	//factor for calculation of NTC temperature
#define ADS_BOILER_TN		373.15	//reference temperature: 100 degree celsius
#define ADS_BOILER_RN		1.12e3	//resistance at TN
#define ADS_T0				273.15	//0 degree celsius in Kelvin for convenience
//TSIC pins:
#define	TEMP_BOILER_PIN		19
#define	TEMP_BU_PIN			4
#define	TEMP_TUBE_PIN		18

#define	BUTTON_LEFT_PIN		13
#define	BUTTON_RIGHT_PIN	12
#define	BREW_VOL_PIN		14
#define	BREW_MAN_PIN		15
#define TANK_PIN			0

#define	FLOW_PUMP_PIN		13
#define	FLOW_RET_PIN		27

#define	PROBE_ANALOG_PIN	34
#define	PROBE_DIGITAL_PIN	35

#define TFT_DC 		23
#define TFT_CS 		17
#define	TFT_MOSI	32
#define	TFT_CLK		33
#define	TFT_MISO	14
#define TFT_RST		16
#define TFT_LED		2

#endif /* HW_CONFIG_H_ */
