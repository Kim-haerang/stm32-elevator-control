/*
 * TimeClock.c
 *
 *  Created on: 2026. 10. 19.
 *      Author: rhoblack
 */
#include "TimeClock.h"
#include "../../../driver/DS3231/DS3231.h"

extern I2C_HandleTypeDef hi2c1;

static TimeClock_t timeClockData; // 중심 데이터
ds3231_time_t rtc_time;

void TimeClock_init(void)
{
	ds3231_init(&hi2c1);
	ds3231_getTime(&rtc_time);
}

TimeClock_t *TimeClock_getTimeData(void)
{
	timeClockData.hour = rtc_time.hour;
	timeClockData.min = rtc_time.min;
	timeClockData.sec = rtc_time.sec;
	return &timeClockData;
}

static void TimeClock_incMSec(void)
{
	if (timeClockData.msec < 99) {
		timeClockData.msec++;
		return;
	}
	timeClockData.msec = 0;
	if (timeClockData.sec < 59) {
		timeClockData.sec++;
		return;
	}
	timeClockData.sec = 0;
	if (timeClockData.min < 59) {
		timeClockData.min++;
		return;
	}
	timeClockData.min = 0;
	if (timeClockData.hour < 23) {
		timeClockData.hour++;
		return;
	}
	timeClockData.hour = 0;
}

void TimeClock_ISR(void)
{
	TimeClock_incMSec();
}

void TimeClock_task(void)
{
	ds3231_getTime(&rtc_time);
}









