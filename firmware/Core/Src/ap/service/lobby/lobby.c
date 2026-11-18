/*
 * lobby.c
 *
 *  Created on: 2026. 11. 6.
 *      Author: kccistc
 */

#include "lobby.h"
#include "../elevator/elevator.h"
#include "../../../driver/HCSR04/HCSR04.h"

#define lobby_trig_port         GPIOC
#define lobby_trig_pin          GPIO_PIN_5
#define lobby_echo_port         GPIOC
#define lobby_echo_pin          GPIO_PIN_6
#define lobby_detect_led_port    GPIOB
#define lobby_return_led_port    GPIOB
#define lobby_detect_led_pin    GPIO_PIN_15
#define lobby_return_led_pin    GPIO_PIN_14
#define LOBBY_LIGHT_LOW  1200
#define LOBBY_LIGHT_HIGH 2700

static uint8_t lobbyCalled;
static uint8_t lobbyBlocked;
static uint8_t lobbyDetected;
static uint32_t lastMeasure;
static uint32_t lastResult;
static uint32_t distanceUs;
static bool distanceValid;
uint32_t distanceCm = 0;

static uint8_t lobby_isBlocked(void);

void lobby_init(void){
	lobbyCalled = 0;
	lobbyBlocked = 0;
	lobbyDetected = 0;
	lastMeasure = HAL_GetTick();
	lastResult = 0;
	distanceUs = 0;
	distanceValid = false;

	HCSR04_init(lobby_trig_port, lobby_trig_pin, lobby_echo_port, lobby_echo_pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == lobby_echo_pin){
		HCSR04_echoTSR();
	}
}

static uint8_t lobby_isBlocked(void){
	uint16_t light = elevator_lobbyLight();

	if(light >= LOBBY_LIGHT_HIGH){
		lobbyBlocked = 1;
	}
	else if(light <= LOBBY_LIGHT_LOW){
		lobbyBlocked = 0;
	}
	return lobbyBlocked;
}

void lobby_task(void){
	uint32_t pulseUs;
	bool pulseValid;
	uint32_t now = HAL_GetTick();

	if(HCSR04_getResult(&pulseUs, &pulseValid)){
		distanceUs = pulseUs;
		distanceValid = pulseValid;
		lastResult = now;
	}
	if(now - lastMeasure >= 60 && HCSR04_start()){
		lastMeasure = now;
	}

	distanceCm = (distanceUs + 57U) / 58U;
	uint8_t valid = distanceValid && now - lastResult < 150;
	lobbyDetected = valid && distanceCm > 0 && distanceCm <= 50;
	uint8_t blocked = 0;
	if(lobbyDetected){
		blocked = lobby_isBlocked();
	}
	else{
		lobbyBlocked = 0;
	}
	elevator_setLobbyBlocked(blocked);

	if(valid && distanceCm > 50){
		lobbyCalled = 0;
	}
	if(lobbyDetected && blocked && !lobbyCalled){
		elevator_requestFirst();
		lobbyCalled = 1;
	}
}
uint32_t distance_getCm(void) {return distanceCm;}

void lobby_display(void){
	if(lobbyDetected){
		HAL_GPIO_WritePin(lobby_detect_led_port, lobby_detect_led_pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(lobby_detect_led_port, lobby_detect_led_pin, GPIO_PIN_RESET);
	}

	if(elevator_isLobbyReturning()){
		HAL_GPIO_WritePin(lobby_return_led_port, lobby_return_led_pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(lobby_return_led_port, lobby_return_led_pin, GPIO_PIN_RESET);
	}
}
