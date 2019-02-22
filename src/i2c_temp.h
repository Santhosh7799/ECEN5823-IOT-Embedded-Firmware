/*
 * File : i2c_temp.h
 * Author: Santhosh Thummanapalli
 * Created on 2/6/2019
 *
 *Description: This is header file used for function declarations  for the i2c_temp.h
 *
 *
 */

#ifndef I2C_TEMP_H_
#define I2C_TEMP_H_

#include "i2cspm.h"
#include "gpio.h"
#include "letimer.h"
#include "sleep.h"

#define TEMP_ENABLE_PORT gpioPortD
#define I2C0_SCL_PORT	gpioPortC
#define I2C0_SDA_PORT 	gpioPortC
#define TEMP_ENABLE_PIN  15
#define I2C0_SCL_PIN 	10
#define I2C0_SDA_PIN 	11
#define SLAVE_ADDR 		(0x40) //temperature sensor address
#define TEMP_READ_REG_ADD	(0xE3)
#define I2C_WORKING_ENERGY_MODE (EM1)



void i2ctemp_init();
void i2ctemp_enable();
void i2ctemp_disable();
void i2ctemp_On();
void i2ctemp_Sleep();
void i2c_read_tempreg(I2C_TypeDef *i2c, uint8_t slaveAddr, uint8_t reg_addr);
void i2c_write_tempreg(I2C_TypeDef *i2c, uint8_t slaveAddr, uint8_t reg_addr);
float get_temp_value();

#endif


