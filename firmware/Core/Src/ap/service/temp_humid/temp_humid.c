/*
 * temp_humid.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */
#include "temp_humid.h"
#include "../../../driver/DHT11/DHT11.h"
#include "../../../driver/lcd/lcd.h"
#include "../sound/sound.h"
#include "../elevator/elevator.h"

#include <stdio.h>

#define dht_port GPIOC
#define dht_pin GPIO_PIN_8

dht11_data_t tempHumid_data;
extern ADC_HandleTypeDef hadc1;
fire_state_t fire_state = NO_FIRE;

void tempHumid_init(void)
{
	dht11_init(dht_port, dht_pin);
}

void tempHumid_disp()
{
	char buff[30];
	sprintf(buff, "T:%02d.%1d C H:%2d%%  ", tempHumid_data.t_int, tempHumid_data.t_dec, tempHumid_data.h_int);
	Lcd_writeStringXY(1, 0, buff);

}

void fire_watch_disp()
{
	char buff[30];
	printf("FIRE! T: %d", tempHumid_data.t_int);
	sprintf(buff, "FIRE! T: %2u.%u   ", tempHumid_data.t_int, tempHumid_data.t_dec);
	Lcd_writeStringXY(1, 0, buff);
}

static void fire_task()
{
	fire_watch_disp();
}

void tempHumid_task(void)
{
	static uint32_t prevTime = 0;
	if(HAL_GetTick() - prevTime < 2000){
		return;
	}
	prevTime = HAL_GetTick();

	dht11_read(&tempHumid_data);


	if((tempHumid_data.t_int >= 30) && (elevator_fireSensor() > 2600)) {
		fire_state = FIRE;
		fire_task();
	}
	else {
		fire_state = NO_FIRE;
		tempHumid_disp();
	}
}

fire_state_t get_fireState(void){
	return fire_state;
}
