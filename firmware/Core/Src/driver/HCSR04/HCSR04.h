#pragma once
#include "stm32f4xx_hal.h"
#include <stdbool.h>
void HCSR04_init(GPIO_TypeDef *trig, uint16_t tp, GPIO_TypeDef *echo, uint16_t ep);
bool HCSR04_start(void);
void HCSR04_echoTSR(void);
bool HCSR04_getResult(uint32_t *us, bool *valid);
