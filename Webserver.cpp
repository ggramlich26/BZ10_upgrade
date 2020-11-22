/*
 * webserver.c
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#include "Webserver.h"
#include "DataManager.h"
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

//const char* CURRENT_PASSWORD_INPUT = "input_current_password";
const char* SSID_INPUT = "input_ssid";
const char* PASSWORD_INPUT = "input_password";
const char* HOST_NAME_INPUT = "input_host_name";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
	<title>Espresso WIFI settings</title>
	<style>
		form  { display: table;      }
		p     { display: table-row;  }
		label { display: table-cell; }
		input { display: table-cell; }
	</style>
</head>

<body>
	<form action="/get" autocomplete="off">
		<p style="font-size:35px">
			<label for="input_ssid">New SSID: </label>
			<input style="font-size:35px" name="input_ssid" maxlength=30 type="text">
		</p>
		<p style="font-size:35px">
			<label for="input_password">New password: &nbsp </label>
			<input style="font-size:35px" name="input_password" maxlength=30 type="text">
		</p>
		<p style="font-size:35px">
			<label for="input_host_name">New host name: &nbsp </label>
			<input style="font-size:35px" name="input_host_name" maxlength=30 type="text">
		</p>
		<br>
		<p>
			<input style="font-size:35px" type="submit" value="Set">
		</p>
	</form>
</body>

</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
}


void webserver_init(){
	// Send web page with input fields to client
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send_P(200, "text/html", index_html);
	});

	server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
		String new_ssid;
		String new_password;
		String new_host_name;
		// GET newnew  SSID value on <ESP_IP>/get?input_ssid=<inputMessage>
		if (request->hasParam(SSID_INPUT)) {
			new_ssid = request->getParam(SSID_INPUT)->value();
		}
		// GET new password value on <ESP_IP>/get?input_password=<inputMessage>
		if (request->hasParam(PASSWORD_INPUT)) {
			new_password = request->getParam(PASSWORD_INPUT)->value();
		}
		// GET new host name value on <ESP_IP>/get?input_host_name=<inputMessage>
		if (request->hasParam(HOST_NAME_INPUT)) {
			new_host_name = request->getParam(HOST_NAME_INPUT)->value();
		}
		String reply = DataManager::setWIFICredentials((new_ssid != NULL?new_ssid.c_str():""),
				(new_password != NULL?new_password.c_str():""),
				(new_host_name != NULL?new_host_name.c_str():""));
		request->send(200, "text/html", "<p style=\"font-size:25\">" + reply + "</p>"
				"<br><a href=\"/\">Return</a>");
	});
	server.onNotFound(notFound);
	server.begin();
}
