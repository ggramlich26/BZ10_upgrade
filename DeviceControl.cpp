/*
 * DeviceControl.cpp
 *
 *  Created on: Aug 23, 2020
 *      Author: tsugua
 */

#include "DeviceControl.h"
#include "DataManager.h"

DeviceControl* DeviceControl::_instance = NULL;

static TSIC *tsicBoiler;
static TSIC *tsicBU;
static TSIC *tsicTube;
static volatile long pumpTicks;
static volatile long bypassTicks;

void IRAM_ATTR tsicBoilerWrapper(){tsicBoiler->TSIC_ISR();}
void IRAM_ATTR tsicBUWrapper(){tsicBU->TSIC_ISR();}
void IRAM_ATTR tsicTubeWrapper(){tsicTube->TSIC_ISR();}
void IRAM_ATTR pumpFlowmeterISR(){pumpTicks++;}
void IRAM_ATTR bypassFlowmeterISR(){bypassTicks++;}

DeviceControl::DeviceControl() {
	init();
}

DeviceControl::~DeviceControl() {
}

MCP23017 mcp = MCP23017(MCP_ADDR);
/// initializes all pins and interrupts etc. necessary
void DeviceControl::init(){
	Wire.begin();
	mcpWriteBuffer = 0x0000;
	mcpReadBuffer = 0x0000;
	pumpTicks = 5;
	bypassTicks = 5;
	boilerPeriodStartTime = 0;
	BUPeriodStartTime = 0;
	boilerLevel = 0;
	BULevel = 0;
	boilerFillSensorError = false;
	button1 = new Button([](){return DeviceControl::instance()->readButton1();});
	button2 = new Button([](){return DeviceControl::instance()->readButton2();});
	buttonVolDist = new Button([](){return DeviceControl::instance()->readButtonVolDist();});
	buttonManDist = new Button([](){return DeviceControl::instance()->readButtonManDist();});

	mcp.init();

    BUHeaterOff(false);
    boilerHeaterOff(false);
    disablePump();
    disableBrewingValve();
    disableBoilerValve();
    disableLEDLeft();
    disableLEDRight();
    disableLEDPower();
    disableLEDTank();
    mcp.write(mcpWriteBuffer);

	uint16_t mcp_ddr = 0x0000;
	mcp_ddr |= (1<<TANK_PIN)|(1<<BUTTON_LEFT_PIN)|(1<<BUTTON_RIGHT_PIN)|
			(1<<BREW_VOL_PIN)|(1<<BREW_MAN_PIN);
	//configure MCP23017 input/output pins
    mcp.portMode(MCP23017_PORT::A, (uint8_t)mcp_ddr);
    mcp.portMode(MCP23017_PORT::B, (uint8_t)(mcp_ddr>>8));

    mcpReadBuffer = mcp.read();

	pinMode(TEMP_BOILER_PIN, INPUT);
	pinMode(TEMP_BU_PIN, INPUT);
	pinMode(TEMP_TUBE_PIN, INPUT);
	pinMode(FLOW_PUMP_PIN, INPUT);
	pinMode(FLOW_RET_PIN, INPUT);
	pinMode(PROBE_DIGITAL_PIN, INPUT);
	pinMode(TFT_LED, OUTPUT);
	digitalWrite(TFT_LED, LOW);


	tsicBoiler = new TSIC(TEMP_BOILER_PIN);
	tsicBU = new TSIC(TEMP_BU_PIN);
	tsicTube = new TSIC(TEMP_TUBE_PIN);
	attachInterrupt(TEMP_BOILER_PIN, tsicBoilerWrapper, CHANGE);
	attachInterrupt(TEMP_BU_PIN, tsicBUWrapper, CHANGE);
	attachInterrupt(TEMP_TUBE_PIN, tsicTubeWrapper, CHANGE);
	attachInterrupt(FLOW_PUMP_PIN, pumpFlowmeterISR, RISING);
	attachInterrupt(FLOW_RET_PIN, bypassFlowmeterISR, RISING);
}

