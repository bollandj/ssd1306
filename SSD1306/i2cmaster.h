#ifndef I2CMASTER_H
#define I2CMASTER_H

extern void i2c_init();
extern uint8_t i2c_start(uint8_t address);
extern uint8_t i2c_write(uint8_t data);
extern uint8_t i2c_read_ack();
extern uint8_t i2c_read_nack();
extern uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length);
extern uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
extern uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
extern uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
extern void i2c_stop();

#endif // I2CMASTER_H
