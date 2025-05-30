#include "se_planta_industrial.h"

/* I2C communication variables*/
const uint8_t SLAVE_ADDRESS = 0x50;
const uint8_t OP_SET_MOTOR1 = 0x01;
const uint8_t OP_SET_MOTOR2 = 0x02;
const uint8_t OP_BUTTON_PRESSED = 0x03;

volatile RECEIVED_DATA_T i2c_rx_buffer[BUFFER_SIZE];
volatile uint8_t read_index = 0;
volatile uint8_t write_index = 0;
volatile bool i2c_lost_data = false;

volatile I2C_COM_STATES_T i2c_com_state = I2C_RECEIVING_OPCODE;
volatile uint8_t current_opcode = 0x00;


/* System control variables*/

volatile uint8_t active_errors = (1 << ERROR_STOP_REQUESTED);

volatile uint8_t flag_stop_requested = 0x0F;
volatile uint8_t flag_critical_temperature = 0x00;
volatile uint8_t flag_presence_detected = 0x00;
volatile uint8_t flag_wrong_inclination = 0x00;

volatile uint64_t temperature = 25;        // Valid value
int main(){

  /* System variables */
  States_t system_state = S_IDLE;
  
  bool notify_supervisor = false;
  uint8_t flag_empty_tank = 0;
  uint8_t flag_full_tank = 0;
  uint16_t tank_level = 20;     // Valid value
  

  /* Enable all interrupts*/
  sei();

  while(1){

    switch(system_state){
      case S_IDLE:
        /* Init communications */  
        i2c_slave_init(SLAVE_ADDRESS);
        uart_init(UBRR);

        /* Init temperature and presence sensors*/
        adc_init();

        /* Init green led */
        set_bit(DDRC, GREEN_LED_PIN);
        set_bit(PORTC, GREEN_LED_PIN);

        /* Init red led*/
        set_bit(DDRC, RED_LED_PIN);
        set_bit(PORTC, RED_LED_PIN);

        /* Init tilt sensor */
        clr_bit(DDRB, SW520D_PIN);

        /* Init master pin */
        set_bit(DDRD, MASTER_PIN);
        clr_bit(PORTD, MASTER_PIN);

        /* Init ultrasonic sensor */
        hcsr04_init();

        /* Enable all interrupts */
        sei();

        _delay_ms(10);
        /* WAIT FOR SUPERVISOR TO INIT PRODUCTION*/
        uart_write("----------------------------------- PLANTA INDUSTRIAL # CHAO DE FABRICA --------------------------------\n");
        uart_write("\nINFO: ESPERANDO PELO SUPERVISOR...\n");

        toggle_bit(PORTD, MASTER_PIN);
        while(flag_stop_requested);

        /* Init timer 2: pwm (motor control) and ADC readings*/
        timer0_init();
        timer1_init();
        timer2_init();
        
        button_init();

        uart_write("\nINFO: PRODUÇÃO INICIADA!\n");


        system_state = S_READING;
        active_errors = 0;
        toggle_bit(PORTD, MASTER_PIN);
        break;
      
      case S_READING:

        if(flag_stop_requested == 0x0F){
          uart_write("\nINFO: Parada solicitada!\n");
          set_bit(active_errors, ERROR_STOP_REQUESTED);
          flag_stop_requested = 0xFF;
          notify_supervisor = true;
        }else if(flag_stop_requested == 0xF0){
          uart_write("\nINFO: (Re)Iniciando produção!\n");
          clr_bit(active_errors, ERROR_STOP_REQUESTED);
          flag_stop_requested = 0x00;
          notify_supervisor = true;
        }

        if(flag_critical_temperature == 0x0F){          // Error detected, no message send to supervisor
          uart_write("\nWARNING: Temperatura critica\n");
          set_bit(active_errors, ERROR_CRITICAL_TEMPERATURE);
          flag_critical_temperature = 0xFF;
          notify_supervisor = true;          
        }else if(flag_critical_temperature == 0xF0){     // Error cleared after detection
          clr_bit(active_errors, ERROR_CRITICAL_TEMPERATURE);
          flag_critical_temperature = 0x00;
          notify_supervisor = true;
        }

        if(flag_presence_detected == 0x0F){
          set_bit(active_errors, ERROR_PRESENCE_DETECTED);
          uart_write("\nWARNING: PRESENCA DETECTADA \n");
          flag_presence_detected = 0xFF;
          notify_supervisor = true;
        }else if(flag_presence_detected == 0XF0){
          clr_bit(active_errors, ERROR_PRESENCE_DETECTED);
          flag_presence_detected = 0x00;
          notify_supervisor = true;
        }

        if(flag_wrong_inclination == 0x0F){
          set_bit(active_errors, ERROR_WRONG_INCLINATION);
          uart_write("\nWARNING: MADEIRA FORA DO EIXO\n");
          flag_wrong_inclination = 0xFF;
          notify_supervisor = true;
        }else if(flag_wrong_inclination == 0xF0){
          clr_bit(active_errors, ERROR_WRONG_INCLINATION);
          flag_wrong_inclination = 0x00;
          notify_supervisor = true;
        }

        // ------------- READ ULTRASSONIC SENSOR (TANK LEVEL) ----------------------
        tank_level = hcsr04_get_distance_cm();

        if(tank_level < TANK_MIN){
          flag_empty_tank |= 0x0F;
          set_bit(active_errors, ERROR_TANK_EMPTY);
        }else if(tank_level > TANK_MAX){
          flag_full_tank |= 0x0F;
          set_bit(active_errors, ERROR_TANK_FULL);
        }else{
          flag_empty_tank &= 0xF0;
          flag_full_tank &= 0xF0; 
          clr_bit(active_errors, ERROR_TANK_EMPTY);
          clr_bit(active_errors, ERROR_TANK_FULL);
        }

        if(flag_empty_tank == 0x0F){
          uart_write("INFO: Tanque em nível mínimo crítico!\n");
          flag_empty_tank = 0xFF;
          notify_supervisor = true;
        }else if(flag_full_tank == 0x0F){
          uart_write("INFO: Tanque em nível máximo crítico!\n");
          flag_full_tank = 0xFF;
          notify_supervisor = true;
        }else if(flag_empty_tank == 0xF0 || flag_full_tank == 0xF0){
          flag_empty_tank &= 0x0F;
          flag_full_tank &= 0x0F;
          notify_supervisor = true;
        }

        // ----------------------------------- NOTIFY SUPERVISOR ------------------------
        if(notify_supervisor){
          toggle_bit(PORTD, MASTER_PIN);
          notify_supervisor = false;
        }

        // ----------------------------------- UPDATE SYSTEM'S STATE --------------------
        system_state = (active_errors) ? S_STOPPED : S_RUNNING;
        break;
      
      case S_RUNNING:
        /* Turn on green led */
        clr_bit(PORTC, GREEN_LED_PIN);

        /* Turn off red led and buzzer*/
        set_bit(PORTC, RED_LED_PIN);
        clr_bit(TCCR0A, COM0A1);

        /* Init motors */
        set_bit(TCCR2A, COM2A1);
        set_bit(TCCR2A, COM2B1);

        /* Deactivate servo*/
        OCR1A = 1000;

        active_errors = 0;
        system_state = S_READING;

        break;
      
      case S_STOPPED:
        /* Turn off green led */
        set_bit(PORTC, GREEN_LED_PIN);

        /* Stop motors */
        clr_bit(TCCR2A,COM2A1);
        clr_bit(TCCR2A, COM2B1);


        if(flag_critical_temperature){
          clr_bit(PORTC, RED_LED_PIN);
          set_bit(TCCR0A, COM0A1);    //Buzzer on
          
        }else{
          set_bit(PORTC, RED_LED_PIN);
          clr_bit(TCCR0A, COM0A1);    // Buzzer off
        }

        if(flag_wrong_inclination){
          OCR1A = 3400;     //Activate servo
        }else{
          OCR1A = 1000;     //Deactivate servo
        }

        system_state = S_READING;

        break;
    }
    _delay_ms(50);
  }

  return 0;
}


