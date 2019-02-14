/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "log.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_core.h"
#include "gpio.h"
#include "sleep.h"
#include "letimer.h"
#include "i2c_temp.h"
#include "common.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif



uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};


int main(void)
{

  float* Temp_value = NULL;
  enum TempSensorState next_state;
  TotalCyclesCompleted =0;
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();
  logInit();
  gpioInit();
  i2ctemp_init();

  // Initialize stack
  gecko_init(&config);



  letimer_init();


  next_state = Temp_Sensor_wait_For_PowerOn;
  /* Infinite loop */
  while (1) {

	 SLEEP_Sleep();
     switch(next_state)
     {
     case Temp_Sensor_wait_For_PowerOn :
    	 if(SchedulerEventSet[EventHandlePowerOn])
    	 {
    		 CORE_DECLARE_IRQ_STATE;
    		CORE_ENTER_CRITICAL();
    		SchedulerEventSet[EventHandlePowerOn]=0;
    		TotalCyclesCompleted++;
    		CORE_EXIT_CRITICAL();
             i2ctemp_On();
             next_state =  Temp_Sensor_wait_For_Sensor_Enabled;
    	     timerSetEventInMs(80);
    	 }
    	 break ;

     case Temp_Sensor_wait_For_Sensor_Enabled:
    	 if(SchedulerEventSet[EventHandleI2CEnabled])
    	 {
    	     CORE_DECLARE_IRQ_STATE;
    	     CORE_ENTER_CRITICAL();
    	     SchedulerEventSet[EventHandleI2CEnabled]=0;
    	     CORE_EXIT_CRITICAL();

    	     i2c_write_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD);
             next_state =  Temp_Sensor_wait_For_Write_Complete;
    	 }


    	 break ;
     case Temp_Sensor_wait_For_Write_Complete :
    	 if( SchedulerEventSet[EventHandleI2CTransferComplete])
    	  {
    		 CORE_DECLARE_IRQ_STATE;
    	     CORE_ENTER_CRITICAL();
    	     SchedulerEventSet[EventHandleI2CTransferComplete]=0;
    	     CORE_EXIT_CRITICAL();

    	     i2c_read_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD);
    	     next_state =  Temp_Sensor_wait_For_Read_Complete;
    	  }
       	 else if(SchedulerEventSet[EventHandleI2CTransferFail])
    	 {

    	     CORE_DECLARE_IRQ_STATE;
    	     CORE_ENTER_CRITICAL();
    	     SchedulerEventSet[EventHandleI2CTransferFail]=0;
    	     CORE_EXIT_CRITICAL();
    	     i2c_write_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD);
    	 }
    	 else
    	 {
    		 if( SchedulerEventSet[EventHandleI2CTransferInProgress])
    		 	  {
    		    		 CORE_DECLARE_IRQ_STATE;
    		    	     CORE_ENTER_CRITICAL();
    		    	     SchedulerEventSet[EventHandleI2CTransferInProgress]=0;
    		    	     CORE_EXIT_CRITICAL();
    		       }
    	 }

         break ;
     case 	Temp_Sensor_wait_For_Read_Complete:
    	 if( SchedulerEventSet[EventHandleI2CTransferComplete])
    	     	  {
    	     		 CORE_DECLARE_IRQ_STATE;
    	     	     CORE_ENTER_CRITICAL();
    	     	     SchedulerEventSet[EventHandleI2CTransferComplete]=0;
    	     	     CORE_EXIT_CRITICAL();
    	     	    get_temp_value();
    	     	     next_state =  Temp_Sensor_wait_For_PowerOff;
    	     	  }
    	        	 else if(SchedulerEventSet[EventHandleI2CTransferFail])
    	     	 {

    	     	     CORE_DECLARE_IRQ_STATE;
    	     	     CORE_ENTER_CRITICAL();
    	     	     SchedulerEventSet[EventHandleI2CTransferFail]=0;
    	     	     CORE_EXIT_CRITICAL();
    	     	     i2c_read_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD);
    	     	 }
    	     	 else
    	     	 {
    	     		 if( SchedulerEventSet[EventHandleI2CTransferInProgress])
    	     		 	  {
    	     		    		 CORE_DECLARE_IRQ_STATE;
    	     		    	     CORE_ENTER_CRITICAL();
    	     		    	     SchedulerEventSet[EventHandleI2CTransferInProgress]=0;
    	     		    	     CORE_EXIT_CRITICAL();
    	     		       }
    	     	 }

    	 break;
     case Temp_Sensor_wait_For_PowerOff:
    	 if( SchedulerEventSet[EventHandlePowerOff])
    	   {
    	    CORE_DECLARE_IRQ_STATE;
    	    CORE_ENTER_CRITICAL();
    		SLEEP_SleepBlockEnd (I2C_WORKING_ENERGY_MODE+1);
    		CORE_EXIT_CRITICAL();
    	    next_state = Temp_Sensor_wait_For_PowerOn;
    	   }
    	 break;
     default:
    	 break;
     }

  }
}
