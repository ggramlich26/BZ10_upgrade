/*
 * DisplayStateMachine.cpp
 *
 *  Created on: Aug 16, 2020
 *      Author: tsugua
 */

#include "DisplayStateMachine.h"
#include "DataManager.h"

DisplayStateMachine::DisplayStateMachine(BrewStateMachine *machine) {
	brewMachine = machine;
	dev = DeviceControl::instance();
	init();
}

DisplayStateMachine::~DisplayStateMachine() {
	// TODO Auto-generated destructor stub
}

void DisplayStateMachine::init(){
	state = idle;
	tft.begin();
	tft.setRotation(1);

	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_red, 320, TEXT_RED_X, TEXT_RED_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_blue, 320, TEXT_BLUE_X, TEXT_BLUE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_orange, 320, TEXT_ORANGE_X, TEXT_ORANGE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);

	tft.setCursor(0,0);
	tft.setTextColor(ILI9341_WHITE);
	tft.setFont();
	tft.setTextSize(3);
	tft.drawRGBBitmap(0, 0, (uint16_t*)image_data_Background_V2_no_symbol, 320, 240);
//  tft.setCursor(138,140);
//  tft.println(String((char)9) + "C");
	tft.drawBitmap(10, 17, image_data_scale_small, 41, 40, ILI9341_WHITE);
	tft.drawBitmap(226, 16, image_data_timer_small, 33, 43, ILI9341_WHITE);
}

void DisplayStateMachine::update(){
	static long lastTempUpdateTime = 0;
	static long brewingStartTime = 0;
	static double brewingStartPumpVolume = 0;
	static double brewingStartBypassVolume = 0;
	switch (state){
	case idle:
		if(millis() >= lastTempUpdateTime + TEMP_UPDATE_INTERVAL_IDLE){
			displayBoilerTemp(dev->getBoilerTemp());
			displayBUTemp(dev->getBUTemp());
			displayTubeTemp(dev->getTubeTemp());
			lastTempUpdateTime = millis();
		}
		if(brewMachine->isBrewing()){
			brewingStartTime = millis();
			brewingStartPumpVolume = dev->getPumpVolume();
			brewingStartBypassVolume = dev->getBypassVolume();
			displayTime(0);
			displayWeight(0);
			state = brewing;
		}
		break;
	case brewing:
		if(millis() >= lastTempUpdateTime + TEMP_UPDATE_INTERVAL_BREWING){
			displayBoilerTemp(dev->getBoilerTemp());
			displayBUTemp(dev->getBUTemp());
			displayTubeTemp(dev->getTubeTemp());
			lastTempUpdateTime = millis();
		}
		displayTime((millis()-brewingStartTime)/1000);
		displayWeight((dev->getPumpVolume()-brewingStartPumpVolume)-(dev->getBypassVolume()-brewingStartBypassVolume));
		if(!brewMachine->isBrewing()){
			state = idle;
		}
		break;
	}
}

void DisplayStateMachine::readBackground(uint16_t *origin, uint16_t *destination,
		uint16_t origin_width, uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	for(uint16_t i = 0; i < height; i++){
		for(uint16_t j = 0; j < width; j++){
			*(destination+i*width+j) = *(origin+(y+i)*origin_width+x+j);
		}
	}
}

void DisplayStateMachine::displayBoilerTemp(float temp){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(displayedBoilerTemp.equals(puffer)){
		return;
	}
	DataManager::pushTempBoiler(temp);
	displayedBoilerTemp = String(puffer);
	tft.drawRGBBitmap(TEXT_RED_X, TEXT_RED_Y, background_red, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	tft.setCursor(27,181);
	tft.print(puffer);
}

void DisplayStateMachine::displayTubeTemp(float temp){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(displayedTubeTemp.equals(puffer)){
		return;
	}
	DataManager::pushTempTube(temp);
	displayedTubeTemp = String(puffer);
	tft.drawRGBBitmap(TEXT_BLUE_X, TEXT_BLUE_Y, background_blue, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	tft.setCursor(124,71);
	tft.print(puffer);
}

void DisplayStateMachine::displayBUTemp(float temp){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(displayedBUTemp.equals(puffer)){
		return;
	}
	DataManager::pushTempBU(temp);
	displayedBUTemp = String(puffer);
	tft.drawRGBBitmap(TEXT_ORANGE_X, TEXT_ORANGE_Y, background_orange, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	tft.setCursor(214,181);
	tft.print(puffer);
}

void DisplayStateMachine::displayTime(int time){
	if(displayedTime == time){
		return;
	}
	displayedTime = time;
	tft.fillRect(268, 28, 2*15+3, 21, ILI9341_BLACK);
	tft.setCursor(268,28);
	tft.println(time);
}

void DisplayStateMachine::displayWeight(int weight){
	if(displayedWeight == weight){
		return;
	}
	displayedWeight = weight;
	tft.fillRect(58, 28, 2*15+3, 21, ILI9341_BLACK);
	tft.setCursor(58,28);
	tft.println(String(weight));
}
