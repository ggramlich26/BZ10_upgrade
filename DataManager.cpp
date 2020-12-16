/*
 * DataManager.cpp
 *
 *  Created on: Aug 21, 2020
 *      Author: tsugua
 */

#include "DataManager.h"
#include "DeviceControl.h"
#include "Webserver.h"
#include "WIFI_config.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Arduino.h"
#include "EEPROM.h"
#include <stdint.h>
#include <WidgetRTC.h>
#include <Time.h>


bool DataManager::blynkEnabled;
double DataManager::targetTempBoiler;
double DataManager::targetTempBU;
long DataManager::fillBoilerOverSondeTime = DEFAULT_FILL_BOILER_OVER_SONDE_TIME;
long DataManager::boilerMaxFillTime = DEFAULT_BOILER_FILL_MAX_TIME;
double DataManager::distributionVolume;
double DataManager::volumeOffset;
double DataManager::boilerControllerP;
double DataManager::BUControllerP;
bool DataManager::blynkInitialized;
unsigned long DataManager::lastWifiConnectTryTime;
int DataManager::preinfusionBuildupTime;
int DataManager::preinfusionWaitTime;
unsigned long DataManager::standbyWakeupTime;
int DataManager::standbyStartTime;
DeviceControl* DataManager::dev;
bool DataManager::scheduleRestart;
bool DataManager::standbyWakeupEnabled;
double DataManager::pumpTickToVolumeFactor;
double DataManager::bypassTickToVolumeFactor;

//	Blynk pins
// 	V1: Boiler temperature in °C
//	V2: BU temperature in °C
//	V3: Tube temperature in °C
//	V4: Boiler target temperature in °C
//	V5: BU target temperature in °C
//	V6: distribution volume in ml (integers only)
//	V7: volume offset in ml (integers only)
//	V8: Boiler P param
//	V9: BU P param
//	V10: Preinfusion buildup time in s (fractions are ok)
//	V11: Preinfusion wait time in s (fractions are ok)
//	V12: Standbye wakeup timer: in s after midnight (Blynk time widget), 0 to disable
//	V13: Standby start time: time after which the machine goes into standby mode if no user interaction occurs,
			//in s, 0 to disable (Blynk time widget)
//	V14: Pump flow sensor tick to volume factor
//	V15: Bypass flow sensor tick to volume factor

//	Flash address parameters
#define SSID_MAX_LEN				30
#define SSID_ADDR					0
#define WIFI_PW_MAX_LEN				30
#define WIFI_PW_ADDR				31
#define	HOST_NAME_MAX_LEN			30
#define	HOST_NAME_ADDR				62
#define	TARGET_TEMP_BOILER_ADDR		93
#define TARGET_TEMP_BOILER_LEN		sizeof(double)
#define	TARGET_TEMP_BU_ADDR			101
#define TARGET_TEMP_BU_LEN			sizeof(double)
#define	DIST_VOL_ADDR				109
#define	DIST_VOL_LEN				sizeof(double)
#define	VOL_OFFSET_ADDR				117
#define	VOL_OFFSET_LEN				sizeof(double)
#define	BOILER_CONTROLLER_P_ADDR	125
#define	BOILER_CONTROLLER_P_LEN		sizeof(double)
#define	BU_CONTROLLER_P_ADDR		133
#define	BU_CONTROLLER_P_LEN			sizeof(double)
#define	BLYNK_ENABLED_ADDR			141
#define	BLYNK_ENABLED_LEN			1
#define	PREINFUSION_BUILDUP_TIME_ADDR	142
#define	PREINFUSION_BUILDUP_TIME_LEN	sizeof(int)
#define	PREINFUSION_WAIT_TIME_ADDR		146
#define	PREINFUSION_WAIT_TIME_LEN		sizeof(int)
#define STANDBY_START_TIME_ADDR		150
#define	STANDBY_START_TIME_LEN		sizeof(int)
#define STANDBY_WAKEUP_TIME_ADDR	154
#define STANDBY_WAKEUP_TIME_LEN		sizeof(long)
#define PUMP_TICK_TO_VOL_FACTOR_ADDR	162
#define PUMP_TICK_TO_VOL_FACTOR_LEN		sizeof(double)
#define BYPASS_TICK_TO_VOL_FACTOR_ADDR	170
#define BYPASS_TICK_TO_VOL_FACTOR_LEN	sizeof(double)


