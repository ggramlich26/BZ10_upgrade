/*
 * WifiManager.h
 *
 *  Created on: 22.09.2022
 *      Author: ag4716
 */

#ifndef WIFIMANAGER_H_
#define WIFIMANAGER_H_

#include "Arduino.h"
#include "DataManager.h"
#include <WiFi.h>
#include "Wifi_config.h"

class WifiManager {
public:
	static WifiManager *instance(){
		if(!_instance)
			_instance = new WifiManager();
		return _instance;
	}
	void update();

private:
	static WifiManager *_instance;
	WifiManager();
	virtual ~WifiManager();
	WifiManager (const WifiManager& );
	void init();
	bool APMode;
	unsigned long startConnectTime;
	void setupBonjour();
	bool bonjourInitialized;
};

#endif /* WIFIMANAGER_H_ */
