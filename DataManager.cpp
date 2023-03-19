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
#include "WifiManager.h"

#include <WiFi.h>
#include <WiFiClient.h>

#include "Arduino.h"
#include "EEPROM.h"
#include <stdint.h>
#include <Time.h>


double DataManager::targetTempBoiler;
double DataManager::targetTempBU;
long DataManager::fillBoilerOverSondeTime = DEFAULT_FILL_BOILER_OVER_SONDE_TIME;
long DataManager::boilerMaxFillTime = DEFAULT_BOILER_FILL_MAX_TIME;
double DataManager::distributionVolume;
double DataManager::volumeOffset;
double DataManager::boilerControllerP;
double DataManager::boilerControllerI;
double DataManager::boilerControllerD;
double DataManager::BUControllerP;
double DataManager::BUControllerI;
double DataManager::BUControllerD;
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
char DataManager::wifiSsid[SSID_MAX_LEN+1];
char DataManager::wifiPassword[WIFI_PW_MAX_LEN+1];
char DataManager::wifiHostName[HOST_NAME_MAX_LEN+1];
bool DataManager::wifiEnabled;
char DataManager::language[LANG_MAX_LEN+1];
char DataManager::bonjourName[BONJOUR_NAME_MAX_LEN+1];
bool DataManager::standbyWakeupTimeConverted;
WifiManager *DataManager::wifiMan;


//	Flash address parameters
#define SSID_ADDR					0
#define WIFI_PW_ADDR				31
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
#define BOILER_CONTROLLER_I_ADDR		178
#define BOILER_CONTROLLER_I_LEN			sizeof(double)
#define BOILER_CONTROLLER_D_ADDR		186
#define BOILER_CONTROLLER_D_LEN			sizeof(double)
#define	BU_CONTROLLER_I_ADDR			194
#define BU_CONTROLLER_I_LEN				sizeof(double)
#define	BU_CONTROLLER_D_ADDR			202
#define BU_CONTROLLER_D_LEN				sizeof(double)
#define BONJOUR_NAME_ADDR				210
#define	LANGUAGE_ADDR					241

#define CHECKSUM_ADDR					247
#define	CHECKSUM_LEN					4
#define	EEPROM_SIZE						300