#define CHECKSUM_ADDR				178
#define	CHECKSUM_LEN				4
#define	EEPROM_SIZE					200
char ssid[SSID_MAX_LEN+1];
char password[WIFI_PW_MAX_LEN+1];
char hostName[HOST_NAME_MAX_LEN+1];

char auth[] = BLYNK_AUTH_KEY;

BlynkTimer timer;
WidgetRTC rtc;


void DataManager::init(){
	dev = DeviceControl::instance();
	blynkInitialized = false;
	lastWifiConnectTryTime = 0;
	scheduleRestart = false;

	//read from flash/blynk (update each other) or use default values
	EEPROM.begin(EEPROM_SIZE);

	//read SSID and password from flash
	for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
		ssid[i] = EEPROM.read(SSID_ADDR+i);
	}
	for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
		password[i] = EEPROM.read(WIFI_PW_ADDR+i);
	}
	for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
		hostName[i] = EEPROM.read(HOST_NAME_ADDR+i);
	}
	uint32_t checksum = 0;
	for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
		checksum |= ((uint32_t)EEPROM.read(CHECKSUM_ADDR+i))<<(8*i);
	}

	eepromRead((uint8_t*)&targetTempBoiler, TARGET_TEMP_BOILER_ADDR, TARGET_TEMP_BOILER_LEN);
	eepromRead((uint8_t*)&targetTempBU, TARGET_TEMP_BU_ADDR, TARGET_TEMP_BU_LEN);
	eepromRead((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN);
	eepromRead((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN);
	uint8_t temp;
	eepromRead((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN);
	blynkEnabled = temp==0xFF;
	eepromRead((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN);
	eepromRead((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN);
	eepromRead((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN);
	eepromRead((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN);
	eepromRead((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN);
	eepromRead((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN);
	eepromRead((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN);
	standbyWakeupTime = 0;
	standbyWakeupEnabled = false;
//	Serial.println("Boiler target: " + String(targetTempBoiler));
//	Serial.println("BU target: " + String(targetTempBU));
//	Serial.println("distribution volume: " + String(distributionVolume));
//	Serial.println("volume offset: " + String(volumeOffset));
//	Serial.println("blynk enabled: " + String(blynkEnabled));
//	Serial.println("SSID: " + String(ssid));
//	Serial.println("password: " + String(password));

	//enter wifi setup mode
	if(dev->getManualDistribution() && dev->getButton1() && dev->getButton2()){
		WIFISetupMode();
	}
	//if wifi not intialized correctly, use default values
	else if(checksum != calculateWIFIChecksum()){
		//copy default SSID to EEPROM and to the ssid variable
		const char* default_ssid = DEFAULT_SSID;
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, default_ssid[i]);
			ssid[i] = default_ssid[i];
			if(default_ssid[i] == '\0'){
				break;
			}
		}
		//copy default password to EEPROM and to the password variable
		const char* default_pw = DEFAULT_PASSWORD;
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, default_pw[i]);
			password[i] = default_pw[i];
			if(default_pw[i] == '\0'){
				break;
			}
		}
		//copy default host name to EEPROM and to the hostName variable
		const char* default_host_name = DEFAULT_HOST_NAME;
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, default_host_name[i]);
			hostName[i] = default_host_name[i];
			if(default_host_name[i] == '\0'){
				break;
			}
		}
		checksum = calculateWIFIChecksum();
		for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
			EEPROM.write(CHECKSUM_ADDR+i, (uint8_t)(checksum>>(8*i)));
		}
		EEPROM.commit();
	}
	//if EEPROM not initialized yet, write default values
	if(isnan(targetTempBU) || isnan(targetTempBoiler) || isnan(distributionVolume) || isnan(volumeOffset) ||
			isnan(boilerControllerP) || isnan(BUControllerP) || isnan(preinfusionBuildupTime) ||
			isnan(preinfusionWaitTime) || isnan(standbyStartTime) || isnan(pumpTickToVolumeFactor) ||
			isnan(bypassTickToVolumeFactor)){
		Serial.println("Writing default values");
		targetTempBoiler = DEFAULT_TEMP_BOILER;
		targetTempBU = DEFAULT_TEMP_BU;
		distributionVolume = DEFAULT_DISTRIBUTION_VOLUME;
		volumeOffset = DEFAULT_VOLUME_OFFSET;
		blynkEnabled = DEFAULT_BLYNK_ENABLED;
		boilerControllerP = DEFAULT_BOILER_CONTROLLER_P;
		BUControllerP = DEFAULT_BU_CONTROLLER_P;
		preinfusionBuildupTime = DEFAULT_PREINFUSION_BUILDUP_TIME;
		preinfusionWaitTime = DEFAULT_PREINFUSION_WAIT_TIME;
		standbyStartTime = DEFAULT_STANDBY_START_TIME;
		standbyWakeupTime = DEFAULT_STANDBY_WAKEUP_TIME;
		pumpTickToVolumeFactor = DEFAULT_PUMP_TICK_TO_VOLUME_FACTOR;
		bypassTickToVolumeFactor = DEFAULT_BYP_TICK_TO_VOLUME_FACTOR;
		eepromWrite((uint8_t*)&targetTempBoiler, TARGET_TEMP_BOILER_ADDR, TARGET_TEMP_BOILER_LEN, false);
		eepromWrite((uint8_t*)&targetTempBU, TARGET_TEMP_BU_ADDR, TARGET_TEMP_BU_LEN, false);
		eepromWrite((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN, false);
		eepromWrite((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN, false);
		uint8_t temp = blynkEnabled?0xFF:0x00;
		eepromWrite((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN, false);
		eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, false);
		eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, false);
		eepromWrite((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN, false);
		eepromWrite((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN, false);
		eepromWrite((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN, false);
		eepromWrite((uint8_t*)&standbyWakeupTime, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN, false);
		eepromWrite((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN, false);
		eepromWrite((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN, false);
		EEPROM.commit();
	}

	if(DataManager::getBlynkEnabled()){
		WiFi.setHostname(hostName);
		WiFi.begin(ssid, password);
		lastWifiConnectTryTime = millis();
		delay(500);
		if(WiFi.isConnected()){
			initBlynk();
		}
	}
}

/// standard update routine
void DataManager::update(){
	if(DataManager::getBlynkEnabled() && !WiFi.isConnected() && millis() >= lastWifiConnectTryTime + WIFI_CONNECT_INTERVAL){
		WiFi.begin(ssid, password);
		lastWifiConnectTryTime = millis();
	}
	if(DataManager::getBlynkEnabled() && !blynkInitialized && WiFi.isConnected()){
		initBlynk();
	}
	if(DataManager::getBlynkEnabled() && blynkInitialized){
		Blynk.run();
		timer.run();
	}
}

/// sends the current boiler temperature to blynk, if enabled
void DataManager::pushTempBoiler(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() && blynkInitialized &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V1, temp);
	}
}

/// sends the current BU temperature to blynk, if enabled
void DataManager::pushTempBU(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() && blynkInitialized &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V2, temp);
	}
}

/// sends the current tube temperature to blynk, if enabled
void DataManager::pushTempTube(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() && blynkInitialized &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V3, temp);
	}
}


double DataManager::getTargetTempBoiler(){
	return targetTempBoiler;
}

double DataManager::getTargetTempBU(){
	return targetTempBU;
}

// saves a new target temperature for the boiler
// @param temp: new target temperature
// @param updateBlynk: updates the target temperature in blynk, if set to true and blynk is enabled
void DataManager::setTargetTempBoiler(double temp, bool updateBlynk){
	if(temp > MAX_TEMP_BOILER){
		temp = MAX_TEMP_BOILER;
	}
	else if(temp < MIN_TEMP_BOILER){
		temp = MIN_TEMP_BOILER;
	}
	if(targetTempBoiler == temp){
		return;
	}

	targetTempBoiler = temp;
	eepromWrite((uint8_t*)&targetTempBoiler, TARGET_TEMP_BOILER_ADDR, TARGET_TEMP_BOILER_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V4, targetTempBoiler);
	}
}

// saves a new target temperature for the brewing unit
// @param temp: new target temperature
// @param updateBlynk: updates the target temperature in blynk, if set to true and blynk is enabled
void DataManager::setTargetTempBU(double temp, bool updateBlynk){
	if(temp > MAX_TEMP_BU){
		temp = MAX_TEMP_BU;
	}
	else if(temp < MIN_TEMP_BU){
		temp = MIN_TEMP_BU;
	}
	if(targetTempBU == temp){
		return;
	}

	targetTempBU = temp;
	eepromWrite((uint8_t*)&targetTempBU, TARGET_TEMP_BU_ADDR, TARGET_TEMP_BU_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V5, targetTempBU);
	}
}

