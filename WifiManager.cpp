/*
 * WifiManager.cpp
 *
 *  Created on: 22.09.2022
 *      Author: ag4716
 */

#include "WifiManager.h"
#include "ESPmDNS.h"

WifiManager* WifiManager::_instance = NULL;

WifiManager::WifiManager() {
	APMode = false;
	init();
	startConnectTime = 0;
	bonjourInitialized = false;
}

WifiManager::~WifiManager() {
}

// connects to the wifi network specified in DataManager or creates an accesspoint if
// the SSID is empty.
void WifiManager::init(){
	if(!DataManager::getWifiSSID().equals("")){
		WiFi.mode(WIFI_STA);
		WiFi.begin(DataManager::getWifiSSID().c_str(), DataManager::getWifiPassword().c_str());
		startConnectTime = millis();
	}
	else{
		WiFi.mode(WIFI_AP);
		WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
		APMode = true;
		if(!bonjourInitialized)
			setupBonjour();
	}
}

// checks if the ESP could connect to the desired wifi network. If the connection could not be established
// during WIFI_CONNECT_TRY_TIME milliseconds, an accesspoint is opened instead
void WifiManager::update(){
	static bool wifiConnected = false;
	if(WiFi.status() != WL_CONNECTED && !APMode){
		wifiConnected = false;
		static unsigned long wifiLastPrintTime = 0;
		if (millis() > wifiLastPrintTime + 1000){
			Serial.println("Connecting to WiFi..");
			wifiLastPrintTime = millis();
		}
		if(millis() > startConnectTime + WIFI_CONNECT_TRY_TIME){
			Serial.println("setting up access point");
			WiFi.mode(WIFI_AP);
			WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
			startConnectTime = millis();
			APMode = true;
			if(!bonjourInitialized)
				setupBonjour();
		}
	}
	else{
		if(!wifiConnected){
			wifiConnected = true;
		  // Print ESP32 Local IP Address
		  Serial.println(WiFi.localIP());
		  Serial.println(WiFi.macAddress());
		  if(!bonjourInitialized)
			  setupBonjour();
		}
		startConnectTime = millis();
	}

}

void WifiManager::setupBonjour()
{
	if (MDNS.begin(DataManager::getBonjourName().c_str()))
	{
		Serial.print(F("mDNS responder started as "));
		Serial.println(DataManager::getBonjourName());

		// Add service to MDNS-SD
		MDNS.addService("n8i-mlp", "tcp", 23);
	}
	else
	{
		Serial.println(F("Error setting up MDNS responder"));
	}
  bonjourInitialized = true;
}