/// update routine, that should be called regularly. Some values are only updated through this routine.
void DeviceControl::update(){
	//update boiler heater
	if(boilerLevel == 0){
//		mcpWriteBuffer &= ~(1<<BOILER_HEATER);
		boilerHeaterOff(false);
	}
	else if(boilerLevel == 100){
//		mcpWriteBuffer |= (1<<BOILER_HEATER);
		boilerHeaterOn(false);
	}
	else{
		if(millis() >= boilerPeriodStartTime + SSR_PERIOD_TIME){
			boilerPeriodStartTime = millis();
//			mcpWriteBuffer |= (1<<BOILER_HEATER);
			boilerHeaterOn(false);
		}
		else if(millis() < boilerPeriodStartTime + boilerLevel/100.0*SSR_PERIOD_TIME){
//			mcpWriteBuffer |= (1<<BOILER_HEATER);
			boilerHeaterOn(false);
		}
		else{
//			mcpWriteBuffer &= ~(1<<BOILER_HEATER);
			boilerHeaterOff(false);
		}
	}
	//update BU heater
	if(BULevel == 0){
//		mcpWriteBuffer &= ~(1<<BU_HEATER);
		BUHeaterOff(false);
	}
	else if(BULevel == 100){
//		mcpWriteBuffer |= (1<<BU_HEATER);
		BUHeaterOn(false);
	}
	else{
		if(millis() >= BUPeriodStartTime + SSR_PERIOD_TIME){
			BUPeriodStartTime = millis();
//			mcpWriteBuffer |= (1<<BU_HEATER);
			BUHeaterOn(false);
		}
		else if(millis() < BUPeriodStartTime + BULevel/100.0*SSR_PERIOD_TIME){
//			mcpWriteBuffer |= (1<<BU_HEATER);
			BUHeaterOn(false);
		}
		else{
//			mcpWriteBuffer &= ~(1<<BU_HEATER);
			BUHeaterOff(false);
		}
	}
	mcp.write(mcpWriteBuffer);

	mcpReadBuffer = mcp.read();
	//update buttons
	button1->update();
	button2->update();
	buttonManDist->update();
	buttonVolDist->update();

}

/// enables the boiler heater
// @param level: heater level between 0 (off) and 100 (full power)
void DeviceControl::enableBoilerHeater(int level){
	if(level < 0) level = 0;
	else if (level > 100) level = 100;
	boilerLevel = level;
}

void DeviceControl::disableBoilerHeater(){
	boilerLevel = 0;
	boilerHeaterOff(true);
}

/// enables the brewing unit heater
// @param level: heater level between 0 (off) and 100 (full power)
void DeviceControl::enableBUHeater(int level){
	if(level < 0) level = 0;
	else if (level > 100) level = 100;
	BULevel = level;
}

void DeviceControl::disableBUHeater(){
	BULevel = 0;
	BUHeaterOff(true);
}