long DataManager::getFillBoilerOverSondeTime(){
	return fillBoilerOverSondeTime;
}

long DataManager::getBoilerMaxFillTime(){
	return boilerMaxFillTime;
}

double DataManager::getDistributionVolume(){
	return distributionVolume;
}

/// saves a new target distribution volume
// @param volume: the new target volume
// @param updateBlynk: sends the new value to blynk, if set to true and blynk is enabled
void DataManager::setDistributionVolume(double volume, bool updateBlynk){
	if(volume > MAX_DISTRIBUTION_VOLUME) volume = MAX_DISTRIBUTION_VOLUME;
	else if(volume < MIN_DISTRIBUTION_VOLUME) volume = MIN_DISTRIBUTION_VOLUME;
	if(distributionVolume == volume){
		return;
	}

	distributionVolume = volume;
	eepromWrite((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V6, distributionVolume);
	}
}

double DataManager::getVolumeOffset(){
	return volumeOffset;
}

/// saves a new volume offset
// @param offset: the new volume offset
// @param updateBlynk: sends the new value to blynk, if set to true and blynk is enabled
void DataManager::setVolumeOffset(double offset, bool updateBlynk){
	if(offset > MAX_VOLUME_OFFSET) offset = MAX_VOLUME_OFFSET;
	if(offset < MIN_VOLUME_OFFSET) offset = MIN_VOLUME_OFFSET;
	if(offset == volumeOffset){
		return;
	}
	volumeOffset = offset;
	eepromWrite((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V7, volumeOffset);
	}
}

