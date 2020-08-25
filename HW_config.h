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

#define	BU_VALVE			1
#define	BOILER_VALVE		3
#define	PUMP				4
#define	BOILER_HEATER		0
#define	BU_HEATER			2
#define	EXP_VALVE			5

#define	LED_LEFT			10
#define	LED_POWER			11
#define	LED_TANK			9
#define	LED_RIGHT			8

#define	TEMP_BOILER_PIN		2
#define	TEMP_BU_PIN			4
#define	TEMP_TUBE_PIN		18

#define	BUTTON_LEFT_PIN		27
#define	BUTTON_RIGHT_PIN	5
#define	BREW_VOL_PIN		26
#define	BREW_MAN_PIN		25

#define	FLOW_PUMP_PIN		13
#define	FLOW_RET_PIN		12

#define	SOUND_ANALOG_PIN	34
#define	SOUND_DIGITAL_PIN	35

#define TFT_DC 		23
#define TFT_CS 		17
#define	TFT_MOSI	32
#define	TFT_CLK		33
#define	TFT_MISO	14
#define TFT_RST		16


#endif /* HW_CONFIG_H_ */
