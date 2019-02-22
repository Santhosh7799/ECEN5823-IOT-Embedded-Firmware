#ifndef __COMMON_H
#define __COMMON_H

#include "em_letimer.h"

#define TotalTicksPerCycle 65535     //FFFF is the total ticks
#define TimePerTick 1/32768          //assuming clock frequency of 32768Hz.
#define TimeTakenPerCycle 2000          //this is in ms and assuming clock frequency of 32768Hz.


//BLUETOOTH Parameters
#define MIN_ADVERTISING_INTERVAL	(250)     // this is in ms
#define MAX_ADVERTISING_INTERVAL	(250)     // this is in ms
#define MIN_CONNECTION_INTERVAL		(75)      // this is in ms
#define MAX_CONNECTION_INTERVAL		(75)      // this is in ms
#define SLAVE_LATENCY_MS			(300)

//these are bluetooth values for use in functions.
#define MIN_ADVERTISING_INTERVAL_VALUE	(400) //Value for advertising interval is obtained from formula=> advertising_time = value *0.625.
#define MAX_ADVERTISING_INTERVAL_VALUE  (400)
#define MIN_CONNECTION_INTERVAL_VALUE	(60)  //Value for connection interval is obtained from formula=> connection_time = value * 1.25
#define MAX_CONNECTION_INTERVAL_VALUE	(60)
#define SLAVE_LATENCY					(3)	  //Slave_Latency is obtained from formula=> Slave_latency = (LE_SLAVE_LATENCY_MS/LE_MAX_CONNECTION_INTERVAL_MS) - 1
#define CONNECTION_TIMEOUT_MS           (600) // as timeout should be >= (1+slave_latency)*(connection_interval_time*2)
#define BLE_TX_MAX 80           //Here tx maximum is 8 db for the radio but giving it in 0.1dbm units for function requirements
#define BLE_TX_MIN -260         //Here tx minimum is -26 db for the radio but giving it in 0.1dbm units for function requirements





 enum TempSensorState{
	Temp_Sensor_wait_For_PowerOn,
	Temp_Sensor_wait_For_Sensor_Enabled,
	Temp_Sensor_wait_For_Write_Complete,
	Temp_Sensor_wait_For_Read_Complete,
	Temp_Sensor_wait_For_PowerOff,
};

bool SchedulerEventSet[6];
enum SetSechdulerEvent
{
	EventHandlePowerOn,
	EventHandleI2CEnabled,
	EventHandleI2CTransferComplete,
	EventHandleI2CTransferFail,
	EventHandleI2CTransferInProgress,
	EventHandlePowerOff
};



#endif /* __COMMON_H_ */
