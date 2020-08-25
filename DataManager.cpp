/*
 * DataManager.cpp
 *
 *  Created on: Aug 21, 2020
 *      Author: tsugua
 */

#include "DataManager.h"
#include "DeviceControl.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Arduino.h"
#include "EEPROM.h"
#include <stdint.h>


bool DataManager::blynkEnabled;
double DataManager::targetTempBoiler;
double DataManager::targetTempBU;
long DataManager::fillBoilerOverSondeTime = DEFAULT_FILL_BOILER_OVER_SONDE_TIME;
long DataManager::boilerMaxFillTime = DEFAULT_BOILER_FILL_MAX_TIME;
double DataManager::distributionVolume;
double DataManager::volumeOffset;
double DataManager::boilerControllerP;
double DataManager::BUControllerP;
DeviceControl* DataManager::dev;

//	Blynk pins
// 	V1: Boiler temperature
//	V2: BU temperature
//	V3: Tube temperature
//	V4: Boiler target temperature
//	V5: BU target temperature
//	V6: distribution volume
//	V7: volume offset
//	V8: Boiler P param
//	V9: BU P param

//	Flash address parameters
#define SSID_MAX_LEN				30
#define SSID_ADDR					0
#define WIFI_PW_MAX_LEN				30
#define WIFI_PW_ADDR				31
#define	HOST_NAME_MAX_LEN			30
#define	HOST_NAME_ADDR				62
#define	TARGET_TEMP_BOILER_ADDR		93
#define TARGET_TEMP_BOILER_LEN		8
#define	TARGET_TEMP_BU_ADDR			101
#define TARGET_TEMP_BU_LEN			8
#define	DIST_VOL_ADDR				109
#define	DIST_VOL_LEN				8
#define	VOL_OFFSET_ADDR				117
#define	VOL_OFFSET_LEN				8
#define	BOILER_CONTROLLER_P_ADDR	125
#define	BOILER_CONTROLLER_P_LEN		8
#define	BU_CONTROLLER_P_ADDR		133
#define	BU_CONTROLLER_P_LEN			8
#define	BLYNK_ENABLED_ADDR			141
#define	BLYNK_ENABLED_LEN			1


#define CHECKSUM_ADDR				142
#define	CHECKSUM_LEN				4
#define	EEPROM_SIZE					200
char ssid[SSID_MAX_LEN+1];
char password[WIFI_PW_MAX_LEN+1];
char hostName[HOST_NAME_MAX_LEN+1];

#define	DEFAULT_SSID		"Hulapalu"
#define	DEFAULT_PASSWORD	"thebestAK13top"
#define	DEFAULT_HOST_NAME	"Espressomaschine"

char auth[] = "odArqE_fTz-LOqvk9ot0U2Anlo7P9oGR";

BlynkTimer timer;


void DataManager::init(){
	dev = DeviceControl::instance();

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
//	Serial.println("Boiler target: " + String(targetTempBoiler));
//	Serial.println("BU target: " + String(targetTempBU));
//	Serial.println("distribution volume: " + String(distributionVolume));
//	Serial.println("volume offset: " + String(volumeOffset));
//	Serial.println("blynk enabled: " + String(blynkEnabled));
//	Serial.println("SSID: " + String(ssid));
//	Serial.println("password: " + String(password));

	//possibly reset Wifi credentials
	if((dev->getManualDistribution() && dev->getButton1() && dev->getButton2())
			|| checksum != calculateWIFIChecksum()){
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
			isnan(boilerControllerP) || isnan(BUControllerP)){
		Serial.println("Writing default values");
		targetTempBoiler = DEFAULT_TEMP_BOILER;
		targetTempBU = DEFAULT_TEMP_BU;
		distributionVolume = DEFAULT_DISTRIBUTION_VOLUME;
		volumeOffset = DEFAULT_VOLUME_OFFSET;
		blynkEnabled = DEFAULT_BLYNK_ENABLED;
		boilerControllerP = DEFAULT_BOILER_CONTROLLER_P;
		BUControllerP = DEFAULT_BU_CONTROLLER_P;
		eepromWrite((uint8_t*)&targetTempBoiler, TARGET_TEMP_BOILER_ADDR, TARGET_TEMP_BOILER_LEN, false);
		eepromWrite((uint8_t*)&targetTempBU, TARGET_TEMP_BU_ADDR, TARGET_TEMP_BU_LEN, false);
		eepromWrite((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN, false);
		eepromWrite((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN, false);
		uint8_t temp = blynkEnabled?0xFF:0x00;
		eepromWrite((uint8_t*)&temp, BLYNK_ENABLED_ADDR, BLYNK_ENABLED_LEN, false);
		eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, false);
		eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, false);
		EEPROM.commit();
	}

	if(DataManager::getBlynkEnabled()){
		WiFi.setHostname(hostName);
		Blynk.begin(auth, ssid, password);
		//write current values to cloud
		Blynk.virtualWrite(V4, targetTempBoiler);
		Blynk.virtualWrite(V5, targetTempBU);
		Blynk.virtualWrite(V6, distributionVolume);
		Blynk.virtualWrite(V7, volumeOffset);
		Blynk.virtualWrite(V8, boilerControllerP);
		Blynk.virtualWrite(V9, BUControllerP);
	}
}

