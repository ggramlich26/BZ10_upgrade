/*
 * WifiManager.cpp
 *
 *  Created on: 22.09.2022
 *      Author: ag4716
 */

#include "WifiManager.h"
#include "ESPmDNS.h"
#include "DataManager.h"

WifiManager* WifiManager::_instance = NULL;

WifiManager::WifiManager() {
	APMode = false;
	startConnectTime = 0;
	wifiInitialized = false;
	bonjourInitialized = false;
	RTCInitialized = false;
	wifiConnected = false;
	init();
}

WifiManager::~WifiManager() {
}

// connects to the wifi network specified in DataManager or creates an accesspoint if
// the SSID is empty.
void WifiManager::init(){
	if(DataManager::getWifiEnabled()){
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
		wifiInitialized = true;
	}
}

// checks if the ESP could connect to the desired wifi network. If the connection could not be established
// during WIFI_CONNECT_TRY_TIME milliseconds, an accesspoint is opened instead
void WifiManager::update(){
	if(!wifiInitialized){
		if(DataManager::getWifiEnabled())
			init();
	}
	else{
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
			  if(!RTCInitialized)
				  setupRTC();
			}
			startConnectTime = millis();
		}
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

void WifiManager::setupRTC(){
	struct tm timeinfo;

	  Serial.println("Setting up time");
	  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
	  if(!getLocalTime(&timeinfo)){
	    Serial.println("  Failed to obtain time");
	    return;
	  }
	  Serial.println("  Got the time from NTP");
	  // Now we can set the real timezone
	  String timezone = "CET-1CEST,M3.5.0/2:00,M10.5.0/3";
	  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
	  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
	  tzset();

	RTCInitialized = true;
}

/// Returns the time in seconds after midnight
int WifiManager::getRTCTime(){
	if(!wifiConnected || !RTCInitialized)
		return -1;
	struct tm timeinfo;
	if(!getLocalTime(&timeinfo)){
		Serial.println("Failed to obtain time 1");
		return -1;
	}
	return timeinfo.tm_hour*60*60 + timeinfo.tm_min*60 + timeinfo.tm_sec;
}

bool WifiManager::connected(){
	return wifiConnected;
}

bool WifiManager::hotspotMode(){
	return APMode;
}
