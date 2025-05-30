#ifndef UART_COM_H
#define UART_COM_H


#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD 9600
#define UBRR F_CPU/16/BAUD-1
#define num_size 5


/* Function prototypes */
void uart_init(unsigned int ubbr0);
void uart_send(unsigned char data);
unsigned char uart_receive();
void uart_write(char *c);

#endif