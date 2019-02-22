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
#include "em_core.h"
#include "log.h"
#include "common.h"
#include "native_gecko.h"

I2C_TransferSeq_TypeDef seq;
uint8_t buffer[3] = {0};

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

	LOG_DEBUG("temperature sensor is initialized \n");
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
/*	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	SLEEP_SleepBlockBegin(I2C_WORKING_ENERGY_MODE+1);
	CORE_EXIT_CRITICAL();*/


	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
    i2ctemp_enable();

	GPIO_PinModeSet(I2C0_SCL_PORT, I2C0_SCL_PIN, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(I2C0_SDA_PORT, I2C0_SDA_PIN, gpioModeWiredAnd, 1);

	//reset the i2c bus
   for (int i = 0; i < 9; i++)
   {
		GPIO_PinOutClear(I2C0_SCL_PORT, I2C0_SCL_PIN);
		GPIO_PinOutSet(I2C0_SCL_PORT, I2C0_SCL_PIN);
   }
   LOG_DEBUG("temperature sensor is ready to use\n");
}

void i2ctemp_Sleep()
{
	GPIO_PinModeSet(I2C0_SCL_PORT, I2C0_SCL_PIN, gpioModeDisabled, 1);
	GPIO_PinModeSet(I2C0_SDA_PORT, I2C0_SDA_PIN, gpioModeDisabled, 1);
	i2ctemp_disable();
	SchedulerEventSet[EventHandlePowerOff]=1;
	LOG_DEBUG("temperature sensor is disabled\n");
	timerSetEventInMs(1);
}



float get_temp_value()
{

	uint32_t tempData = 0;
	float temp_val =0;


	// assuming its a 7 bit address with temperature resolution of 10
	tempData= (uint16_t)(((uint16_t)buffer[0]) << 8);
	tempData  = ((tempData ) |(buffer[1] & 0xFC));

	LOG_INFO("I2C_transaction successfull %d \n",tempData);

     temp_val = ((((17572 * (float)(tempData)) / 65536) - 4685)/100);
	 LOG_INFO("read temperature in float %f \n",temp_val);
	 i2ctemp_Sleep();
	return temp_val;
}

void i2c_read_tempreg(I2C_TypeDef *i2c, uint8_t slaveAddr, uint8_t reg_addr)
{

	//since using 7 -bit address mode, we shift the address
	seq.addr = (uint16_t)((slaveAddr<<1) & ((uint8_t)0xFE));

	buffer[0] =  reg_addr;
	seq.buf[0].data = buffer;
	seq.buf[0].len = 1;

	seq.flags = I2C_FLAG_READ;
    buffer[1] = 0;
	seq.buf[1].data = buffer+1;
	seq.buf[1].len = 2;

	I2C_TransferInit(i2c, &seq);
	  NVIC_EnableIRQ(I2C0_IRQn);
}

void i2c_write_tempreg(I2C_TypeDef *i2c, uint8_t slaveAddr, uint8_t reg_addr)
{

	//since using 7 -bit address mode, we shift the address
	seq.addr = (uint16_t)((slaveAddr<<1) & ((uint8_t)0xFE));

	buffer[0] =  reg_addr;
	seq.buf[0].data = buffer;
	seq.buf[0].len = 1;

	seq.flags = I2C_FLAG_WRITE;

	buffer[1] = 0;
	seq.buf[1].data = buffer+1;
	seq.buf[1].len = 1;

	I2C_TransferInit(i2c, &seq);
	  NVIC_EnableIRQ(I2C0_IRQn);


}




void I2C0_IRQHandler()
{

		//Just run the I2C_Transfer function that checks interrupts flags and returns the appropriate status
	i2c_status	=I2C_Transfer(I2C0);
	if(i2c_status != i2cTransferInProgress)
	{
		if(i2c_status == i2cTransferDone)
		{
		    SchedulerEventSet[EventHandleI2CTransferComplete]=1;
		    gecko_external_signal(EventHandleI2CTransferComplete);
		}
		else
		{
			SchedulerEventSet[EventHandleI2CTransferFail]=1;
			gecko_external_signal(EventHandleI2CTransferFail);
		}
	}
	else
	{
		SchedulerEventSet[EventHandleI2CTransferInProgress]=1;
		gecko_external_signal(EventHandleI2CTransferInProgress);
	}

}



