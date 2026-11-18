/*
 * lobby.h
 *
 *  Created on: 2026. 11. 6.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_LOBBY_LOBBY_H_
#define SRC_AP_SERVICE_LOBBY_LOBBY_H_

#include "stm32f4xx_hal.h"

void lobby_init(void);
void lobby_task(void);
uint32_t distance_getCm(void);
void lobby_display(void);

#endif /* SRC_AP_SERVICE_LOBBY_LOBBY_H_ */
