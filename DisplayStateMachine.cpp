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
}

void DisplayStateMachine::init(){
	machStat = MachineStatusStateMachine::instance();
	state = idle;
	dev->enableDisplayBacklight();
	tft.init();
	tft.fillScreen(TFT_BLACK);
	tft.setRotation(1);

	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_red, 320, TEXT_RED_X, TEXT_RED_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_blue, 320, TEXT_BLUE_X, TEXT_BLUE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);
	readBackground((uint16_t*)image_data_Background_V2_no_symbol, background_orange, 320, TEXT_ORANGE_X, TEXT_ORANGE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT);

	tft.setCursor(0,0);
	tft.setTextFont(1);
	tft.setTextColor(TFT_WHITE);
	tft.setTextSize(3);
	drawBackground();
}

void DisplayStateMachine::drawBackground(){
	tft.pushImage(0, 0, 320, 240, (uint16_t*)image_data_Background_V2_no_symbol);
//  tft.setCursor(138,140);
//  tft.println(String((char)9) + "C");
	tft.drawBitmap(10, 17, image_data_scale_small, 41, 40, TFT_WHITE);
	tft.drawBitmap(226, 16, image_data_timer_small, 33, 43, TFT_WHITE);
	displayTime(0);
	displayWeight(0);
}

void DisplayStateMachine::update(){
	static long lastTempUpdateTime = 0;
	static long brewingStartTime = 0;
	static double brewingStartPumpVolume = 0;
	static double brewingStartBypassVolume = 0;

	//update no wifi and blynk enabled icons
	if(DataManager::getBlynkEnabled() && state != standby){
		if(!DataManager::getWifiConnected() && !noWifiDisplayed){
			noWifiDisplayed = true;
			blynkDisplayed = false;
			tft.fillRect(145, 200, 30, 30, TFT_BLACK);
			tft.drawBitmap(145, 200, image_data_no_wifi_small, 30, 24, TFT_WHITE);
		}
		else if(DataManager::getWifiConnected() && !blynkDisplayed){
			blynkDisplayed = true;
			noWifiDisplayed = false;
			tft.fillRect(145, 200, 30, 30, TFT_BLACK);
			tft.drawBitmap(145, 200, image_data_blynk_small, 30, 30, TFT_WHITE);
		}
	}
	else if((noWifiDisplayed || blynkDisplayed) && state != standby){
		blynkDisplayed = false;
		noWifiDisplayed = false;
		tft.fillRect(145, 200, 30, 30, TFT_BLACK);
	}

	switch (state){
	case idle:
		if(millis() >= lastTempUpdateTime + TEMP_UPDATE_INTERVAL_IDLE){
			displayBoilerTemp(dev->getBoilerTemp(), dev->getBoilerTempSensorError());
			displayBUTemp(dev->getBUTemp(), dev->getBUTempSensorError());
			displayTubeTemp(dev->getTubeTemp(), dev->getTubeTempSensorError());
			lastTempUpdateTime = millis();
		}

		//transitions
		if(machStat->inStandbye()){
			tft.fillRect(0, 0, 320, 240, TFT_BLACK);
			tft.drawBitmap((320-158)/2, (240-180)/2, image_data_standby, 158, 180, 0b0001100011100011);
			dev->disableDisplayBacklight();
			state = standby;
		}
		else if(brewMachine->isBrewing() || brewMachine->isPreinfusing()){
			brewingStartTime = 0;
			brewingStartPumpVolume = dev->getPumpVolume();
			brewingStartBypassVolume = dev->getBypassVolume();
			displayTime(0);
			displayWeight(0);
			state = brewing;
		}
		else if(brewMachine->isCleaning()){
			state = cleaning;
		}
		break;
	case brewing:
	{
		if(brewMachine->isBrewing() && brewingStartTime == 0){	//preinfusion done
			brewingStartTime = millis();
		}
		if(millis() >= lastTempUpdateTime + TEMP_UPDATE_INTERVAL_BREWING){
			displayBoilerTemp(dev->getBoilerTemp(), dev->getBoilerTempSensorError());
			displayBUTemp(dev->getBUTemp(), dev->getBUTempSensorError());
			displayTubeTemp(dev->getTubeTemp(), dev->getTubeTempSensorError());
			lastTempUpdateTime = millis();
		}
		if(brewMachine->isBrewing()){// do not update time during preinfusion
			displayTime((millis()-brewingStartTime)/1000);
		}
		int weight = (dev->getPumpVolume()-brewingStartPumpVolume)-(dev->getBypassVolume()-brewingStartBypassVolume) -
				DataManager::getVolumeOffset();
		displayWeight(weight>=0?weight:0);	//display 0 while weight smaller than zero

		//transitions
		if(!brewMachine->isBrewing() && !brewMachine->isPreinfusing()){
			state = idle;
		}
		break;
	}
	case cleaning:
		if(millis() >= lastTempUpdateTime + TEMP_UPDATE_INTERVAL_IDLE){
			displayBoilerTemp(dev->getBoilerTemp(), dev->getBoilerTempSensorError());
			displayBUTemp(dev->getBUTemp(), dev->getBUTempSensorError());
			displayTubeTemp(dev->getTubeTemp(), dev->getTubeTempSensorError());
			lastTempUpdateTime = millis();
		}
		displayTime(0);
		displayWeight(brewMachine->getCurrentCleaningCycle());

		//transitions
		if(!brewMachine->isCleaning()){
			state = idle;
		}
		break;
	case standby:
		//transitions
		if(!machStat->inStandbye()){
			dev->enableDisplayBacklight();
			//get everything drawn again
			noWifiDisplayed = false;
			blynkDisplayed = false;
			displayedTime = -1;
			displayedWeight = -1;
			displayedBoilerTemp = "";
			displayedBUTemp = "";
			displayedTubeTemp = "";
			drawBackground();
			state = idle;
		}
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

void DisplayStateMachine::displayBoilerTemp(float temp, bool sensorError){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(sensorError)
		strcpy(puffer, "error");
	if(displayedBoilerTemp.equals(puffer)){
		return;
	}
	if(!sensorError)
		DataManager::pushTempBoiler(temp);
	displayedBoilerTemp = String(puffer);
	tft.pushImage(TEXT_RED_X, TEXT_RED_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT, background_red);
//	tft.setCursor(27,181);
	if(displayedBoilerTemp.length() < 5){
		tft.setCursor(TEXT_RED_X + LETTER_WIDTH/2, TEXT_RED_Y);
	}
	else{
		tft.setCursor(TEXT_RED_X, TEXT_RED_Y);
	}
	tft.print(puffer);
}

void DisplayStateMachine::displayTubeTemp(float temp, bool sensorError){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(sensorError)
		strcpy(puffer, "error");
	if(displayedTubeTemp.equals(puffer)){
		return;
	}
	if(!sensorError)
		DataManager::pushTempTube(temp);
	displayedTubeTemp = String(puffer);
	tft.pushImage(TEXT_BLUE_X, TEXT_BLUE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT, background_blue);
//	tft.setCursor(124,71);
	if(displayedTubeTemp.length() < 5){
		tft.setCursor(TEXT_BLUE_X + LETTER_WIDTH/2, TEXT_BLUE_Y);
	}
	else{
		tft.setCursor(TEXT_BLUE_X, TEXT_BLUE_Y);
	}
	tft.print(puffer);
}

void DisplayStateMachine::displayBUTemp(float temp, bool sensorError){
	char puffer[7];
	dtostrf(temp, 2, 1, puffer);
	if(sensorError)
		strcpy(puffer, "error");
	if(displayedBUTemp.equals(puffer)){
		return;
	}
	if(!sensorError)
		DataManager::pushTempBU(temp);
	displayedBUTemp = String(puffer);
	tft.pushImage(TEXT_ORANGE_X, TEXT_ORANGE_Y, TEXT_BACKGROUND_WIDTH, TEXT_BACKGROUND_HEIGHT, background_orange);
//	tft.setCursor(214,181);
	if(displayedBUTemp.length() < 5){
		tft.setCursor(TEXT_ORANGE_X + LETTER_WIDTH/2, TEXT_ORANGE_Y);
	}
	else{
		tft.setCursor(TEXT_ORANGE_X, TEXT_ORANGE_Y);
	}
	tft.print(puffer);
}

void DisplayStateMachine::displayTime(int time){
	if(displayedTime == time){
		return;
	}
	displayedTime = time;
	tft.fillRect(268, 28, 2*15+3, 21, TFT_BLACK);
	tft.setCursor(268,28);
	tft.print(time);
}

void DisplayStateMachine::displayWeight(int weight){
	if(displayedWeight == weight){
		return;
	}
	displayedWeight = weight;
	tft.fillRect(58, 28, 2*15+3, 21, TFT_BLACK);
	tft.setCursor(58,28);
	tft.println(String(weight));
}