void DeviceControl::enablePump(){
#ifdef PUMP_LOW_LEVEL_TRIGGER
	bool update = ((mcpWriteBuffer & (1<<PUMP))>0);
	mcpWriteBuffer &= ~(1<<PUMP);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = !((mcpWriteBuffer & (1<<PUMP))>0);
	mcpWriteBuffer |= (1<<PUMP);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

void DeviceControl::disablePump(){
#ifdef PUMP_LOW_LEVEL_TRIGGER
	bool update = !((mcpWriteBuffer & (1<<PUMP))>0);
	mcpWriteBuffer |= (1<<PUMP);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = ((mcpWriteBuffer & (1<<PUMP))>0);
	mcpWriteBuffer &= ~(1<<PUMP);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

//make water flow to brewing unit instead of sink
void DeviceControl::enableBrewingValve(){
#ifdef BOILER_VALVE_LOW_LEVEL_TRIGGER
	bool update = ((mcpWriteBuffer & (1<<BU_VALVE))>0);
	mcpWriteBuffer &= ~(1<<BU_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = !((mcpWriteBuffer & (1<<BU_VALVE))>0);
	mcpWriteBuffer |= (1<<BU_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

//make water flow to sink instead of brewing unit
void DeviceControl::disableBrewingValve(){
#ifdef BOILER_VALVE_LOW_LEVEL_TRIGGER
	bool update = !((mcpWriteBuffer & (1<<BU_VALVE))>0);
	mcpWriteBuffer |= (1<<BU_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = ((mcpWriteBuffer & (1<<BU_VALVE))>0);
	mcpWriteBuffer &= ~(1<<BU_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

//make water flow into boiler
void DeviceControl::enableBoilerValve(){
#ifdef BOILER_VALVE_LOW_LEVEL_TRIGGER
	bool update = ((mcpWriteBuffer & (1<<BOILER_VALVE))>0);
	mcpWriteBuffer &= ~(1<<BOILER_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = !((mcpWriteBuffer & (1<<BOILER_VALVE))>0);
	mcpWriteBuffer |= (1<<BOILER_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

//make water flow to brewing heater
void DeviceControl::disableBoilerValve(){
#ifdef BOILER_VALVE_LOW_LEVEL_TRIGGER
	bool update = !((mcpWriteBuffer & (1<<BOILER_VALVE))>0);
	mcpWriteBuffer |= (1<<BOILER_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#else
	bool update = ((mcpWriteBuffer & (1<<BOILER_VALVE))>0);
	mcpWriteBuffer &= ~(1<<BOILER_VALVE);
	if(update)
		mcp.write(mcpWriteBuffer);
#endif
}

void DeviceControl::enableLEDLeft(){
	mcpWriteBuffer |= (1<<LED_LEFT);
}

void DeviceControl::disableLEDLeft(){
	mcpWriteBuffer &= ~(1<<LED_LEFT);
}

void DeviceControl::enableLEDRight(){
	mcpWriteBuffer |= (1<<LED_RIGHT);
}

void DeviceControl::disableLEDRight(){
	mcpWriteBuffer &= ~(1<<LED_RIGHT);
}

/// enable the water tank empty indicator LED
void DeviceControl::enableLEDTank(){
	mcpWriteBuffer |= (1<<LED_TANK);
}

/// disable the water tank empty indicator LED
void DeviceControl::disableLEDTank(){
	mcpWriteBuffer &= ~(1<<LED_TANK);
}

/// enable the power LED
void DeviceControl::enableLEDPower(){
	mcpWriteBuffer |= (1<<LED_POWER);
}

/// disable the power LED
void DeviceControl::disableLEDPower(){
	mcpWriteBuffer &= ~(1<<LED_POWER);
}

bool DeviceControl::getBoilerFull(){
//	return true; //todo delete
	return digitalRead(PROBE_DIGITAL_PIN);
}

bool DeviceControl::getTankFull(){
	//todo
	return (mcpReadBuffer & (1<<TANK_PIN))>0;
//	return true;
}

/**
 * returns the deprelled manual distribution switch state
 */
bool DeviceControl::getManualDistribution(){
	return buttonManDist->isPressed();
}

/**
 * Returns the deprelled volumetric distribution switch state
 */
bool DeviceControl::getVolumetricDistribution(){
	return buttonVolDist->isPressed();
}

bool DeviceControl::readButton1(){
	return (mcpReadBuffer & (1<<BUTTON_LEFT_PIN))>0;
}

bool DeviceControl::readButton2(){
	return (mcpReadBuffer & (1<<BUTTON_RIGHT_PIN))>0;
}

bool DeviceControl::readButtonVolDist(){
	return (mcpReadBuffer & (1<<BREW_VOL_PIN))>0;
}

bool DeviceControl::readButtonManDist(){
	return (mcpReadBuffer & (1<<BREW_MAN_PIN))>0;
}

/***
 * returns the debounced button 1 state
 */
bool DeviceControl::getButton1State(){
	return button1->isPressed();
}

/***
 * returns the debounced button 2 state
 */
bool DeviceControl::getButton2State(){
	return button2->isPressed();
}

//returns the total volume measured by the pump flowmeter since start in ml
double DeviceControl::getPumpVolume(){
	return pumpTicks * DataManager::getPumpTickToVolumeFactor();
}

//returns the total volume measured by the bypass flowmeter since start in ml
double DeviceControl::getBypassVolume(){
	return bypassTicks * DataManager::getBypassTickToVolumeFactor();
}

double	DeviceControl::getBoilerTemp(){
	return tsicBoiler->getTemperature();
}

double DeviceControl::getBUTemp(){
	return tsicBU->getTemperature();
}

double DeviceControl::getTubeTemp(){
	return tsicTube->getTemperature();
}

bool DeviceControl::getBoilerTempSensorError(){
	return tsicBoiler->sensorError();
}

bool DeviceControl::getBUTempSensorError(){
	return tsicBU->sensorError();
}

bool DeviceControl::getTubeTempSensorError(){
	return tsicTube->sensorError();
}

bool DeviceControl::getBoilerFillSensorError(){
	return boilerFillSensorError;
}

void DeviceControl::setBoilerFillSensorError(bool error){
	boilerFillSensorError = error;
}

/*** "Stupid" function for triggering the boiler heater SSR
 * @param send: if true and boiler heater had not been turned on before,
 * 				MCP23017 will be updated instantly
 */
void DeviceControl::boilerHeaterOn(bool send){
#ifdef BOILER_HEATER_LOW_LEVEL_TRIGGER
	bool update = ((mcpWriteBuffer & (1<<BOILER_HEATER))>0);
	mcpWriteBuffer &= ~(1<<BOILER_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#else
	bool update = !((mcpWriteBuffer & (1<<BOILER_HEATER))>0);
	mcpWriteBuffer |= (1<<BOILER_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#endif
}

/*** "Stupid" function for triggering the boiler heater SSR
 * @param send: if true and boiler heater had been turned on before,
 * 				MCP23017 will be updated instantly
 */
void DeviceControl::boilerHeaterOff(bool send){
#ifdef BOILER_HEATER_LOW_LEVEL_TRIGGER
	bool update = !((mcpWriteBuffer & (1<<BOILER_HEATER))>0);
	mcpWriteBuffer |= (1<<BOILER_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#else
	bool update = ((mcpWriteBuffer & (1<<BOILER_HEATER))>0);
	mcpWriteBuffer &= ~(1<<BOILER_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#endif
}

/*** "Stupid" function for triggering the brewing unit heater SSR
 * @param send: if true and BU heater had not been turned on before,
 * 				MCP23017 will be updated instantly
 *
 */
void DeviceControl::BUHeaterOn(bool send){
#ifdef BU_HEATER_LOW_LEVEL_TRIGGER
	bool update = ((mcpWriteBuffer & (1<<BU_HEATER))>0);
	mcpWriteBuffer &= ~(1<<BU_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#else
	bool update = !((mcpWriteBuffer & (1<<BU_HEATER))>0);
	mcpWriteBuffer |= (1<<BU_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#endif
}

/*** "Stupid" function for triggering the brewing unit heater SSR
 * @param send: if true and BU heater had been turned on before,
 * 				MCP23017 will be updated instantly
 *
 */
void DeviceControl::BUHeaterOff(bool send){
#ifdef BU_HEATER_LOW_LEVEL_TRIGGER
	bool update = !((mcpWriteBuffer & (1<<BU_HEATER))>0);
	mcpWriteBuffer |= (1<<BU_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#else
	bool update = ((mcpWriteBuffer & (1<<BU_HEATER))>0);
	mcpWriteBuffer &= ~(1<<BU_HEATER);
	if(update && send)
		mcp.write(mcpWriteBuffer);
#endif
}

void DeviceControl::enableDisplayBacklight(){
	digitalWrite(TFT_LED, LOW);
}

void DeviceControl::disableDisplayBacklight(){
	digitalWrite(TFT_LED, HIGH);
}
