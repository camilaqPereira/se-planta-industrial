#include "se_automacao_industrial_supervisor.h"

/* I2C communication variables*/
const uint8_t SLAVE_ADDRESS = 0x50;
const uint8_t OP_SET_MOTOR1 = 0x01;
const uint8_t OP_SET_MOTOR2 = 0x02;
const uint8_t OP_BUTTON_PRESSED = 0x03;

// Write control variables
volatile bool more_data_to_send = false;
volatile uint8_t data_to_send[2] = {0x00, 0x00}; 
volatile uint8_t data_index = 0;

// Read control variables
volatile bool read_complete = false;
volatile uint8_t received_data = 0x00;


/* System control variables */

volatile uint8_t new_speed_motor1 = 0;
volatile uint8_t speed_motor1 = 0;

volatile uint8_t new_speed_motor2 = 0;
volatile uint8_t speed_motor2 = 0;

volatile bool button_request = false;
volatile bool reading_requested = false;
volatile bool update_supervisor = false;

int main(){
  uint8_t last_state = 0;
  uint8_t active_errors = 0;

  /* Configure communications*/
  i2c_master_init();

  clr_bit(DDRD, SLAVE_IN_PIN);

  uart_init(UBRR);

  /* Configure stop button*/
  clr_bit(DDRD, BUTTON_PIN);
  set_bit(PORTD, BUTTON_PIN);

  EICRA = (1 << ISC01) | (1 << ISC10);
  EIMSK = (1 << INT0) | (1 << INT1);

  /* Configure ADC */
  adc_init();

  /* Enable all interrupts*/
  sei();
  
  /* Configure timer 0*/
  timer0_init();
  //timer1_init();
  
  _delay_ms(10);


  uart_write("---------------------------------- PLANTA INDUSTRIAL (SUPERVISOR) ---------------------------------------\n");
  uart_write("\nPRESSIONE O BOTÃO PARA INICIAR A PRODUÇÃO...\n");
  

  while(1){
    if(button_request){
      uart_write("\nINFO: Parada/Inicio solicitado!\n");
      data_to_send[0] = OP_BUTTON_PRESSED;
      data_to_send[1] = 0x00;
      i2c_start_write(&more_data_to_send, &data_index);
      button_request = false;
    }

    if(new_speed_motor1 != speed_motor1){
      speed_motor1 = new_speed_motor1;
      data_to_send[0] = OP_SET_MOTOR1;
      data_to_send[1] = speed_motor1;
      i2c_start_write(&more_data_to_send, &data_index);

    }

    if(new_speed_motor2 != speed_motor2){
      speed_motor2 = new_speed_motor2;
      data_to_send[0] = OP_SET_MOTOR2;
      data_to_send[1] = speed_motor2;
      i2c_start_write(&more_data_to_send, &data_index);  
      
    }

    if(reading_requested){
      reading_requested = false;
      i2c_start_read(&more_data_to_send, &read_complete);
    }
    
    if(read_complete){
      last_state = active_errors;
      active_errors = received_data;   
      read_complete = false;

      if(!test_bit(last_state, ERROR_STOP_REQUESTED) && test_bit(active_errors, ERROR_STOP_REQUESTED)){
        uart_write("\nINFO: Parada realizada com sucesso!\n");
      }else if(test_bit(last_state, ERROR_STOP_REQUESTED) && !test_bit(active_errors, ERROR_STOP_REQUESTED)){
        uart_write("\nINFO: (Re)Inicialzação da produção!\n");
      }

      if(!test_bit(last_state, ERROR_WRONG_INCLINATION) && test_bit(active_errors, ERROR_WRONG_INCLINATION)){
        uart_write("\nWARNING: Madeira fora do eixo\n");
        uart_write("INFO: Parada realizada com sucesso!\n");
      }

      if(!test_bit(last_state, ERROR_CRITICAL_TEMPERATURE) && test_bit(active_errors, ERROR_CRITICAL_TEMPERATURE)){
        uart_write("\nWARNING: Temperatura crítica\n");
        uart_write("INFO: Parada realizada com sucesso!\n");
      }
      
      if(!test_bit(last_state, ERROR_PRESENCE_DETECTED) && test_bit(active_errors, ERROR_PRESENCE_DETECTED)){
        uart_write("\nWARNING: Presença detectada\n");
        uart_write("INFO: Parada realizada com sucesso!\n");

      }
      
    }
  }
  return 0;
}


