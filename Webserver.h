/*
 * webserver.h
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DataManager.h"


class Webserver {
public:
	static Webserver *instance(){
		if(!_instance)
			_instance = new Webserver();
		return _instance;
	}
	void init();

private:
	static Webserver *_instance;
	Webserver();
	virtual ~Webserver();
	Webserver (const Webserver& );
	static DataManager* dat;
	static void serializeSettingsData(AsyncResponseStream *response);
	static String evaluateSettingsData(uint8_t *data, size_t len);

	void notFound(AsyncWebServerRequest *request);
};
#endif /* WEBSERVER_H_ */
