/*
 * Display.h
 *
 *  Created on: 2026. 10. 20.
 *      Author: rhoblack
 */

#ifndef SRC_AP_DISPLAY_DISPLAY_H_
#define SRC_AP_DISPLAY_DISPLAY_H_

#include <stdint.h>

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t msec;
}timeClockTime_t;

void Disp_elevatorFloor(uint16_t floor);

void Disp_init(void);
void Disp_setTimeClockTime(timeClockTime_t * pTime);
void Disp_timeClockTime(void);
void Disp_fndISR(void);


/* LCD rows only; these functions never change the FND floor display. */
void Disp_temperatureHumidity(uint8_t temperature, uint8_t humidity, uint8_t valid);

#endif /* SRC_AP_DISPLAY_DISPLAY_H_ */