void adc_init(){
  ADMUX = (1 << REFS0);     // Vcc reference and mux 0 selected
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);       // Enable ADC with prescaler = 128
  ADCSRB = 0x00;   // Continuous mode
  DIDR0 = (1 << ADC0D) | (1 << ADC1D);
}

void timer0_init(){
  TCCR0A = 0x00;                                                 // Normal mode
  TCCR0B = (1 << CS02) | (1 << CS00);                            // prescaler = 1024
  TIMSK0 = (1 << TOIE0);                                         // Enable overflow interrupt
}

void timer1_init(){
  TCCR1A = 0x00;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);      //prescaler = 1024
  OCR1A = 46874;                                          // T = 3s
  TIMSK1 = (1 << OCIE1A);
}

ISR(TIMER0_OVF_vect){
  static uint8_t counter = 3;

  if(!(counter--)){
    /* Enable ADC conversion */
    set_bit(ADCSRA, ADSC);
    counter = 3;
  }
}

ISR(ADC_vect){
  uint8_t adc_channel = ADMUX & 0x0F;
  uint8_t speed = (uint8_t)(((uint32_t)ADC * 255) / 1023);

  if(adc_channel){
    new_speed_motor2 = speed;
    adc_channel = 0;
  }else{
    new_speed_motor1 = speed;
    adc_channel = 1;
  }

  ADMUX = (ADMUX & 0xF0) | (adc_channel & 0x0F);
}


ISR(INT0_vect){
  button_request = 0x0F;
}

ISR(INT1_vect){
  reading_requested = true;
}


ISR(INT1_COMPA_vect){
  update_supervisor = true;
  uart_write("Update\n");
}



ISR(TWI_vect) {
    switch (TWSR & 0xF8) { // Máscara para ignorar os 3 bits menos significativos
        // Condição de START
        case 0x08: // START transmitido
        case 0x10: // Repeated START transmitido
            if (more_data_to_send) {
                // Operação de escrita: SLA+W (escrita)
                TWDR = (SLAVE_ADDRESS << 1); // SLA+W
            } else {
                // Operação de leitura: SLA+R (leitura)
                TWDR = (SLAVE_ADDRESS << 1) | 1; // SLA+R
            }
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Continua
            break;

        // Escrita
        case 0x18: // SLA+W transmitido, ACK recebido
        case 0x28: // Dado transmitido, ACK recebido
            if (data_index < sizeof(data_to_send) && data_to_send[data_index]) {
                TWDR = data_to_send[data_index++]; // Envia próximo dado
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Continua
            } else {
                more_data_to_send = false; // Todos os dados enviados
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // Envia STOP
            }
            break;

        // Leitura
        case 0x40: // SLA+R transmitido, ACK recebido
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT); // Prepara para receber dado com ACK
            break;

        case 0x50: // Dado recebido, ACK enviado
        case 0x58: // Dado recebido, NACK enviado
            received_data = TWDR; // Lê o último dado
            read_complete = true; // Define a flag indicando que a leitura foi concluída
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // Envia STOP
            break;

        // Erros ou estados inesperados
        case 0x20: // SLA+W transmitido, NACK recebido
        case 0x30: // Dado transmitido, NACK recebido
        case 0x48: // SLA+R transmitido, NACK recebido
        default:
            TWCR = (1 << TWEN) | (1 << TWSTO) | (1 << TWINT); // Envia STOP
            more_data_to_send = 0; // Finaliza transmissão
            read_complete = 1; // Finaliza leitura em caso de erro
            break;
    }
}