bool DataManager::getBlynkEnabled(){
	return blynkEnabled;
}

void DataManager::setBlynkEnabled(bool enabled){
	if(enabled && !blynkInitialized && !WiFi.isConnected()){
		WiFi.setHostname(hostName);
		WiFi.begin(ssid, password);
		lastWifiConnectTryTime = millis();
	}
	blynkEnabled = enabled;
	uint8_t temp = blynkEnabled?0xFF:0x00;
	eepromWrite((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN, true);
}

double DataManager::getBoilerControllerP(){
	return boilerControllerP;
}

/// saves a new boiler controller P parameter
// @param p: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBoilerControllerP(double p, bool updateBlynk){
	if(p < MIN_BOILER_CONTROLLER_P) p = MIN_BOILER_CONTROLLER_P;
	else if(p > MAX_BOILER_CONTROLLER_P) p = MAX_BOILER_CONTROLLER_P;
	if(boilerControllerP == p) return;
	boilerControllerP = p;
	eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V8, boilerControllerP);
	}
}

double DataManager::getBUControllerP(){
	return BUControllerP;
}

/// saves a new BU controller P parameter
// @param p: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBUControllerP(double p, bool updateBlynk){
	if(p < MIN_BU_CONTROLLER_P) p = MIN_BU_CONTROLLER_P;
	else if(p > MAX_BU_CONTROLLER_P) p = MAX_BU_CONTROLLER_P;
	if(BUControllerP == p) return;
	BUControllerP = p;
	eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V9, BUControllerP);
	}

}

