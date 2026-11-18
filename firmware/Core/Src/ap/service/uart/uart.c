/*
 * uart.c
 *
 *  Created on: 2026. 11. 7.
 *      Author: kccistc
 */
#include "uart.h"
#include "../elevator/elevator.h"
#include "../temp_humid/temp_humid.h"
#include "../lobby/lobby.h"
#include "../../../driver/uart_protocol/uart_protocol.h"
#include "../../../driver/stepMotor/stepMotor.h"
#include "../../../driver/DHT11/DHT11.h"

elevator_t elev = {
		.current_floor = FIRST,
		.target_floor = FIRST,
		.act = ELEVATOR_STOP
};
volatile uint8_t temp_req_flag = 0;
volatile uint8_t uart_emergency_cmd = 0;

volatile uint8_t uart_floor_cmd = 0;
void packet_do_func(uint8_t cmd, uint8_t *payload, uint8_t payload_len)
{
	switch(cmd)
	{
	case CMD_ELEV_FIRST: {// cmd = 0x10, payload = 1, payload_len = 1
		uint8_t send_payload;
		if(payload_len >= 1){

			if(payload[0] == 1){
				uart_floor_cmd = 1;
//					while(floor_value[0] <= 1800){
//						motor_task(STEP_MOTOR_DIR_CCW);
//					}

			}

			send_payload = RSP_OK;

		}
		else {
			send_payload = RSP_ERR;

		}
		UART_protocol_send(cmd + 0x80, &send_payload, 1);
		break;

	}
	case CMD_ELEV_SEC:{
		uint8_t send_payload;
		if(payload_len >= 1){
			if(payload[0] == 1){

				if(elev.current_floor == FIRST) {
//					elev.act = ELEVATOR_MOVING;
//					elev.target_floor = SECOND;
//					while(floor_value[1] <= 1800){
//						motor_task(STEP_MOTOR_DIR_CW);
//					}
					uart_floor_cmd = 2;
				}

				else if(elev.current_floor == THIRD){
					elev.act = ELEVATOR_MOVING;
					elev.target_floor = SECOND;
//					while(floor_value[1] <= 1800){
//						motor_task(STEP_MOTOR_DIR_CCW);
//					}
					uart_floor_cmd = 2;
				}

			}



			send_payload = RSP_OK;

		}
		else {
			send_payload = RSP_ERR;

		}
		UART_protocol_send(cmd + 0x80, &send_payload, 1);
		break;
	}
	case CMD_ELEV_THR:{
		uint8_t send_payload;

		if(payload_len >= 1){
			if(payload[0] == 1){

				uart_floor_cmd = 3;
//					while(floor_value[2] <= 1800){
//						motor_task(STEP_MOTOR_DIR_CW);
//					}

			}

			send_payload = RSP_OK;

		}
		else{
			send_payload = RSP_ERR;

		}
		UART_protocol_send(cmd + 0x80, &send_payload, 1);
		break;
	}
	case CMD_FIRE_WARN:{
	    uint8_t send_payload;

	    if(payload_len >= 1){
	        uart_emergency_cmd = (payload[0] == 1) ? 1 : 0;
	        send_payload = RSP_OK;
	    }
	    else{
	        send_payload = RSP_ERR;
	    }

	    UART_protocol_send(cmd + 0x80, &send_payload, 1);
	    break;
	}
	case CMD_ELEV_STATUS_REQ:{
//		uint8_t send_payload[3];
//		act_state_t elev_act;
		//		for(int i=0; i<4; i++) {
		//			send_payload[i] = HAL_GPIO_ReadPin(led[i].port, led[i].pin) == GPIO_PIN_SET ? 1 : 0;
		//		}
		 uint8_t act = elevator_getAct();

		 uint8_t send_payload[3] = {
		         act == ELEVATOR_MOVING,
		         act == ELEVATOR_STOP,
		         act == ELEVATOR_EMERGENCY
		     };


//		send_payload[0] = (elev.act == ELEVATOR_MOVING) ? 1 : 0;
//		send_payload[1] = (elev.act == ELEVATOR_STOP) ? 1 : 0;
//		send_payload[2] = (elev.act == ELEVATOR_EMERGENCY) ? 1 : 0;


		UART_protocol_send(cmd+0x80, send_payload, 3);
		break;
	}
	case CMD_TEMP_HUMID_REQ:{
//		dht11_data_t th_d;
//		dht11_read(&th_d);
//		uint8_t dummy_temp_int = th_d.t_int;
//		uint8_t dummy_temp_dec = th_d.t_dec;
//		uint8_t dummy_humid_int = th_d.h_int;
//		uint8_t dummy_humid_dec = th_d.h_dec;

//		uint8_t send_payload[4];
//		   uint8_t send_payload[4] = {25, 0, 50, 0};
//		send_payload[0] = dummy_temp_int;
//		send_payload[1] = dummy_temp_dec;
//		send_payload[2] = dummy_humid_int;
//		send_payload[3] = dummy_humid_dec;
		temp_req_flag = 1;
//		UART_protocol_send(cmd+0x80, send_payload, 4);

		break;
	}
	case CMD_DISTANCE_REQ:{
		uint32_t distance;//287;
		uint8_t send_payload[4];
		distance = distance_getCm();
		send_payload[0] = (uint8_t)((uint32_t) distance >> 24); // 상위 8bit
		send_payload[1] = (uint8_t)((uint32_t)(distance>>16) & 0xff); // 중상위 8bit
		send_payload[2] = (uint8_t)((uint32_t)(distance >> 8) & 0xff); // 중하위 8bit
		send_payload[3] = (uint8_t)((uint32_t) distance & 0xff); // 하위 8bit


		UART_protocol_send(cmd+0x80, send_payload, 4);
		break;
	}
	default:
		break;
	}
}


void uart_temp_task(void)
{
    if (temp_req_flag == 0) {
        return;
    }

    temp_req_flag = 0;

    uint8_t send_payload[4] = {
        tempHumid_data.t_int,
        tempHumid_data.t_dec,
        tempHumid_data.h_int,
        tempHumid_data.h_dec
    };

    UART_protocol_send(CMD_TEMP_HUMID_REQ + 0x80,
                       send_payload, 4);
}
