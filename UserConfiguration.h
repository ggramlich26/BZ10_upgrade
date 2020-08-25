/*
 * UserConfiguration.h
 *
 *  Created on: May 30, 2020
 *      Author: tsugua
 */

#ifndef USERCONFIGURATION_H_
#define USERCONFIGURATION_H_

class UserConfiguration {
public:
	UserConfiguration();
	virtual ~UserConfiguration();

	long getFillBoilerOverSondeTime(){
		return fillBoilerOverSondeTime;
	}
	long getBoilerMaxFillTime(){
		return boilerMaxFillTime;
	}
	double getDistributionVolume(){
		return distributionVolume;
	}
	double getOffsetVolume(){
		return offsetVolume;
	}
private:
	long fillBoilerOverSondeTime = 5000;
	long boilerMaxFillTime = 120000;
	double distributionVolume = 25;
	double offsetVolume = 5;
};

#endif /* USERCONFIGURATION_H_ */
