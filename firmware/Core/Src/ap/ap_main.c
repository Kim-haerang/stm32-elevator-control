/*
 * ap_main.c
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#include "ap_main.h"
#include "service/elevator/elevator.h"
#include "service/Display/Display.h"
#include "service/lobby/lobby.h"
#include "service/TimeClock/TimeClock.h"
#include "service/sound/sound.h"
#include "service/temp_humid/temp_humid.h"
#include "service/uart/uart.h"
#include "../driver/uart_protocol/uart_protocol.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;

uint16_t curFloor;

static uint32_t lastTimeDisplay;
static timeClockTime_t displayTime;

static void ap_timeDisplay(void){
	TimeClock_t *time = TimeClock_getTimeData();
	displayTime.hour = time->hour;
	displayTime.min = time->min;
	displayTime.sec = time->sec;
	displayTime.msec = time->msec;
	Disp_setTimeClockTime(&displayTime);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		elevator_timerISR();
	}
	else if(htim->Instance == TIM4){
		Disp_fndISR();
	}
	else if(htim->Instance == TIM5){
		elevator_emergencyISR();
	}
}

static void ap_init(void){
	tempHumid_init();
	sound_init();
	elevator_init();
	Disp_init();
	TimeClock_init();
	lobby_init();
	curFloor = 0;
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_Base_Start_IT(&htim5);
	UART_protocol_init(&huart2, packet_do_func);
}

int ap_main(void){

	ap_init();

	ap_timeDisplay();
	sound_power();
	lastTimeDisplay = HAL_GetTick();

	while(1){
		lobby_task();
		elevator_task();
		tempHumid_task();
		sound_task();
		lobby_display();
		curFloor = elevator_curFloor();
		Disp_elevatorFloor(curFloor + 1U);
		uart_temp_task();
		if(HAL_GetTick() - lastTimeDisplay >= 1000){
			lastTimeDisplay = HAL_GetTick();
			TimeClock_task();
			ap_timeDisplay();
		}
	}
	return 0;
}