void timer2_init(){
  /* Configure pwm output*/
  set_bit(DDRB, MOTOR1_PIN);
  clr_bit(PORTB, MOTOR1_PIN);
  
  /* Configure pwm output*/
  set_bit(DDRD, MOTOR2_PIN);
  clr_bit(PORTD, MOTOR2_PIN);

  TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20);      // PWM with corrected phase on A
  TCCR2B = (1 << CS21);                                       // Prescaler = 8
  TIMSK2 = (1 << TOIE2);                                      //Enable overflow interrupt
  
  OCR2A = 0;                                                  // Configure initial duty cycle for A
  OCR2B = 0;                                                  // Configure initial duty cycle for B
}

void timer0_init(){
  /* Configure OC0A as output*/
  set_bit(DDRD, BUZZER_PIN);
  
  TCCR0A = (1 << WGM01) | (1 << WGM00);    // Fast PWM
  TCCR0B = (1 << CS01);      // prescaler = 8 (T = 0.5 us)

  OCR0A = 200;
}

void timer1_init(){
 
  set_bit(DDRB, SERVO_PIN);

  // Fast PWM, TOP = ICR1
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8

  ICR1 = 39999; // TOP for 50 Hz -> T = 20 ms
  OCR1A = 1000; //Deactivate servo
}

void adc_init(){
  ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX2);     // Vcc reference and mux 6 selected
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);       // Enable ADC with prescaler = 128
  ADCSRB = 0x00;   // Continuous mode
}

void button_init(){
  clr_bit(DDRD, BUTTON_PIN);
  set_bit(PORTD, BUTTON_PIN);     //Pull up

  EICRA = (1 << ISC01);            // Trigger: falling edge
  EIMSK = (1 << INT0);
}


void hcsr04_init(void) {
    // Set up trigger pin
    set_bit(DDRC, TRIGGER_PIN);
    clr_bit(PORTC, TRIGGER_PIN); // Start LOW

    // Set up echo pin
    clr_bit(PORTC, ECHO_PIN);
    clr_bit(DDRC, ECHO_PIN);
}

