/*
 * Display.c
 *
 *  Created on: 2026. 10. 20.
 *      Author: rhoblack
 */

#include "Display.h"
#include "../../../driver/fnd/fnd.h"
#include "../../../driver/lcd/lcd.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

timeClockTime_t *pTimeClockTime;

void Disp_init(void)
{
	Lcd_init(&hi2c1);
}

void Disp_setTimeClockTime(timeClockTime_t * pTime)
{
	pTimeClockTime = pTime;
	Disp_timeClockTime();
}

void Disp_timeClockTime(void)
{
	char timeStr[17];
	snprintf(timeStr, sizeof timeStr, "Time %02d:%02d:%02d",pTimeClockTime->hour, pTimeClockTime->min, pTimeClockTime->sec);
	Lcd_writeStringXY(0, 0, timeStr);
}

void Disp_elevatorFloor(uint16_t floor){
	fnd_set_num(floor);
}

void Disp_fndISR(void)
{
	fnd_show_number();
}

