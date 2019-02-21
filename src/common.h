#ifndef __COMMON_H
#define __COMMON_H

#include "em_letimer.h"

#define TotalTicksPerCycle 65535     //FFFF is the total ticks
#define TimePerTick 1/32768          //assuming clock frequency of 32768Hz.
#define TimeTakenPerCycle 2000          //this is in ms and assuming clock frequency of 32768Hz.


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