double DataManager::getPumpTickToVolumeFactor(){
	return pumpTickToVolumeFactor;
}

/// saves a new pump flow meter tick to volume conversion factor
// @param f: the new factor
// @param updateBlynk: sends the new factor to blynk, if set to true and blynk is enabled
void DataManager::setPumpTickToVolumeFactor(double f, bool updateBlynk){
	if(f < MIN_TICK_TO_VOLUME_FACTOR) f = MIN_TICK_TO_VOLUME_FACTOR;
	else if(f > MAX_TICK_TO_VOLUME_FACTOR) f = MAX_TICK_TO_VOLUME_FACTOR;
	if(pumpTickToVolumeFactor == f) return;
	pumpTickToVolumeFactor = f;
	eepromWrite((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V14, pumpTickToVolumeFactor);
	}
}

double DataManager::getBypassTickToVolumeFactor(){
	return bypassTickToVolumeFactor;
}

/// saves a new bypass flow meter tick to volume conversion factor
// @param f: the new factor
// @param updateBlynk: sends the new factor to blynk, if set to true and blynk is enabled
void DataManager::setBypassTickToVolumeFactor(double f, bool updateBlynk){
	if(f < MIN_TICK_TO_VOLUME_FACTOR) f = MIN_TICK_TO_VOLUME_FACTOR;
	else if(f > MAX_TICK_TO_VOLUME_FACTOR) f = MAX_TICK_TO_VOLUME_FACTOR;
	if(bypassTickToVolumeFactor == f) return;
	bypassTickToVolumeFactor = f;
	eepromWrite((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V15, bypassTickToVolumeFactor);
	}
}

/// initialize Blynk by creating the connection and writing current values
void DataManager::initBlynk(){
	Blynk.begin(auth, ssid, password);
	//write current values to cloud
	Blynk.virtualWrite(V4, targetTempBoiler);
	Blynk.virtualWrite(V5, targetTempBU);
	Blynk.virtualWrite(V6, distributionVolume);
	Blynk.virtualWrite(V7, volumeOffset);
	Blynk.virtualWrite(V8, boilerControllerP);
	Blynk.virtualWrite(V9, BUControllerP);
	Blynk.virtualWrite(V10, preinfusionBuildupTime/1000);
	Blynk.virtualWrite(V11, preinfusionWaitTime/1000);
	//reading from EEPROM: stored in s after midnight
	long time = 0;
	eepromRead((uint8_t*)&time, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN);
	if(time >= 0){
		Blynk.virtualWrite(V12, time, 0, "Europe/Berlin");
		//conversion to machine time
		long realTime = ((long)hour()*60*60+minute()*60+second());
		if(time < realTime){
			time += 24*60*60;
		}
		standbyWakeupTime = millis()+(time-realTime)*1000;
		while(standbyWakeupTime < millis())
			standbyWakeupTime += 24*60*60*1000;	//increment wakeup time until it causes and immediate wakeup
												//or is past the current system time
		standbyWakeupEnabled = true;
	}
	else{
		standbyWakeupEnabled = false;
		Blynk.virtualWrite(V12, "", 0, "Europe/Berlin");
	}
	if(standbyStartTime > 0)
		Blynk.virtualWrite(V13, standbyStartTime/1000, 0, "Europe/Berlin");
	else
		Blynk.virtualWrite(V13, "", 0, "Europe/Berlin");

	setSyncInterval(10 * 60); // Sync interval for RTC in seconds (10 minutes)
	Blynk.virtualWrite(V14, pumpTickToVolumeFactor);
	Blynk.virtualWrite(V15, bypassTickToVolumeFactor);

	blynkInitialized = true;
}

bool DataManager::getWifiConnected(){
	return WiFi.isConnected();
}

int DataManager::getPreinfusionBuildupTime(){
	return preinfusionBuildupTime;
}

