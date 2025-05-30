#include "i2c_com.h"



void i2c_master_init(void) {
    TWSR = 0; // Sem prescaler
    TWBR = (((F_CPU / I2C_RATE) - 16) / 2); // Frequência de 100 kHz
    TWCR = (1 << TWEN) | (1 << TWIE); // Habilita TWI e interrupções
}

void i2c_slave_init(uint8_t slave_address) {
    TWAR = (slave_address << 1); // Configura endereço do escravo (7 bits)
    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA); // Habilita TWI, interrupções e ACK automático
}

void i2c_start_write(bool *more_data_to_send, uint8_t *data_index) {
    *data_index = 0; // Reseta o índice de envio
    *more_data_to_send = true; // Indica que há mais dados para enviar

    // Envia condição de START
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Envia START
}

void i2c_start_read(bool *more_data_to_send, bool *read_complete) {
    *read_complete = 0; // Reseta a flag de leitura
	*more_data_to_send = 0;
  
    // Envia condição de START
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Envia START
}




