/*
 * webserver.c
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#include "Webserver.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

Webserver* Webserver::_instance = NULL;
AsyncWebServer server(80);


Webserver::Webserver() {
}

Webserver::~Webserver() {
}

void Webserver::init() {
	// Send web page with input fields to client
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/index.html", String(), false);
	});
	// Route to load espresso.css file
	server.on("/espresso.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/espresso.css", "text/css");
	});
	// Route to load w3.css file
	server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/w3.css", "text/css");
	});
	// Route to load main.js file
	server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/main.js", "text/javascript");
	});
	// Route to load languages.js file
	server.on("/languages.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/languages.js", "text/javascript");
	});
	// Route to load mlr.js file
	server.on("/mlr.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/mlr.js", "text/javascript");
	});
	// Route to load settings.js file
	server.on("/settings.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/settings.js", "text/javascript");
	});
	// Route to get the current settings data
	server.on("/getSettings", HTTP_GET, [](AsyncWebServerRequest *request){
		AsyncResponseStream *response = request->beginResponseStream("application/json");
		serializeSettingsData(response);
		request->send(response);
	});
	// All POST requests are handled in this routine
	server.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
	    Serial.println("Running");
	    // POST request for setting the settings data
	    if (request->url() == "/setSettingsData") {
	    	String answer = evaluateSettingsData(data, len);
	    	request->send(200, "text/plain", answer);
	    }
	  });
	server.onNotFound([](AsyncWebServerRequest *request){
		request->send(404, "text/plain", "Not found");
	});
	server.begin();
}

// This function creates a JSON containing all the settings. The JSON is written
// to the response stream.
void Webserver::serializeSettingsData(AsyncResponseStream *response){
	DynamicJsonDocument json(1024);
	json["wifiSSID"] = DataManager::getWifiSSID();
	json["bonjourName"] = DataManager::getBonjourName();
	json["language"] = DataManager::getLanguage();
	json["boilerP"] = DataManager::getBoilerControllerP();
	json["boilerI"] = DataManager::getBoilerControllerI();
	json["boilerD"] = DataManager::getBoilerControllerD();
	json["BUP"] = DataManager::getBUControllerP();
	json["BUI"] = DataManager::getBUControllerI();
	json["BUD"] = DataManager::getBUControllerD();
	json["boilerFlowmConv"] = DataManager::getPumpTickToVolumeFactor();
	json["bypassFlowmConv"] = DataManager::getBypassTickToVolumeFactor();
	json["wakeupTime"] = DataManager::getStandbyWakeupTimeString();
	json["standbyTime"] = DataManager::getStandbyStartTimeString();
	json["boilerTemp"] = DataManager::getTargetTempBoiler();
	json["BUTemp"] = DataManager::getTargetTempBU();
	json["volOffset"] = DataManager::getVolumeOffset();
	json["volDist"] = DataManager::getDistributionVolume();
	json["preinfBuildup"] = DataManager::getPreinfusionBuildupTime();
	json["preinfWait"] = DataManager::getPreinfusionWaitTime();
	serializeJson(json, *response);
}

// This function evaluates the data sent by a POST request to /setSettingsData
// It updates the DataManager with the data given in the JSON from the POST request
// It returns ok or an error message
String Webserver::evaluateSettingsData(uint8_t *data, size_t len){
	DynamicJsonDocument doc(1024);
	DeserializationError error = deserializeJson(doc, data);
	if (error){
		return error.f_str();
	}
	const char* ssid = NULL;
	const char* pw = NULL;
	if (doc.containsKey("wifiSSID"))
		ssid = doc["wifiSSID"];
	if (doc.containsKey("wifiPW"))
		pw = doc["wifiPW"];
	if (doc.containsKey("bonjourName"))
		DataManager::setBonjourName(doc["bonjourName"]);
	if (doc.containsKey("language"))
		DataManager::setLanguage(doc["language"]);
	if (doc.containsKey("boilerP"))
		DataManager::setBoilerControllerP(doc["boilerP"]);
	if (doc.containsKey("boilerI"))
		DataManager::setBoilerControllerI(doc["boilerI"]);
	if (doc.containsKey("boilerD"))
		DataManager::setBoilerControllerD(doc["boilerD"]);
	if (doc.containsKey("BUP"))
		DataManager::setBUControllerP(doc["BUP"]);
	if (doc.containsKey("BUI"))
		DataManager::setBUControllerI(doc["BUI"]);
	if (doc.containsKey("BUD"))
		DataManager::setBUControllerD(doc["BUD"]);
	if (doc.containsKey("boilerFlowmConv"))
		DataManager::setPumpTickToVolumeFactor(doc["boilerFlowmConv"]);
	if (doc.containsKey("bypassFlowmConv"))
		DataManager::setBypassTickToVolumeFactor(doc["bypassFlowmConv"]);
	if (doc.containsKey("wakeupTime"))
		DataManager::setStandbyWakeupTime((const char*)doc["wakeupTime"]);
	if (doc.containsKey("standbyTime"))
		DataManager::setStandbyStartTime((const char*)doc["standbyTime"]);
	if (doc.containsKey("boilerTemp"))
		DataManager::setTargetTempBoiler(doc["boilerTemp"]);
	if (doc.containsKey("BUTemp"))
		DataManager::setTargetTempBU(doc["BUTemp"]);
	if (doc.containsKey("volOffset"))
		DataManager::setVolumeOffset(doc["volOffset"]);
	if (doc.containsKey("volDist"))
		DataManager::setDistributionVolume(doc["volDist"]);
	if (doc.containsKey("preinfBuildup"))
		DataManager::setPreinfusionBuildupTime(doc["preinfBuildup"]);
	if (doc.containsKey("preinfWait"))
		DataManager::setPreinfusionWaitTime(doc["preinfWait"]);
	if(ssid != NULL || pw != NULL)
		return DataManager::setWIFICredentials(ssid, pw, NULL);
	return "ok";
}



//////////////////////////////// OLD ////////////////////////////////////////7


//AsyncWebServer server(80);
//
////const char* CURRENT_PASSWORD_INPUT = "input_current_password";
//const char* SSID_INPUT = "input_ssid";
//const char* PASSWORD_INPUT = "input_password";
//const char* HOST_NAME_INPUT = "input_host_name";
//
//// HTML web page to handle 3 input fields (input1, input2, input3)
//const char index_html[] PROGMEM = R"rawliteral(
//<!DOCTYPE HTML>
//<html>
//<head>
//	<title>Espresso WIFI settings</title>
//	<style>
//		form  { display: table;      }
//		p     { display: table-row;  }
//		label { display: table-cell; }
//		input { display: table-cell; }
//	</style>
//</head>
//
//<body>
//	<form action="/get" autocomplete="off">
//		<p style="font-size:35px">
//			<label for="input_ssid">New SSID: </label>
//			<input style="font-size:35px" name="input_ssid" maxlength=30 type="text">
//		</p>
//		<p style="font-size:35px">
//			<label for="input_password">New password: &nbsp </label>
//			<input style="font-size:35px" name="input_password" maxlength=30 type="text">
//		</p>
//		<p style="font-size:35px">
//			<label for="input_host_name">New host name: &nbsp </label>
//			<input style="font-size:35px" name="input_host_name" maxlength=30 type="text">
//		</p>
//		<br>
//		<p>
//			<input style="font-size:35px" type="submit" value="Set">
//		</p>
//	</form>
//</body>
//
//</html>
//)rawliteral";
//
//void notFound(AsyncWebServerRequest *request) {
//	request->send(404, "text/plain", "Not found");
//}
//
//
//void webserver_init(){
//	// Send web page with input fields to client
//	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//		request->send_P(200, "text/html", index_html);
//	});
//
//	server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
//		String new_ssid;
//		String new_password;
//		String new_host_name;
//		// GET newnew  SSID value on <ESP_IP>/get?input_ssid=<inputMessage>
//		if (request->hasParam(SSID_INPUT)) {
//			new_ssid = request->getParam(SSID_INPUT)->value();
//		}
//		// GET new password value on <ESP_IP>/get?input_password=<inputMessage>
//		if (request->hasParam(PASSWORD_INPUT)) {
//			new_password = request->getParam(PASSWORD_INPUT)->value();
//		}
//		// GET new host name value on <ESP_IP>/get?input_host_name=<inputMessage>
//		if (request->hasParam(HOST_NAME_INPUT)) {
//			new_host_name = request->getParam(HOST_NAME_INPUT)->value();
//		}
//		String reply = DataManager::setWIFICredentials((new_ssid != NULL?new_ssid.c_str():""),
//				(new_password != NULL?new_password.c_str():""),
//				(new_host_name != NULL?new_host_name.c_str():""));
//		request->send(200, "text/html", "<p style=\"font-size:25\">" + reply + "</p>"
//				"<br><a href=\"/\">Return</a>");
//	});
//	server.onNotFound(notFound);
//	server.begin();
//}
