/*
 * elevator.h
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_
#define SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_

#include "stm32f4xx_hal.h"
#include "../../../driver/stepMotor/stepMotor.h"
typedef enum{
	FIRST = 0,
	SECOND,
	THIRD
}floor_state_t;

typedef enum{
	ELEVATOR_STOP,
	ELEVATOR_MOVING,
	ELEVATOR_EMERGENCY
}act_state_t;

typedef struct {
	floor_state_t current_floor;
	floor_state_t target_floor;
	act_state_t act;
} elevator_t;

#define ADC_CH_COUNT	5

extern volatile uint16_t floor_value[ADC_CH_COUNT];

uint16_t elevator_fireSensor(void);
void elevator_init(void);
floor_state_t elevator_curFloor(void);
uint16_t elevator_lobbyLight(void);
void elevator_task(void);
void elevator_requestFirst(void);
int elevator_isMoving(void);
uint8_t elevator_isLobbyReturning(void);
void elevator_setLobbyBlocked(uint8_t blocked);
void motor_task(stepMotor_dir_t dir);
uint8_t elevator_getAct(void);
void elevator_timerISR(void);
void elevator_emergencyISR(void);

#endif /* SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_ */
