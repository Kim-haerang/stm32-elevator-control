#include "sound.h"
#include "../../../driver/buzzer/buzzer.h"

#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494

#define C5 (C4 * 2)
#define D5 (D4 * 2)
#define E5 (E4 * 2)
#define F5 (F4 * 2)
#define G5 (G4 * 2)
#define A5 (A4 * 2)
#define B5 (B4 * 2)

#define C6 (C5 * 2)
#define D6 (D5 * 2)
#define E6 (E5 * 2)
#define F6 (F5 * 2)
#define G6 (G5 * 2)
#define A6 (A5 * 2)
#define B6 (B5 * 2)

#define C7 (C6 * 2)
#define D7 (D6 * 2)
#define E7 (E6 * 2)
#define F7 (F6 * 2)
#define G7 (G6 * 2)
#define A7 (A6 * 2)
#define B7 (B6 * 2)

extern TIM_HandleTypeDef htim3;

typedef enum {
    SOUND_STOP,
    SOUND_FLOOR1,
    SOUND_FLOOR2,
    SOUND_FLOOR3,
    SOUND_EMERGENCY
} sound_state_t;

static sound_state_t sound_state = SOUND_STOP;
static uint8_t sound_step = 0;
static uint32_t sound_time = 0;

void sound_floor(uint8_t floor){

    if(sound_state == SOUND_EMERGENCY){
        return;
    }

    sound_step = 0;
    sound_time = HAL_GetTick();

    switch(floor){
    case 1:
        sound_state = SOUND_FLOOR1;
        break;

    case 2:
        sound_state = SOUND_FLOOR2;
        break;

    case 3:
        sound_state = SOUND_FLOOR3;
        break;

    default:
        return;
    }
    buzzer_on(C6);
}

void sound_emergency(void){

    if(sound_state == SOUND_EMERGENCY){
        return;
    }

    sound_state = SOUND_EMERGENCY;
    sound_step = 0;
    sound_time = HAL_GetTick();

    buzzer_on(C6);
}
void sound_emergencyStop(void){

    if(sound_state == SOUND_EMERGENCY){
        buzzer_off();
        sound_state = SOUND_STOP;
    }
}

void sound_power(void){
	buzzer_on(C6); // c4
	HAL_Delay(100);
	buzzer_on(E6); // e4
	HAL_Delay(100);
	buzzer_on(G6); // g4
	HAL_Delay(100);
	buzzer_on(C7); // c5
	HAL_Delay(100);
	buzzer_off();
}

void sound_init(void){
	buzzer_init(&htim3, TIM_CHANNEL_4);
}

void sound_task(void){

    if(sound_state == SOUND_STOP){
        return;
    }

    if(HAL_GetTick() - sound_time < 150){
        return;
    }

    sound_time = HAL_GetTick();
    sound_step++;

    switch(sound_state){

    case SOUND_FLOOR1:
        buzzer_off();
        sound_state = SOUND_STOP;
        break;

    case SOUND_FLOOR2:
        if(sound_step == 1){
            buzzer_on(E6);
        }
        else{
            buzzer_off();
            sound_state = SOUND_STOP;
        }
        break;

    case SOUND_FLOOR3:
        if(sound_step == 1){
            buzzer_on(E6);
        }
        else if(sound_step == 2){
            buzzer_on(G6);
        }
        else{
            buzzer_off();
            sound_state = SOUND_STOP;
        }
        break;

    case SOUND_EMERGENCY:
        if(sound_step % 2 == 1){
            buzzer_on(G6);
        }
        else{
            buzzer_on(C6);
        }
        break;

    default:
        break;
    }
}