void DataManager::init(){
	dev = DeviceControl::instance();
	wifiMan = WifiManager::instance();
	lastWifiConnectTryTime = 0;
	scheduleRestart = false;
	standbyWakeupTimeConverted = false;

	//read from flash/blynk (update each other) or use default values
	EEPROM.begin(EEPROM_SIZE);

	//read SSID and password from flash
	for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
		wifiSsid[i] = EEPROM.read(SSID_ADDR+i);
	}
	for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
		wifiPassword[i] = EEPROM.read(WIFI_PW_ADDR+i);
	}
	for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
		wifiHostName[i] = EEPROM.read(HOST_NAME_ADDR+i);
	}
	uint32_t checksum = 0;
	for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
		checksum |= ((uint32_t)EEPROM.read(CHECKSUM_ADDR+i))<<(8*i);
	}
	for(uint8_t i = 0; i < LANG_MAX_LEN+1; i++){
		language[i] = EEPROM.read(LANGUAGE_ADDR+i);
	}
	for(uint8_t i = 0; i < BONJOUR_NAME_MAX_LEN+1; i++){
		bonjourName[i] = EEPROM.read(BONJOUR_NAME_ADDR+i);
	}

	eepromRead((uint8_t*)&targetTempBoiler, TARGET_TEMP_BOILER_ADDR, TARGET_TEMP_BOILER_LEN);
	eepromRead((uint8_t*)&targetTempBU, TARGET_TEMP_BU_ADDR, TARGET_TEMP_BU_LEN);
	eepromRead((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN);
	eepromRead((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN);
	uint8_t temp;
	eepromRead((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN);
	wifiEnabled = temp==0xFF;
	eepromRead((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN);
	eepromRead((uint8_t*)&boilerControllerI, BOILER_CONTROLLER_I_ADDR, BOILER_CONTROLLER_I_LEN);
	eepromRead((uint8_t*)&boilerControllerD, BOILER_CONTROLLER_D_ADDR, BOILER_CONTROLLER_D_LEN);
	eepromRead((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN);
	eepromRead((uint8_t*)&BUControllerI, BU_CONTROLLER_I_ADDR, BU_CONTROLLER_I_LEN);
	eepromRead((uint8_t*)&BUControllerD, BU_CONTROLLER_D_ADDR, BU_CONTROLLER_D_LEN);
	eepromRead((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN);
	eepromRead((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN);
	eepromRead((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN);
	eepromRead((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN);
	eepromRead((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN);
	eepromRead((uint8_t*)&standbyWakeupTime, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN);
	standbyWakeupEnabled = false;
	convertStandbyWakeupTimeToMachineTime();
//	Serial.println("Boiler target: " + String(targetTempBoiler));
//	Serial.println("BU target: " + String(targetTempBU));
//	Serial.println("distribution volume: " + String(distributionVolume));
//	Serial.println("volume offset: " + String(volumeOffset));
//	Serial.println("blynk enabled: " + String(blynkEnabled));
//	Serial.println("SSID: " + String(ssid));
//	Serial.println("password: " + String(password));

	bool resetWifiCredentials = false;
	//reset wifi credentials
	if(dev->readButtonManDist() && dev->readButton1() && dev->readButton2()){
		//resetWifiCredentials = true; //todo: uncomment this line
		Serial.println("Resetting WiFi credentials");
	}
	//if wifi not intialized correctly, use default values
	if(resetWifiCredentials || checksum != calculateWIFIChecksum()){
		//copy default SSID to EEPROM and to the ssid variable
		const char* default_ssid = DEFAULT_SSID;
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, default_ssid[i]);
			wifiSsid[i] = default_ssid[i];
			if(default_ssid[i] == '\0'){
				break;
			}
		}
		//copy default password to EEPROM and to the password variable
		const char* default_pw = DEFAULT_PASSWORD;
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, default_pw[i]);
			wifiPassword[i] = default_pw[i];
			if(default_pw[i] == '\0'){
				break;
			}
		}
		//copy default host name to EEPROM and to the hostName variable
		const char* default_host_name = DEFAULT_HOST_NAME;
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, default_host_name[i]);
			wifiHostName[i] = default_host_name[i];
			if(default_host_name[i] == '\0'){
				break;
			}
		}
		checksum = calculateWIFIChecksum();
		for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
			EEPROM.write(CHECKSUM_ADDR+i, (uint8_t)(checksum>>(8*i)));
		}
		EEPROM.commit();
		const char* default_language = DEFAULT_LANGUAGE;
		setLanguage(default_language);
		const char* default_bonjourName = DEFAULT_BONJOUR_NAME;
		setBonjourName(default_bonjourName);
	}
	//if EEPROM not initialized yet, write default values
	if(isnan(targetTempBU) || isnan(targetTempBoiler) || isnan(distributionVolume) || isnan(volumeOffset) ||
			isnan(boilerControllerP) || isnan(boilerControllerI) || isnan(boilerControllerD) ||
			isnan(BUControllerP) || isnan(BUControllerI) || isnan(BUControllerD) ||
			isnan(preinfusionBuildupTime) || isnan(preinfusionWaitTime) || isnan(standbyStartTime) ||
			isnan(pumpTickToVolumeFactor) || isnan(bypassTickToVolumeFactor)){
		Serial.println("Writing default values");
		targetTempBoiler = DEFAULT_TEMP_BOILER;
		targetTempBU = DEFAULT_TEMP_BU;
		distributionVolume = DEFAULT_DISTRIBUTION_VOLUME;
		volumeOffset = DEFAULT_VOLUME_OFFSET;
		wifiEnabled = DEFAULT_BLYNK_ENABLED;
		boilerControllerP = DEFAULT_BOILER_CONTROLLER_P;
		boilerControllerI = DEFAULT_CONTROLLER_I;
		boilerControllerD = DEFAULT_CONTROLLER_D;
		BUControllerP = DEFAULT_BU_CONTROLLER_P;
		BUControllerI = DEFAULT_CONTROLLER_I;
		BUControllerD = DEFAULT_CONTROLLER_D;
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
		uint8_t temp = wifiEnabled?0xFF:0x00;
		eepromWrite((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN, false);
		eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, false);
		eepromWrite((uint8_t*)&boilerControllerI, BOILER_CONTROLLER_I_ADDR, BOILER_CONTROLLER_I_LEN, false);
		eepromWrite((uint8_t*)&boilerControllerD, BOILER_CONTROLLER_D_ADDR, BOILER_CONTROLLER_D_LEN, false);
		eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, false);
		eepromWrite((uint8_t*)&BUControllerI, BU_CONTROLLER_I_ADDR, BU_CONTROLLER_I_LEN, false);
		eepromWrite((uint8_t*)&BUControllerD, BU_CONTROLLER_D_ADDR, BU_CONTROLLER_D_LEN, false);
		eepromWrite((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN, false);
		eepromWrite((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN, false);
		eepromWrite((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN, false);
		eepromWrite((uint8_t*)&standbyWakeupTime, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN, false);
		convertStandbyWakeupTimeToMachineTime();
		eepromWrite((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN, false);
		eepromWrite((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN, false);
		EEPROM.commit();
	}
}

/// standard update routine
void DataManager::update(){
	if (scheduleRestart){
		scheduleRestart = false;
		delay(1000);
		ESP.restart();
	}
	if(!standbyWakeupTimeConverted)
		convertStandbyWakeupTimeToMachineTime();
}

/// sends the current boiler temperature to blynk, if enabled
void DataManager::pushTempBoiler(double temp){
//	static long lastUpdateTime = 0;
//	if(DataManager::getBlynkEnabled() && blynkInitialized &&
//			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
//					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
//		lastUpdateTime = millis();
//		Blynk.virtualWrite(V1, temp);
//	}
}

/// sends the current BU temperature to blynk, if enabled
void DataManager::pushTempBU(double temp){
//	static long lastUpdateTime = 0;
//	if(DataManager::getBlynkEnabled() && blynkInitialized &&
//			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
//					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
//		lastUpdateTime = millis();
//		Blynk.virtualWrite(V2, temp);
//	}
}

/// sends the current tube temperature to blynk, if enabled
void DataManager::pushTempTube(double temp){
//	static long lastUpdateTime = 0;
//	if(DataManager::getBlynkEnabled() && blynkInitialized &&
//			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
//					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
//		lastUpdateTime = millis();
//		Blynk.virtualWrite(V3, temp);
//	}
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
void DataManager::setTargetTempBoiler(double temp){
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
}

// saves a new target temperature for the brewing unit
// @param temp: new target temperature
// @param updateBlynk: updates the target temperature in blynk, if set to true and blynk is enabled
void DataManager::setTargetTempBU(double temp){
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
void DataManager::setDistributionVolume(double volume){
	if(volume > MAX_DISTRIBUTION_VOLUME) volume = MAX_DISTRIBUTION_VOLUME;
	else if(volume < MIN_DISTRIBUTION_VOLUME) volume = MIN_DISTRIBUTION_VOLUME;
	if(distributionVolume == volume){
		return;
	}

	distributionVolume = volume;
	eepromWrite((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN, true);
}

double DataManager::getVolumeOffset(){
	return volumeOffset;
}

/// saves a new volume offset
// @param offset: the new volume offset
// @param updateBlynk: sends the new value to blynk, if set to true and blynk is enabled
void DataManager::setVolumeOffset(double offset){
	if(offset > MAX_VOLUME_OFFSET) offset = MAX_VOLUME_OFFSET;
	if(offset < MIN_VOLUME_OFFSET) offset = MIN_VOLUME_OFFSET;
	if(offset == volumeOffset){
		return;
	}
	volumeOffset = offset;
	eepromWrite((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN, true);
}

bool DataManager::getWifiEnabled(){
	return wifiEnabled;
}

void DataManager::setWifiEnabled(bool enabled){
	wifiEnabled = enabled;
	uint8_t temp = wifiEnabled?0xFF:0x00;
	eepromWrite((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN, true);
}

double DataManager::getBoilerControllerP(){
	return boilerControllerP;
}

/// saves a new boiler controller P parameter
// @param p: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBoilerControllerP(double p){
	if(p < MIN_BOILER_CONTROLLER_P) p = MIN_BOILER_CONTROLLER_P;
	else if(p > MAX_BOILER_CONTROLLER_P) p = MAX_BOILER_CONTROLLER_P;
	if(boilerControllerP == p) return;
	boilerControllerP = p;
	eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, true);
}

double DataManager::getBoilerControllerI(){
	return boilerControllerI;
}

/// saves a new boiler controller I parameter
// @param i: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBoilerControllerI(double i){
	if(i < MIN_CONTROLLER_I) i = MIN_CONTROLLER_I;
	else if(i > MAX_CONTROLLER_I) i = MAX_CONTROLLER_I;
	if(boilerControllerI == i) return;
	boilerControllerI = i;
	eepromWrite((uint8_t*)&boilerControllerI, BOILER_CONTROLLER_I_ADDR, BOILER_CONTROLLER_I_LEN, true);
}

double DataManager::getBoilerControllerD(){
	return boilerControllerD;
}

/// saves a new boiler controller D parameter
// @param d: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBoilerControllerD(double d){
	if(d < MIN_CONTROLLER_D) d = MIN_CONTROLLER_D;
	else if(d > MAX_CONTROLLER_D) d = MAX_CONTROLLER_D;
	if(boilerControllerD == d) return;
	boilerControllerD = d;
	eepromWrite((uint8_t*)&boilerControllerD, BOILER_CONTROLLER_D_ADDR, BOILER_CONTROLLER_D_LEN, true);
}

double DataManager::getBUControllerP(){
	return BUControllerP;
}

/// saves a new BU controller P parameter
// @param p: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBUControllerP(double p){
	if(p < MIN_BU_CONTROLLER_P) p = MIN_BU_CONTROLLER_P;
	else if(p > MAX_BU_CONTROLLER_P) p = MAX_BU_CONTROLLER_P;
	if(BUControllerP == p) return;
	BUControllerP = p;
	eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, true);
}

double DataManager::getBUControllerI(){
	return BUControllerI;
}

/// saves a new BU controller I parameter
// @param i: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBUControllerI(double i){
	if(i < MIN_CONTROLLER_I) i = MIN_CONTROLLER_I;
	else if(i > MAX_CONTROLLER_I) i = MAX_CONTROLLER_I;
	if(BUControllerI == i) return;
	BUControllerI = i;
	eepromWrite((uint8_t*)&BUControllerI, BU_CONTROLLER_I_ADDR, BU_CONTROLLER_I_LEN, true);
}

double DataManager::getBUControllerD(){
	return BUControllerD;
}

/// saves a new BU controller D parameter
// @param d: the new parameter
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setBUControllerD(double d){
	if(d < MIN_CONTROLLER_D) d = MIN_CONTROLLER_D;
	else if(d > MAX_CONTROLLER_D) d = MAX_CONTROLLER_D;
	if(BUControllerD == d) return;
	BUControllerD = d;
	eepromWrite((uint8_t*)&BUControllerD, BU_CONTROLLER_D_ADDR, BU_CONTROLLER_D_LEN, true);
}

double DataManager::getPumpTickToVolumeFactor(){
	return pumpTickToVolumeFactor;
}

/// saves a new pump flow meter tick to volume conversion factor
// @param f: the new factor
// @param updateBlynk: sends the new factor to blynk, if set to true and blynk is enabled
void DataManager::setPumpTickToVolumeFactor(double f){
	if(f < MIN_TICK_TO_VOLUME_FACTOR) f = MIN_TICK_TO_VOLUME_FACTOR;
	else if(f > MAX_TICK_TO_VOLUME_FACTOR) f = MAX_TICK_TO_VOLUME_FACTOR;
	if(pumpTickToVolumeFactor == f) return;
	pumpTickToVolumeFactor = f;
	eepromWrite((uint8_t*)&pumpTickToVolumeFactor, PUMP_TICK_TO_VOL_FACTOR_ADDR, PUMP_TICK_TO_VOL_FACTOR_LEN, true);
}

double DataManager::getBypassTickToVolumeFactor(){
	return bypassTickToVolumeFactor;
}

/// saves a new bypass flow meter tick to volume conversion factor
// @param f: the new factor
// @param updateBlynk: sends the new factor to blynk, if set to true and blynk is enabled
void DataManager::setBypassTickToVolumeFactor(double f){
	if(f < MIN_TICK_TO_VOLUME_FACTOR) f = MIN_TICK_TO_VOLUME_FACTOR;
	else if(f > MAX_TICK_TO_VOLUME_FACTOR) f = MAX_TICK_TO_VOLUME_FACTOR;
	if(bypassTickToVolumeFactor == f) return;
	bypassTickToVolumeFactor = f;
	eepromWrite((uint8_t*)&bypassTickToVolumeFactor, BYPASS_TICK_TO_VOL_FACTOR_ADDR, BYPASS_TICK_TO_VOL_FACTOR_LEN, true);
}

bool DataManager::getWifiConnected(){
	return wifiMan->connected();
}

bool DataManager::getHotspotMode(){
	return wifiMan->hotspotMode();
}

int DataManager::getPreinfusionBuildupTime(){
	return preinfusionBuildupTime;
}

/// sets the preinfusion pressure buildup time
// @param time time in ms, 0 to disable preinfusion
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setPreinfusionBuildupTime(int time){
	if(time > MAX_PREINFUSION_BUILDUP_TIME) time = MAX_PREINFUSION_BUILDUP_TIME;
	else if(time < 0) time = 0;
	if(preinfusionBuildupTime == time) return;
	preinfusionBuildupTime = time;
	eepromWrite((uint8_t*)&preinfusionBuildupTime, PREINFUSION_BUILDUP_TIME_ADDR, PREINFUSION_BUILDUP_TIME_LEN, true);
}

int DataManager::getPreinfusionWaitTime(){
	return preinfusionWaitTime;
}

/// sets the preinfusion waiting time
// @param time time in ms
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setPreinfusionWaitTime(int time){
	if(time > MAX_PREINFUSION_WAIT_TIME) time = MAX_PREINFUSION_WAIT_TIME;
	else if(time < 0) time = 0;
	if(preinfusionWaitTime == time) return;
	preinfusionWaitTime = time;
	eepromWrite((uint8_t*)&preinfusionWaitTime, PREINFUSION_WAIT_TIME_ADDR, PREINFUSION_WAIT_TIME_LEN, true);
}

/// returns the time in ms after system start when the machine should wake up from standby
//	Always %(24*60*60*1000) = 1 day.
unsigned long DataManager::getStandbyWakeupTime(){
	return standbyWakeupTime;
}

/// returns the standby wakeup time in the format hh:mm
String DataManager::getStandbyWakeupTimeString(){
	unsigned long time;
	eepromRead((uint8_t*)&time, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN);
	int hour = time/60/60;
	int minute = (time/60)%60;
	return String(hour/10) + String(hour%10) + ":" + String(minute/10) + String(minute%10);
}

void DataManager::setStandbyWakeupTime(const char* time){
	if(time[2] != ':')
		return;
	int hour = atoi(time);
	int minute = atoi(time+3);
	if(hour >= 24 || hour < 0 || minute >= 60 || minute < 0)
		return;
	long numberTime = hour*60*60 + minute*60;
	setStandbyWakeupTime(numberTime);
}

/// sets the time when the machine should wake up from standby
// @param time time in s after midnight, -1 to disable
void DataManager::setStandbyWakeupTime(long time){
	eepromWrite((uint8_t*)&time, STANDBY_WAKEUP_TIME_ADDR, STANDBY_WAKEUP_TIME_LEN, true);
	standbyWakeupTime = time;
	convertStandbyWakeupTimeToMachineTime();
}

void DataManager::convertStandbyWakeupTimeToMachineTime(){
	unsigned long time = standbyWakeupTime;
	long realTime = WifiManager::instance()->getRTCTime();
	if(realTime < 0)
		return;
	//disable wakeup if 00:00
	if(time > 0){
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
	standbyWakeupTimeConverted = true;
}

bool DataManager::getStandbyWakeupEnabled(){
	return standbyWakeupEnabled;
}

void DataManager::incStandbyWakeupTimeByOneDay(){
	standbyWakeupTime += 24*60*60*1000;
}

/// sets the time when the machine should wake up from standby
// @param time time in the format hh:mm
void DataManager::setStandbyStartTime(const char* time){
	if(time[2] != ':')
		return;
	int hour = atoi(time);
	int minute = atoi(time+3);
	if(hour >= 24 || hour < 0 || minute >= 60 || minute < 0)
		return;
	long numberTime = hour*60*60 + minute*60;
	setStandbyStartTime(numberTime*1000);
}

/// sets the time after which the machine will go in standby mode if no user interaction occurs
// @param time time in ms
// @param updateBlynk: sends the new parameter to blynk, if set to true and blynk is enabled
void DataManager::setStandbyStartTime(int time){
	if(time < 0) time = 0;
	if(standbyStartTime == time) return;
	standbyStartTime = time;
	eepromWrite((uint8_t*)&standbyStartTime, STANDBY_START_TIME_ADDR, STANDBY_START_TIME_LEN, true);
}

/// returns the time in ms after which the machine goes into standby if no user interaction occurs
int DataManager::getStandbyStartTime(){
	return standbyStartTime;
}

/// returns the time after which the machine goes into standby if no user interaction occurs as hh:mm
String DataManager::getStandbyStartTimeString(){
	int hour = standbyStartTime/60/60/1000;
	int minute = (standbyStartTime/60/1000)%60;
	return String(hour/10) + String(hour%10) + ":" + String(minute/10) + String(minute%10);
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
		res ^= ((uint32_t)(wifiSsid[i])<<(8*j));
		if(wifiSsid[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < WIFI_PW_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(wifiPassword[i])<<(8*j));
		if(wifiPassword[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < HOST_NAME_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(wifiHostName[i])<<(8*j));
		if(wifiHostName[i] == '\0'){
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

String DataManager::setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName){
	if(newSSID != NULL && strcmp(newSSID, "") != 0){
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, newSSID[i]);
			wifiSsid[i] = newSSID[i];
			if(newSSID[i] == '\0'){
				break;
			}
		}
	}
	if(newPassword != NULL && strcmp(newPassword, "") != 0){
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, newPassword[i]);
			wifiPassword[i] = newPassword[i];
			if(newPassword[i] == '\0'){
				break;
			}
		}
	}
	if(newHostName != NULL && strcmp(newHostName, "") != 0){
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, newHostName[i]);
			wifiHostName[i] = newHostName[i];
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

String DataManager::getWifiSSID(){
	return String(wifiSsid);
}

String DataManager::getWifiPassword(){
	return String(wifiPassword);
}

String DataManager::getWifiHostName(){
	return String(wifiHostName);
}

String DataManager::getLanguage(){
	return String(language);
}

String DataManager::getBonjourName(){
	return String(bonjourName);
}

void DataManager::setLanguage(const char* newLanguage){
	if(newLanguage != NULL && strcmp(newLanguage, "") != 0){
		for(uint8_t i = 0; i < LANG_MAX_LEN+1; i++){
			EEPROM.write(LANGUAGE_ADDR + i, newLanguage[i]);
			language[i] = newLanguage[i];
			if(newLanguage[i] == '\0'){
				break;
			}
		}
		EEPROM.commit();
	}
}

void DataManager::setBonjourName(const char* name){
	if(name != NULL && strcmp(name, "") != 0){
		for(uint8_t i = 0; i < BONJOUR_NAME_MAX_LEN+1; i++){
			EEPROM.write(BONJOUR_NAME_ADDR + i, name[i]);
			bonjourName[i] = name[i];
			if(name[i] == '\0'){
				break;
			}
		}
		EEPROM.commit();
	}
}
