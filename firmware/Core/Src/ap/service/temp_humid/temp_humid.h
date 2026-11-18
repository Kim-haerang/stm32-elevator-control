/*
 * temp_humid.h
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_TEMP_HUMID_TEMP_HUMID_H_
#define SRC_AP_SERVICE_TEMP_HUMID_TEMP_HUMID_H_

#include "../../../driver/DHT11/DHT11.h"
extern dht11_data_t tempHumid_data;

typedef enum{
	NO_FIRE,
	FIRE
}fire_state_t;

void tempHumid_init(void);
void tempHumid_task(void);
void tempHumid_disp(void);
void fire_watch_disp(void);
fire_state_t get_fireState(void);

#endif /* SRC_AP_SERVICE_TEMP_HUMID_TEMP_HUMID_H_ */
