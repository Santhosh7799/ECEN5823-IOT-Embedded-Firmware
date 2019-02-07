/*
 * File : i2c.c
 * Author: Santhosh Thummanapalli
 * Created on 2/6/2019
 *
 *Description: This is the file contains function definitions using i2c.
 *             These functions are used to obtain the temperature from the
 *             Si7021 sensor on the gecko board
 *             the four main functions used here are
 *             1. powering the si7021
 *             2. obtaining the temperature value
 *             3. powering off the si7021
 *
 *
 */


#include "i2c_temp.h"
#include "em_i2c.h"
#include "log.h"

volatile I2C_TransferReturn_TypeDef i2c_status;
void i2ctemp_init()
{
	GPIO_PinModeSet(TEMP_ENABLE_PORT, TEMP_ENABLE_PIN, gpioModePushPull, 0);
	I2CSPM_Init_TypeDef init_temp ={
			I2C0,                       /* Use I2C instance 0 */                       \
		    gpioPortC,                  /* SCL port */                                 \
		    10,                         /* SCL pin */                                  \
		    gpioPortC,                  /* SDA port */                                 \
		    11,                         /* SDA pin */                                  \
		    14,                         /* Location of SCL */                          \
		    16,                         /* Location of SDA */                          \
		    0,                          /* Use currently configured reference clock */ \
		    I2C_FREQ_STANDARD_MAX,      /* Set to standard rate  */                    \
		    i2cClockHLRStandard,        /* Set to use 4:4 low/high duty cycle */       \
		  };
	I2CSPM_Init(&init_temp);
	LOG_INFO("temperature sensor is initialized");
}

void i2ctemp_enable()
{
		GPIO_PinOutSet(TEMP_ENABLE_PORT, TEMP_ENABLE_PIN);
}
void i2ctemp_disable()
{

		GPIO_PinOutClear(TEMP_ENABLE_PORT, TEMP_ENABLE_PIN);
}

//wakes up the sensor
void i2ctemp_On()
{
	i2ctemp_enable();

	//need to modify this part
	timerWaitUs(18000);


	GPIO_PinModeSet(I2C0_SCL_PORT, I2C0_SCL_PIN, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(I2C0_SDA_PORT, I2C0_SDA_PIN, gpioModeWiredAnd, 1);

	//reset the i2c bus
   for (int i = 0; i < 9; i++)
   {
		GPIO_PinOutClear(I2C0_SCL_PORT, I2C0_SCL_PIN);
		GPIO_PinOutSet(I2C0_SCL_PORT, I2C0_SCL_PIN);
   }
   LOG_INFO("temperature sensor is ready to use");
}

void i2ctemp_Sleep()
{
	GPIO_PinModeSet(I2C0_SCL_PORT, I2C0_SCL_PIN, gpioModeDisabled, 1);
	GPIO_PinModeSet(I2C0_SDA_PORT, I2C0_SDA_PIN, gpioModeDisabled, 1);
	i2ctemp_disable();
	LOG_INFO("temperature sensor is disabled");
}



int get_temp_value(float* Temp_value)
{
	int ret_val = 0;
	uint16_t tempData = 0;
	uint16_t a=0;

	i2ctemp_On();

	SLEEP_SleepBlockBegin(I2C_WORKING_ENERGY_MODE+1);

	ret_val = i2c_read_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD,&tempData);

	if (ret_val < 0)
	{
		return ret_val;
	}
	LOG_INFO("I2C_transaction successfull %d",ret_val);
	 a = ((((17572 * (tempData)) / 65536) - 4685)/100);

*Temp_value = ((((17572 * (float)(tempData)) / 65536) - 4685)/100);
	 LOG_INFO("read temperature status %d",a);
	SLEEP_SleepBlockEnd (I2C_WORKING_ENERGY_MODE+1);
	 i2ctemp_Sleep();
	return ret_val;
}

int i2c_read_tempreg(I2C_TypeDef *i2c, uint8_t slaveAddr, uint8_t reg_addr, uint16_t *Temp_data)
{
	I2C_TransferSeq_TypeDef seq;
	uint8_t buffer[3] = {0};


	//since using 7 -bit address mode, we shift the address
	seq.addr = (uint16_t)((slaveAddr<<1) & ((uint8_t)0xFE));

	buffer[0] =  reg_addr;
	seq.buf[0].data = buffer;
	seq.buf[0].len = 1;

	seq.flags = I2C_FLAG_WRITE_READ;

	seq.buf[1].data = buffer+1;
	seq.buf[1].len = 2;

	i2c_status = I2CSPM_Transfer(I2C0, &seq);

	*Temp_data = (uint16_t)(((uint16_t)buffer[1]) << 8);
	*Temp_data  = ((*Temp_data ) |(buffer[2] & 0xFC));

	return i2c_status;
}