void DataManager::update(){
	if(DataManager::getBlynkEnabled()){
		Blynk.run();
		timer.run();
	}
}

void DataManager::pushTempBoiler(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V1, temp);
	}
}

void DataManager::pushTempBU(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V2, temp);
	}
}

void DataManager::pushTempTube(double temp){
	static long lastUpdateTime = 0;
	if(DataManager::getBlynkEnabled() &&
			(millis() >= lastUpdateTime+IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL ||
					millis() >= lastUpdateTime + BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL)){
		lastUpdateTime = millis();
		Blynk.virtualWrite(V3, temp);
	}
}


inline double DataManager::getTargetTempBoiler(){
	return targetTempBoiler;
}

inline double DataManager::getTargetTempBU(){
	return targetTempBU;
}

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
	if(getBlynkEnabled() && updateBlynk){
		Blynk.virtualWrite(V4, targetTempBoiler);
	}
}

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
	if(getBlynkEnabled() && updateBlynk){
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

void DataManager::setDistributionVolume(double volume, bool updateBlynk){
	if(volume > MAX_DISTRIBUTION_VOLUME) volume = MAX_DISTRIBUTION_VOLUME;
	else if(volume < MIN_DISTRIBUTION_VOLUME) volume = MIN_DISTRIBUTION_VOLUME;
	if(distributionVolume == volume){
		return;
	}

	distributionVolume = volume;
	eepromWrite((uint8_t*)&distributionVolume, DIST_VOL_ADDR, DIST_VOL_LEN, true);
	if(getBlynkEnabled() && updateBlynk){
		Blynk.virtualWrite(V6, distributionVolume);
	}
}

double DataManager::getVolumeOffset(){
	return volumeOffset;
}

void DataManager::setVolumeOffset(double offset, bool updateBlynk){
	if(offset > MAX_VOLUME_OFFSET) offset = MAX_VOLUME_OFFSET;
	if(offset < MIN_VOLUME_OFFSET) offset = MIN_VOLUME_OFFSET;
	if(offset == volumeOffset){
		return;
	}
	volumeOffset = offset;
	eepromWrite((uint8_t*)&volumeOffset, VOL_OFFSET_ADDR, VOL_OFFSET_LEN, true);
	if(getBlynkEnabled() && updateBlynk){
		Blynk.virtualWrite(V7, volumeOffset);
	}
}

bool DataManager::getBlynkEnabled(){
	return blynkEnabled;
}

void DataManager::setBlynkEnabled(bool enabled){
	blynkEnabled = enabled;
	//todo: possibly run initialization or deinitialization
}

double DataManager::getBoilerControllerP(){
	return boilerControllerP;
}

void DataManager::setBoilerControllerP(double p, bool updateBlynk){
	if(p < MIN_BOILER_CONTROLLER_P) p = MIN_BOILER_CONTROLLER_P;
	else if(p > MAX_BOILER_CONTROLLER_P) p = MAX_BOILER_CONTROLLER_P;
	if(boilerControllerP == p) return;
	boilerControllerP = p;
	eepromWrite((uint8_t*)&boilerControllerP, BOILER_CONTROLLER_P_ADDR, BOILER_CONTROLLER_P_LEN, true);
	if(getBlynkEnabled() && updateBlynk){
		Blynk.virtualWrite(V8, boilerControllerP);
	}
}

double DataManager::getBUControllerP(){
	return BUControllerP;
}

void DataManager::setBUControllerP(double p, bool updateBlynk){
	if(p < MIN_BU_CONTROLLER_P) p = MIN_BU_CONTROLLER_P;
	else if(p > MAX_BU_CONTROLLER_P) p = MAX_BU_CONTROLLER_P;
	if(BUControllerP == p) return;
	BUControllerP = p;
	eepromWrite((uint8_t*)&BUControllerP, BU_CONTROLLER_P_ADDR, BU_CONTROLLER_P_LEN, true);
	if(getBlynkEnabled() && updateBlynk){
		Blynk.virtualWrite(V9, BUControllerP);
	}

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

void DataManager::eepromRead(uint8_t *dst, int addr, int len){
	if(len <= 0){
		return;
	}
	for(int i = 0; i < len; i++){
		*(dst++) = EEPROM.read(addr++);
	}
}

void DataManager::eepromWrite(uint8_t *src, int addr, int len, bool commit){
	if(len <= 0){
		return;
	}
	for(int i = 0; i < len; i++){
		EEPROM.write(addr++, *(src++));
	}
	if(commit){
		EEPROM.commit();
	}
}



