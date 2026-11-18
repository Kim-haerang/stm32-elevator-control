/*
 * uart_protocol.h
 *
 *  Created on: 2026. 11. 4.
 *      Author: kccistc
 */

#ifndef SRC_DRIVER_UART_PROTOCOL_UART_PROTOCOL_H_
#define SRC_DRIVER_UART_PROTOCOL_UART_PROTOCOL_H_

#include "stm32f4xx_hal.h"
#define CMD_ELEV_FIRST		0x10
#define CMD_ELEV_SEC		0x11
#define CMD_ELEV_THR		0x12
#define CMD_FIRE_WARN		0x13
#define CMD_ELEV_STATUS_REQ	0x14
#define CMD_TEMP_HUMID_REQ	0x20
#define CMD_DISTANCE_REQ	0x30

// 응답 response CMD
#define CMD_LED_RSP			0x90
#define CMD_LED_STATUS_RSP	0x94
#define CMD_TEMP_HUMID_RSP	0xA0
#define CMD_DISTANCE_RSP	0xB0
#define RSP_OK				0x00
#define RSP_ERR				0x01
#define PKT_START			0xAA
#define PKT_MAX_LEN			16

//함수 포인터 자료형
typedef void (*uart_packet_callback_t)(uint8_t cmd, uint8_t *payload, uint8_t payload_len);

void UART_protocol_init(UART_HandleTypeDef *huart2, uart_packet_callback_t packet_cb);
void UART_protocol_send(uint8_t cmd, uint8_t *payload, uint8_t payload_len);


#endif /* SRC_DRIVER_UART_PROTOCOL_UART_PROTOCOL_H_ */
