#ifndef SE_PLANTA_INDUSTRIAL_SUPERVISOR_H
#define SE_PLANTA_INDUSTRIAL_SUPERVISOR_H


#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c_com.h"
#include "uart_com.h"
#include <util/delay.h>
#include "utils.h"

#define BUTTON_PIN PD2
#define SLAVE_IN_PIN PD3

// Shift values
#define ERROR_STOP_REQUESTED 1
#define ERROR_CRITICAL_TEMPERATURE 2
#define ERROR_WRONG_INCLINATION 3
#define ERROR_PRESENCE_DETECTED 4
#define ERROR_TANK_EMPTY 5
#define ERROR_TANK_FULL 6










#endif