/*
 * elevator.c
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */


#include "elevator.h"
#include "../../../driver/button/button.h"
#include "../../../driver/StepMotor/StepMotor.h"
#include "../temp_humid/temp_humid.h"
#include "../uart/uart.h"
#include "../sound/sound.h"

#define LOBBY_LIGHT_INDEX 3
#define FIRE_SENSOR_INDEX 4

#define btn_port 		GPIOC
#define btn1_pin		GPIO_PIN_0
#define btn2_pin		GPIO_PIN_1
#define btn3_pin		GPIO_PIN_3

#define btnEmer_port	GPIOB
#define btnEmer_pin		GPIO_PIN_13

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern volatile uint8_t uart_emergency_cmd;
volatile uint16_t floor_value[ADC_CH_COUNT] = {0};

static volatile elevator_t elevator = {
		.current_floor = FIRST,
		.target_floor = FIRST,
		.act = ELEVATOR_STOP
};

button_t btn1, btn2, btn3, btnEmer;

static stepMotor_pin_t pinMotor = {
		.port = {GPIOA, GPIOA, GPIOB, GPIOB},
		.pin  = {GPIO_PIN_12, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_2}
};

static stepMotor_t motor  = {.pin = &pinMotor};

static void floor_detect(volatile elevator_t *e);
static void elevator_first(void);
static void elevator_second(void);
static void elevator_third(void);

static volatile uint8_t lobbyRequest;
static volatile uint8_t lobbyBlocked;
static volatile uint8_t lobbyReturning;

static int8_t moveDirection;
static volatile uint8_t stopAtNextFloor;
static volatile uint8_t buttonPending[3];
static uint8_t buttonReleased[3];
static volatile uint8_t pendingFloorSound;

uint16_t elevator_fireSensor(void)
{
	return floor_value[FIRE_SENSOR_INDEX];
}

void elevator_requestFirst(void){
	lobbyRequest = 1;
}

int elevator_isMoving(void){
	return elevator.act == ELEVATOR_MOVING;
}

uint8_t elevator_isLobbyReturning(void){
	return lobbyReturning && elevator.act == ELEVATOR_MOVING;
}

void elevator_setLobbyBlocked(uint8_t blocked){
	lobbyBlocked = blocked;
}

void elevator_init(void){
	lobbyRequest = 0;
	lobbyBlocked = 0;
	lobbyReturning = 0;
	button_init(&btn1, btn_port, btn1_pin);
	button_init(&btn2, btn_port, btn2_pin);
	button_init(&btn3, btn_port, btn3_pin);
	button_init(&btnEmer, btnEmer_port, btnEmer_pin);

	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)floor_value, ADC_CH_COUNT);
}

floor_state_t elevator_curFloor(void){
	return elevator.current_floor;
}

uint16_t elevator_lobbyLight(void){
	return floor_value[LOBBY_LIGHT_INDEX];
}

volatile uint8_t getEmergency = 0;
uint8_t btnEmergency = 0;
volatile uint8_t emergency = 0;

void elevator_checkEmergency(void){

	fire_state_t fireState = get_fireState();
	if(fireState == FIRE) {
		getEmergency = 1;
	}
	if(fireState == NO_FIRE){
		getEmergency = 0;
	}
	return;
}

void elevator_task(void)
{
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();
	uint8_t floorSound = pendingFloorSound;
	pendingFloorSound = 0;
	__set_PRIMASK(savedPrimask);

	elevator_checkEmergency();
	if(button_getState(&btn1) == ACT_RELEASE) buttonPending[0] = 1;
	if(button_getState(&btn2) == ACT_RELEASE) buttonPending[1] = 1;
	if(button_getState(&btn3) == ACT_RELEASE) buttonPending[2] = 1;

	static uint32_t prevLcdTime = 0;

	if(HAL_GetTick() - prevLcdTime >= 500){
	    prevLcdTime = HAL_GetTick();
	    if(emergency){
	        fire_watch_disp();
	    }
	    else{
	        tempHumid_disp();
	    }
	}

	if(floorSound != 0 && !emergency){
	    sound_floor(floorSound);
	}

	if(emergency){
	    sound_emergency();
	}
	else{
	    sound_emergencyStop();
	}
}

