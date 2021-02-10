/*
 * DataManager.h
 *
 *  Created on: Aug 20, 2020
 *      Author: tsugua
 */

#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

#include "Arduino.h"
#include "DeviceControl.h"

#define DEFAULT_TEMP_BOILER		110
#define	DEFAULT_TEMP_BU			94
#define	MAX_TEMP_BOILER			120
#define	MAX_TEMP_BU				110
#define	MIN_TEMP_BOILER			10
#define	MIN_TEMP_BU				10

#define	DEFAULT_FILL_BOILER_OVER_SONDE_TIME		5000
#define	DEFAULT_BOILER_FILL_MAX_TIME			120000
#define MAX_FILL_BOILER_OVER_SONDE_TIME
#define	MIN_FILL_BOILER_OVER_SONDE_TIME
#define MAX_BOILER_FILL_MAX_TIME
#define	MIN_BOILER_FILL_MAX_TIME

#define	DEFAULT_DISTRIBUTION_VOLUME				30
#define	MAX_DISTRIBUTION_VOLUME					200
#define	MIN_DISTRIBUTION_VOLUME					1
#define DEFAULT_VOLUME_OFFSET					5
#define	MAX_VOLUME_OFFSET						100
#define	MIN_VOLUME_OFFSET						0

#define	DEFAULT_BLYNK_ENABLED					true

#define	DEFAULT_BOILER_CONTROLLER_P				25
#define	MAX_BOILER_CONTROLLER_P					100
#define	MIN_BOILER_CONTROLLER_P					0.01
#define	DEFAULT_BU_CONTROLLER_P					1
#define	MAX_BU_CONTROLLER_P						100
#define	MIN_BU_CONTROLLER_P						0.01
#define DEFAULT_CONTROLLER_I					0
#define MAX_CONTROLLER_I						100
#define MIN_CONTROLLER_I						0
#define DEFAULT_CONTROLLER_D					0
#define MAX_CONTROLLER_D						100
#define MIN_CONTROLLER_D						0
#define DEFAULT_PUMP_TICK_TO_VOLUME_FACTOR		0.448
#define DEFAULT_BYP_TICK_TO_VOLUME_FACTOR		0.4579
#define MIN_TICK_TO_VOLUME_FACTOR				0.001
#define	MAX_TICK_TO_VOLUME_FACTOR				10

#define DEFAULT_PREINFUSION_BUILDUP_TIME		2000
#define	MAX_PREINFUSION_BUILDUP_TIME			20000
#define	DEFAULT_PREINFUSION_WAIT_TIME			5000
#define	MAX_PREINFUSION_WAIT_TIME				20000

#define	IDLE_BLYNK_MIN_TEMP_UPDATE_INTERVAL		3000
#define	BREWING_BLYNK_MIN_TEMP_UPDATE_INTERVAL	500

#define DEFAULT_STANDBY_START_TIME				90*60*1000 // time after which standby mode will be entered if no user action
#define DEFAULT_STANDBY_WAKEUP_TIME				-1			//time in s after midnight at which the device will wake up from standby, -1 to disable wakeup

#define WIFI_CONNECT_INTERVAL					10000	//interval in which the device tries to connect to wifi


class DataManager {
public:
	static void pushTempBoiler(double temp);
	static void pushTempBU(double temp);
	static void pushTempTube(double temp);

	static double getTargetTempBoiler();
	static double getTargetTempBU();
	static void setTargetTempBoiler(double temp, bool updateBlynk);
	static void setTargetTempBU(double temp, bool updateBlynk);

	static long getFillBoilerOverSondeTime();
	static long getBoilerMaxFillTime();
	static double getDistributionVolume();
	static void setDistributionVolume(double volume, bool updateBlynk);
	static double getVolumeOffset();
	static void setVolumeOffset(double offset, bool updateBlynk);
	static double getBoilerControllerP();
	static void setBoilerControllerP(double p, bool updateBlynk);
	static double getBoilerControllerI();
	static void setBoilerControllerI(double i, bool updateBlynk);
	static double getBoilerControllerD();
	static void setBoilerControllerD(double d, bool updateBlynk);
	static double getBUControllerP();
	static void setBUControllerP(double p, bool updateBlynk);
	static double getBUControllerI();
	static void setBUControllerI(double i, bool updateBlynk);
	static double getBUControllerD();
	static void setBUControllerD(double d, bool updateBlynk);
	static double getPumpTickToVolumeFactor();
	static void setPumpTickToVolumeFactor(double f, bool updateBlynk);
	static double getBypassTickToVolumeFactor();
	static void setBypassTickToVolumeFactor(double f, bool updateBlynk);

	static int getPreinfusionBuildupTime();
	static void setPreinfusionBuildupTime(int time, bool updateBlynk);
	static int getPreinfusionWaitTime();
	static void setPreinfusionWaitTime(int time, bool updateBlynk);

	static void setStandbyStartTime(int time, bool updateBlynk);
	static int getStandbyStartTime();
	static unsigned long getStandbyWakeupTime();
	static void setStandbyWakeupTime(long time, bool updateBlynk);
	static bool getStandbyWakeupEnabled();
	static void incStandbyWakeupTimeByOneDay();

	static bool getBlynkEnabled();
	static void setBlynkEnabled(bool enabled);

	static bool getWifiConnected();

	static String setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName);

	static void init();
	static void update();
private:
	DataManager(){}
	virtual ~DataManager(){}

	static DeviceControl *dev;

	static double targetTempBoiler;
	static double targetTempBU;

	static long fillBoilerOverSondeTime;
	static long boilerMaxFillTime;

	static double distributionVolume;
	static double volumeOffset;

	static bool blynkEnabled;

	static double boilerControllerP;
	static double boilerControllerI;
	static double boilerControllerD;
	static double BUControllerP;
	static double BUControllerI;
	static double BUControllerD;
	static double pumpTickToVolumeFactor;
	static double bypassTickToVolumeFactor;

	static int preinfusionBuildupTime;
	static int preinfusionWaitTime;
	static unsigned long standbyWakeupTime;
	static int standbyStartTime;	//time after wich standby mode will be entered
	static bool standbyWakeupEnabled;	//used to disable wakeup functionality

	static uint32_t calculateWIFIChecksum();
	static void eepromWrite(uint8_t *src, int addr, int len, bool commit);
	static void eepromRead(uint8_t *dst, int addr, int len);
	static void WIFISetupMode();

	static bool scheduleRestart;

	static void initBlynk();
	static bool blynkInitialized;
	static unsigned long lastWifiConnectTryTime;
};
#endif /* DATAMANAGER_H_ */

