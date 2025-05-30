#ifndef I2C_COM_H
#define I2C_COM_H

#include <avr/io.h>

#define I2C_SDA_PIN PC4                     // SDA pin number
#define I2C_SCL_PIN PC5                     // SCL pin number
#define I2C_RATE 100000
#define I2C_TIMEOUT 10000                   // Timeout for I2C operations


#define PAYLOAD_SIZE 2                      // (PAYLOAD_SIZE * uint8_t) bytes to be received


extern const uint8_t SLAVE_ADDRESS;         // Slave address for I2C communication
extern const uint8_t OP_SET_MOTOR1;
extern const uint8_t OP_SET_MOTOR2;
extern const uint8_t OP_BUTTON_PRESSED;


/* Function prototypes*/
void i2c_master_init(void);
void i2c_slave_init(uint8_t slave_address);
void i2c_start_read(bool *more_data_to_send, bool *read_complete);
void i2c_start_write(bool *more_data_to_send, uint8_t *data_index);


#endif