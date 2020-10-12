/*
 * DisplayStateMachine.h
 *
 *  Created on: Aug 16, 2020
 *      Author: tsugua
 */

#ifndef DISPLAYSTATEMACHINE_H_
#define DISPLAYSTATEMACHINE_H_
#include "HW_config.h"
#include "BrewStateMachine.h"
#include "DeviceControl.h"

#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "graphics/Background_V2_no_symbol.h"
#include "graphics/scale_small.h"
#include "graphics/timer_small.h"

#define	TEMP_UPDATE_INTERVAL_IDLE		1000
#define	TEMP_UPDATE_INTERVAL_BREWING	500


#define TEXT_BLUE_X		116
#define	TEXT_BLUE_Y		71
#define TEXT_RED_X		29
#define	TEXT_RED_Y		179
#define	TEXT_ORANGE_X	210
#define	TEXT_ORANGE_Y	179
#define	LETTER_WIDTH	15
#define	LETTER_HEIGHT	21
#define	TEXT_BACKGROUND_WIDTH	(5*LETTER_WIDTH+4*3)
#define	TEXT_BACKGROUND_HEIGHT	LETTER_HEIGHT

class DisplayStateMachine {
	enum DisplayStates {idle, brewing, cleaning};

public:
	DisplayStateMachine(BrewStateMachine *machine);
	virtual ~DisplayStateMachine();
	void update();
private:
	void init();
	Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
	uint16_t background_red[TEXT_BACKGROUND_WIDTH*TEXT_BACKGROUND_HEIGHT];
	uint16_t background_blue[TEXT_BACKGROUND_WIDTH*TEXT_BACKGROUND_HEIGHT];
	uint16_t background_orange[TEXT_BACKGROUND_WIDTH*TEXT_BACKGROUND_HEIGHT];

	DeviceControl *dev;
	DisplayStates state;
	BrewStateMachine *brewMachine;
	int displayedTime = -1;
	int displayedWeight = -1;
	String displayedBoilerTemp = "";
	String displayedBUTemp = "";
	String displayedTubeTemp = "";

	void readBackground(uint16_t *origin, uint16_t *destination, uint16_t origin_width, uint16_t x, uint16_t y, uint16_t width, uint16_t height);;
	void displayBoilerTemp(float temp, bool sensorError);
	void displayBUTemp(float temp, bool sensorError);
	void displayTubeTemp(float temp, bool sensorError);
	void displayTime(int time);
	void displayWeight(int weight);
};

#endif /* DISPLAYSTATEMACHINE_H_ */