void elevator_emergencyISR(void)
{
	static uint8_t stable = GPIO_PIN_SET;
	static uint8_t candidate = GPIO_PIN_SET;
	static uint8_t count;
	uint8_t raw = HAL_GPIO_ReadPin(btnEmer_port, btnEmer_pin);
	if(raw != candidate){
		candidate = raw;
		count = 1;
	}
	else if(count < 5){
		count++;
	}
	if(count >= 5 && stable != candidate){
		uint8_t previous = stable;
		stable = candidate;
		if(previous == GPIO_PIN_RESET && stable == GPIO_PIN_SET){
			btnEmergency = (btnEmergency == 1) ? 0 : 1;
		}
	}
	emergency = (getEmergency || btnEmergency || uart_emergency_cmd);
	if(emergency && elevator.act == ELEVATOR_MOVING){
		stopAtNextFloor = 1;
	}
}

void elevator_timerISR(void)
{
	for(int i = 0; i < 3; i++){
		buttonReleased[i] = buttonPending[i];
		buttonPending[i] = 0;
	}

	switch (elevator.act) {
	case ELEVATOR_STOP:
		moveDirection = 0;
		stopAtNextFloor = 0;
		if(emergency == 1){
			elevator.act = ELEVATOR_EMERGENCY;
			return;
		}
		if(lobbyRequest){
			lobbyRequest = 0;
			if(elevator.current_floor != FIRST){
				elevator.target_floor = FIRST;
				lobbyReturning = 1;
				floor_detect(&elevator);
			}
			return;
		}
		if(elevator.current_floor == FIRST && lobbyBlocked){
			return;
		}
		switch (elevator.current_floor) {
		case FIRST:
			elevator_first();
			break;

		case SECOND:
			elevator_second();
			break;

		case THIRD:
			elevator_third();
			break;
		}
		break;

		case ELEVATOR_MOVING:
			if(emergency) stopAtNextFloor = 1;
			floor_detect(&elevator);
			break;

		case ELEVATOR_EMERGENCY:
			if(emergency != 1){
				elevator.act = ELEVATOR_STOP;
			}
			break;
	}
}

void motor_task(stepMotor_dir_t dir){
	StepMotor_stepPhase(&motor, dir);
}

static void floor_detect(volatile elevator_t *e)
{
	e->act = ELEVATOR_MOVING;
	if(moveDirection == 0){
		moveDirection = e->current_floor < e->target_floor ? 1 : -1;
	}
	for(int floor = (int)e->current_floor + moveDirection;
		moveDirection > 0 ? floor <= (int)e->target_floor : floor >= (int)e->target_floor;
		floor += moveDirection){
		if(floor_value[floor] > 1800){
			e->current_floor = (floor_state_t)floor;
			pendingFloorSound = (uint8_t)floor + 1U;
			if(stopAtNextFloor || e->current_floor == e->target_floor){
				e->act = (stopAtNextFloor || emergency)
					? ELEVATOR_EMERGENCY : ELEVATOR_STOP;
				lobbyReturning = 0;
				moveDirection = 0;
				return;
			}
		}
	}
	motor_task(moveDirection > 0 ? STEP_MOTOR_DIR_CW : STEP_MOTOR_DIR_CCW);
}

static void elevator_first(void){
	if(buttonReleased[0]){
		return;
	}
	else if((buttonReleased[1])||(uart_floor_cmd == 2)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = SECOND;
		floor_detect(&elevator);
	}
	else if((buttonReleased[2]) ||(uart_floor_cmd == 3)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = THIRD;
		floor_detect(&elevator);
	}
}

static void elevator_second(void){
	if((buttonReleased[0])||(uart_floor_cmd == 1)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = FIRST;
		floor_detect(&elevator);
	}
	else if((buttonReleased[1])||(uart_floor_cmd == 2)){
		return;
	}
	else if((buttonReleased[2])||(uart_floor_cmd == 3)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = THIRD;
		floor_detect(&elevator);
	}
}

static void elevator_third(void){
	if((buttonReleased[0])||(uart_floor_cmd == 1)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = FIRST;
		floor_detect(&elevator);
	}
	else if((buttonReleased[1]) ||(uart_floor_cmd == 2)){
		elevator.act = ELEVATOR_MOVING;
		elevator.target_floor = SECOND;
		floor_detect(&elevator);
	}
	else if(buttonReleased[2]){
		return;
	}
}

uint8_t elevator_getAct(void)
{
    return elevator.act;
}
