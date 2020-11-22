/*
 * device_config.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

#define SR_SER				22
#define	SR_SCK				19
#define	SR_RCK				21

#define MCP_SCL				22
#define MCP_SDA				21
#define MCP_ADDR			0x21

#define	BU_VALVE			9
#define BU_VALVE_LOW_LEVEL_TRIGGER
#define	BOILER_VALVE		8
#define BOILER_VALVE_LOW_LEVEL_TRIGGER
#define	PUMP				10
#define PUMP_LOW_LEVEL_TRIGGER
#define	BOILER_HEATER		2
//#define BOILER_HEATER_LOW_LEVEL_TRIGGER
#define	BU_HEATER			1
//#define BU_HEATER_LOW_LEVEL_TRIGGER
#define	EXP_VALVE			11
#define EXP_VALVE_LOW_LEVEL_TRIGGER

#define	LED_LEFT			4
#define	LED_POWER			5
#define	LED_TANK			6
#define	LED_RIGHT			7

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