// ----- Send 10us Trigger Pulse using Timer 1 -----
void hcsr04_send_trigger(void) {
    set_bit(PORTC, TRIGGER_PIN); // Set TRIG HIGH

    // Wait 10us using Timer 1 (prescaler 8, 16MHz: 0.5us per tick)
    uint16_t start = TCNT1;
    uint16_t icr = ICR1;
    while (1) {
        uint16_t now = TCNT1;
        uint16_t waited = (now >= start) ? (now - start) : (icr - start + 1 + now);
        if (waited >= 20) break; // 20 ticks = 10us
    }

    clr_bit(PORTC, TRIGGER_PIN); // Set TRIG LOW
}


// ----- Read Echo Duration in Microseconds -----
uint16_t hcsr04_read_echo_us(void) {
    uint32_t timeout = 60000UL;    // Timeout threshold (about 30ms at 0.5us/tick)
    uint32_t count = 0;
    uint16_t icr = ICR1;

    // Wait for ECHO HIGH (start of pulse)
    while (!test_bit(PINC, ECHO_PIN)) {
        if (++count > timeout) return 0; // Timeout: No echo detected
    }
    uint16_t start = TCNT1;

    // Wait for ECHO LOW (end of pulse)
    count = 0;
    while (test_bit(PINC, ECHO_PIN)) {
        if (++count > timeout) return 0; // Timeout: Echo too long
    }
    uint16_t end = TCNT1;

    // Calculate ticks, handle wraparound
    uint16_t ticks = (end >= start) ? (end - start) : (icr - start + 1 + end);

    // Convert to microseconds (0.5us per tick)
    return ticks / 2;
}

// ----- Full Measurement Function: Returns echo duration in us -----
uint16_t hcsr04_get_distance_cm(void) {
    hcsr04_send_trigger();
    uint16_t echo_duration = hcsr04_read_echo_us(); 
    return echo_duration / 58;
}


ISR(INT0_vect){
  flag_stop_requested ^= 0x0F; //TOGGLE BITS
}

ISR(TIMER2_OVF_vect){
  static uint8_t counter = 200;                                       // T = (200 * 12.8 us) = 25.6 ms

  if(!(counter--)){
    /* Enable ADC conversion */
    set_bit(ADCSRA, ADSC);

    /* Read tilt sensor */
    if(test_bit(PINB, SW520D_PIN)){   // HIGH: tilted
      flag_wrong_inclination |= 0x0F;
    }else{                            //LOW:  tilted
      flag_wrong_inclination &= 0xF0;
    }
    
    /* Reset counter*/
    counter = 200;
  }
}

ISR(ADC_vect){
  uint8_t adc_channel = ADMUX & 0x0F;

  if(adc_channel == 6){     //LDR SENSOR

    if(ADC > LDR_LIMIT){
      flag_presence_detected |= 0x0F;
    }else{
      flag_presence_detected &= 0xF0;
    }

    adc_channel++;

  }else if(adc_channel == 7){           //LM35 SENSOR
    temperature = ADC * 5.0 / 1023 / 0.01;
    
    if(temperature < 10 || temperature > 40){
      flag_critical_temperature |= 0x0F;
    }else{
      flag_critical_temperature &= 0xF0;
    }
    adc_channel--;
  }

  ADMUX = (ADMUX & 0xF0) | (adc_channel & 0x0F);
}

ISR(TWI_vect) {

  switch (TWSR & 0xF8) { // Máscara para ignorar os 3 bits menos significativos
    // Escrita: Mestre quer enviar dados
    case 0x60: // SLA+W recebido, ACK enviado
      i2c_com_state = I2C_RECEIVING_OPCODE;
      TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT); // Prepara para receber dado
      break;

    case 0x80: // Dado recebido, ACK enviado
      
      switch(i2c_com_state){
        
        case I2C_RECEIVING_OPCODE:
          current_opcode = TWDR;
          if(current_opcode == 0x03){
            flag_stop_requested ^= 0x0F; //TOGGLE BITS
          }else{
            i2c_com_state = I2C_RECEIVING_VALUE;
          }
          break;
        
        case I2C_RECEIVING_VALUE:
          if(current_opcode == 0x01){
            OCR2A = TWDR;
          }else if(current_opcode == 0x02){
            OCR2B = TWDR;
          }

          i2c_com_state = I2C_RECEIVING_OPCODE;     
          break;
      }  
                      
      TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT); // Prepara para receber mais dados
      break;

    // Leitura: Mestre quer ler dados
    case 0xA8: // SLA+R recebido, ACK enviado
      TWDR = active_errors; // Prepara o dado a ser enviado
      TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Continua enviando
      break;

    case 0xC0: // Dado enviado, NACK recebido
    case 0xC8: // Último dado enviado, ACK recebido
      TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT); // Prepara para próxima comunicação
      break;

    // Erro ou estado inesperado
    default:
      TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT); // Reinicia o TWI
      break;
  }
}