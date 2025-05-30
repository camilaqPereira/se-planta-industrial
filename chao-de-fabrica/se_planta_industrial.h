#ifndef SE_PLANTA_INDUSTRIAL_H
#define SE_PLANTA_INDUSTRIAL_H

#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c_com.h"
#include "uart_com.h"
#include "utils.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define MASTER_PIN PD5
#define MOTOR1_PIN PB3
#define MOTOR2_PIN PD3
#define LM35_PIN A7
#define LDR_PIN A6
#define GREEN_LED_PIN PC2
#define RED_LED_PIN PC3
#define BUZZER_PIN PD6
#define SW520D_PIN PB4
#define BUTTON_PIN PD2
#define SERVO_PIN PB1
#define TRIGGER_PIN PC0
#define ECHO_PIN PC1

// Shift values
#define ERROR_STOP_REQUESTED 1
#define ERROR_CRITICAL_TEMPERATURE 2
#define ERROR_WRONG_INCLINATION 3
#define ERROR_PRESENCE_DETECTED 4
#define ERROR_TANK_EMPTY 5
#define ERROR_TANK_FULL 6

#define TANK_MAX 25
#define TANK_MIN 10
#define LDR_LIMIT 900

typedef enum {
  S_IDLE,
  S_READING,
  S_RUNNING,
  S_STOPPED
}States_t;


/* Function prototypes */
void timer2_init();
void adc_init();










#endif