/// sets the preinfusion pressure buildup time
// @param time time in ms, 0 to disable preinfusion
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setPreinfusionBuildupTime(int time, bool updateBlynk){
	if(time > MAX_PREINFUSION_BUILDUP_TIME) time = MAX_PREINFUSION_BUILDUP_TIME;
	else if(time < 0) time = 0;
	if(preinfusionBuildupTime == time) return;
	preinfusionBuildupTime = time;
	eepromWrite((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V10, preinfusionBuildupTime/1000);
	}
}

int DataManager::getPreinfusionWaitTime(){
	return preinfusionWaitTime;
}

/// sets the preinfusion waiting time
// @param time time in ms
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setPreinfusionWaitTime(int time, bool updateBlynk){
	if(time > MAX_PREINFUSION_WAIT_TIME) time = MAX_PREINFUSION_WAIT_TIME;
	else if(time < 0) time = 0;
	if(preinfusionWaitTime == time) return;
	preinfusionWaitTime = time;
	eepromWrite((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V11, preinfusionWaitTime/1000);
	}
}

/// returns the time in ms after system start when the machine should wake up from standby
//	Always %(24*60*60*1000) = 1 day.
unsigned long DataManager::getStandbyWakeupTime(){
	return standbyWakeupTime;
}

/// sets the time when the machine should wake up from standby
// @param time time in s after midnight, -1 to disable
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setStandbyWakeupTime(long time, bool updateBlynk){
	eepromWrite((uint8_t*)&time, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN, true);
	long realTime = ((long)hour()*60*60+minute()*60+second());
	//disable wakeup if -1
	if(time >= 0){
		//if it is later than the wakeup time, add one day to it
		if(time < realTime){
			time += 24*60*60;
		}
		standbyWakeupTime = millis()+(time-realTime)*1000;
		while(standbyWakeupTime < millis())
			standbyWakeupTime += 24*60*60*1000;	//increment wakeup time until it causes and immediate wakeup
												//or is past the current system time
		standbyWakeupEnabled = true;
	}
	else{
		standbyWakeupEnabled = false;
	}
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		if(time >= 0)
			Blynk.virtualWrite(V12, time%(24*60*60), 0, "Europe/Berlin");
		else
			Blynk.virtualWrite(V12, "", 0, "Europe/Berlin");
	}
}

bool DataManager::getStandbyWakeupEnabled(){
	return standbyWakeupEnabled;
}

void DataManager::incStandbyWakeupTimeByOneDay(){
	standbyWakeupTime += 24*60*60*1000;
}

