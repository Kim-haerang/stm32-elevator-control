#include "HCSR04.h"
#include "../../common/delay/delay.h"
static GPIO_TypeDef *tport, *eport;
static uint16_t tpin, epin;
static volatile unsigned state;
static volatile uint32_t rise, width;
static uint32_t started;
void HCSR04_init(GPIO_TypeDef *t, uint16_t tp, GPIO_TypeDef *e, uint16_t ep) {
 tport=t; tpin=tp; eport=e; epin=ep; state=0; delay_init();
 HAL_GPIO_WritePin(tport,tpin,GPIO_PIN_RESET);
}
bool HCSR04_start(void) {
 if(state) return false;
 started=HAL_GetTick();
 if(HAL_GPIO_ReadPin(eport,epin)==GPIO_PIN_SET) { state=4; return true; }
 state=1;
 HAL_GPIO_WritePin(tport,tpin,GPIO_PIN_SET); delay_us(10);
 HAL_GPIO_WritePin(tport,tpin,GPIO_PIN_RESET); return true;
}
void HCSR04_echoTSR(void) {
 if(state==1 && HAL_GPIO_ReadPin(eport,epin)==GPIO_PIN_SET) {
  rise=delay_get_cycle(); state=2;
 } else if(state==2 && HAL_GPIO_ReadPin(eport,epin)==GPIO_PIN_RESET) {
  width=delay_elapsed_us(rise); state=3;
 }
}
bool HCSR04_getResult(uint32_t *us, bool *valid) {
 uint32_t mask=__get_PRIMASK(); __disable_irq();
 bool ready=false;
 if(state==3) { *us=width; *valid=width>=116 && width<=23200; state=0; ready=true; }
 else if(state && (state==4 || HAL_GetTick()-started>=30)) {
  *us=0; *valid=false; state=0; ready=true;
 }
 __set_PRIMASK(mask); return ready;
}
