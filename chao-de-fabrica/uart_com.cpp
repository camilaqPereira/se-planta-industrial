#include "uart_com.h"


void uart_init(unsigned int ubrr0){
  UBRR0H = (unsigned int) (ubrr0 >> 8);
  UBRR0L = (unsigned int)ubrr0;

  UCSR0A = 0;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void uart_send(unsigned char data){
	while(!(UCSR0A & (1 << UDRE0)));
  UDR0 = data;
}


unsigned char uart_receive(){
	while(!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void uart_write(char *c){
  for(; *c !=0; c++){
    uart_send(*c);
  }
}