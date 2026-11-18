/*
 * uart.h
 *
 *  Created on: 2026. 11. 7.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_UART_UART_H_
#define SRC_AP_SERVICE_UART_UART_H_
#include <stdint.h>
extern volatile uint8_t uart_floor_cmd;
//extern dht11_data_t tempHumid_data;
void packet_do_func(uint8_t cmd, uint8_t *payload, uint8_t payload_len);
void uart_temp_task(void);;

#endif /* SRC_AP_SERVICE_UART_UART_H_ */