/// sets the time after which the machine will go in standby mode if no user interaction occurs
// @param time time in ms
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setStandbyStartTime(int time, bool updateBlynk){
	if(time < 0) time = 0;
	if(standbyStartTime == time) return;
	standbyStartTime = time;
	eepromWrite((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN, true);
	if(getBlynkEnabled() && blynkInitialized && updateBlynk){
		Blynk.virtualWrite(V13, standbyStartTime/1000);
	}
}

/// returns the time in ms after which the machine goes into standby if no user interaction occurs
int DataManager::getStandbyStartTime(){
	return standbyStartTime;
}


//////////////////////////////////////////////////////////////////
//																//
//				Blynk functions									//
//																//
//////////////////////////////////////////////////////////////////

BLYNK_WRITE(V4){
	DataManager::setTargetTempBoiler(param.asDouble(), false);
}

BLYNK_WRITE(V5){
	DataManager::setTargetTempBU(param.asDouble(), false);
}

BLYNK_WRITE(V6){
	DataManager::setDistributionVolume(param.asDouble(), false);
}

BLYNK_WRITE(V7){
	DataManager::setVolumeOffset(param.asDouble(), false);
}

BLYNK_WRITE(V8){
	DataManager::setBoilerControllerP(param.asDouble(), false);
}

BLYNK_WRITE(V9){
	DataManager::setBUControllerP(param.asDouble(), false);
}

BLYNK_WRITE(V10){
	DataManager::setPreinfusionBuildupTime((int)(param.asDouble()*1000), false);
}

BLYNK_WRITE(V11){
	DataManager::setPreinfusionWaitTime((int)(param.asDouble()*1000), false);
}

BLYNK_WRITE(V12){
	String str = param.asString();
	if(str.equals(""))
		DataManager::setStandbyWakeupTime(-1, false);
	else
		DataManager::setStandbyWakeupTime(param.asLong(), false);
}

BLYNK_WRITE(V13){
	DataManager::setStandbyStartTime(param.asInt()*1000, false);
}

BLYNK_WRITE(V14){
	DataManager::setPumpTickToVolumeFactor(param.asDouble(), false);
}

BLYNK_WRITE(V15){
	DataManager::setBypassTickToVolumeFactor(param.asDouble(), false);
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

//////////////////////////////////////////////////////////////////
//																//
//				EEPROM functions								//
//																//
//////////////////////////////////////////////////////////////////


/// used to make sure valid data is read from EEPROM
uint32_t DataManager::calculateWIFIChecksum(){
	uint32_t res = 0;
	for(uint8_t i = 0, j = 0; i < SSID_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(ssid[i])<<(8*j));
		if(ssid[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < WIFI_PW_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(password[i])<<(8*j));
		if(password[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < HOST_NAME_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(hostName[i])<<(8*j));
		if(hostName[i] == '\0'){
			break;
		}
	}
	return res;
}

/// reads data from the EEPROM
// @param dst: memory address, where the data read is to be stored
// @param addr: EEPROM start address
// @param len: length of the data to be read in bytes
void DataManager::eepromRead(uint8_t *dst, int addr, int len){
	if(len <= 0){
		return;
	}
	for(int i = 0; i < len; i++){
		*(dst++) = EEPROM.read(addr++);
	}
}

/// writes data to the EEPROM
// @param src: memory address, where the data to be written is to be stored
// @param addr: EEPROM start address
// @param len: length of the data to be written in bytes
// @param commit: will finish the EEPROM write with a commit, if true.
//					Only performing such a commit will make the data being actually written
void DataManager::eepromWrite(uint8_t *src, int addr, int len, bool commit){
	if(len < 0){
		return;
	}
	for(int i = 0; i < len; i++){
		EEPROM.write(addr++, *(src++));
	}
	if(commit){
		EEPROM.commit();
	}
}

/**
 * Enters wifi setup mode: machine will create an access point with default SSID and password.
 * On the machine IP address (192.168.4.1) will be a webserver running to set up the new WIFI credentials
 * No regular operation will be possible in this mode
 */
void DataManager::WIFISetupMode(){
	WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);
	webserver_init();
	while(1){
		dev->enableLEDRight();
		dev->update();
		delay(500);
		dev->disableLEDRight();
		dev->update();
		delay(500);
		if(scheduleRestart){
			scheduleRestart = false;
			delay(1000);
			ESP.restart();
		}
	}
}

String DataManager::setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName){
	if(newSSID != NULL && strcmp(newSSID, "") != 0){
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, newSSID[i]);
			ssid[i] = newSSID[i];
			if(newSSID[i] == '\0'){
				break;
			}
		}
	}
	if(newPassword != NULL && strcmp(newPassword, "") != 0){
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, newPassword[i]);
			password[i] = newPassword[i];
			if(newPassword[i] == '\0'){
				break;
			}
		}
	}
	if(newHostName != NULL && strcmp(newHostName, "") != 0){
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, newHostName[i]);
			hostName[i] = newHostName[i];
			if(newHostName[i] == '\0'){
				break;
			}
		}
	}
	uint32_t checksum = calculateWIFIChecksum();
	for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
		EEPROM.write(CHECKSUM_ADDR+i, (uint8_t)(checksum>>(8*i)));
	}
	EEPROM.commit();
	scheduleRestart = true;
	return "Successfully set new WIFI credentials. You can change them again by restarting your machine while having both "
			"buttons pressed and the distribution switch set to manual distribution.";
}
