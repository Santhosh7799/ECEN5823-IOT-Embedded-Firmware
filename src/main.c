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
#include "infrastructure.h"


#include "gecko_ble_errors.h"

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

//function to convert and send the temp value. This function is  a modified vertion from BLE_THERMOMETER developed by si labs
void Updatetemperature(float tempData)
{
  uint8_t TempBuffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
  uint8_t flags = 0x00;   /* flags set as 0 for Celsius, no time stamp and no temperature type. */
  uint8_t *p = TempBuffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */
  /* Convert flags to bitstream and append them in the temperature data buffer (TempBuffer) */
   UINT8_TO_BITSTREAM(p, flags);


  UINT32_TO_BITSTREAM(p, (FLT_TO_UINT32(tempData, -3)));


   /* This enables the Health Thermometer in the Blue Gecko app to display the temperature.
   *  0xFF as connection ID will send indications to all connections. */
  gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_temperature_measurement, 5, TempBuffer);
}












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

//  float* Temp_value = NULL;
  enum TempSensorState next_state;
  TotalCyclesCompleted =0;
  int8_t rssi;
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  gpioInit();


  // Initialize stack
  gecko_init(&config);



  letimer_init();
  logInit();
  i2ctemp_init();
  next_state = Temp_Sensor_wait_For_PowerOn;
  /* Infinite loop */
  while (1) {

	  struct gecko_cmd_packet* evt;

	  evt = gecko_wait_event();

	  switch (BGLIB_MSG_ID(evt->header)) {

	       case gecko_evt_system_boot_id:
	        	BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0,MIN_ADVERTISING_INTERVAL_VALUE, MAX_ADVERTISING_INTERVAL_VALUE, 0, 0));
	        	BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0,le_gap_general_discoverable, le_gap_undirected_connectable));
                break;

	       case gecko_evt_le_connection_opened_id:

	            gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection,MIN_CONNECTION_INTERVAL_VALUE,\
	        												  MAX_CONNECTION_INTERVAL_VALUE, \
	        												  SLAVE_LATENCY, \
	        												  CONNECTION_TIMEOUT_MS);

	            break;

	       case gecko_evt_gatt_server_characteristic_status_id:
	     	  if (evt-> data.evt_gatt_server_characteristic_status.status_flags == gatt_server_confirmation)
	     	  {
	     		  gecko_cmd_le_connection_get_rssi(evt-> data.evt_gatt_server_characteristic_status.connection);
	     	  }
	         break;
// For Updating the transmission power. this function takes input in 0.1dbm units. so all the values are converted.
	       case gecko_evt_le_connection_rssi_id:

	 			rssi = evt->data.evt_le_connection_rssi.rssi;
	 			gecko_cmd_system_halt(1);
	 			if(rssi > -35)
	 				gecko_cmd_system_set_tx_power(BLE_TX_MIN);
	 			else if(rssi > -45)
	 				gecko_cmd_system_set_tx_power(-200);
	 			else if(rssi > -55)
	 				gecko_cmd_system_set_tx_power(-150);
	 			else if(rssi > -65)
	 				gecko_cmd_system_set_tx_power(-50);
	 			else if(rssi > -75)
	 				gecko_cmd_system_set_tx_power(0);
	 			else if(rssi > -85)
	 				gecko_cmd_system_set_tx_power(50);
	 			else
	 				gecko_cmd_system_set_tx_power(BLE_TX_MAX);
	 			gecko_cmd_system_halt(0);
	 			break;

           case gecko_evt_system_external_signal_id:
    	 	 	    switch(next_state)
    	 	 	    {
    	 	 	 	      case Temp_Sensor_wait_For_PowerOn :
    	                                 if(SchedulerEventSet[EventHandlePowerOn])
    	                                 {
    	                                	 	CORE_DECLARE_IRQ_STATE;
    	                                	 	CORE_ENTER_CRITICAL();
    	                                	 	SchedulerEventSet[EventHandlePowerOn]=0;
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
    	 	 	 							NVIC_DisableIRQ(I2C0_IRQn);
    	 	 	 							LOG_DEBUG("temperature sensor write is successull\n");
    	 	 	 							i2c_read_tempreg(I2C0,SLAVE_ADDR, TEMP_READ_REG_ADD);
    	 	 	 							next_state =  Temp_Sensor_wait_For_Read_Complete;
    	 	 	 						}
    	 	 	 						else if(SchedulerEventSet[EventHandleI2CTransferFail])
    	 	 	 						{
    	 	 	 							CORE_DECLARE_IRQ_STATE;
    	 	 	 							CORE_ENTER_CRITICAL();
    	 	 	 							SchedulerEventSet[EventHandleI2CTransferFail]=0;
    	 	 	 							CORE_EXIT_CRITICAL();
    	 	 	 							NVIC_DisableIRQ(I2C0_IRQn);
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
    	 	 	 	      case Temp_Sensor_wait_For_Read_Complete:
    	 	 	 						if( SchedulerEventSet[EventHandleI2CTransferComplete])
    	 	 	 						{
    	 	 	 							float tempData;
    	 	 	 							CORE_DECLARE_IRQ_STATE;
    	 	 	 							CORE_ENTER_CRITICAL();
    	 	 	 							SchedulerEventSet[EventHandleI2CTransferComplete]=0;
    	 	 	 							CORE_EXIT_CRITICAL();
    	 	 	 							NVIC_DisableIRQ(I2C0_IRQn);
    	 	 	 							LOG_DEBUG("temperature sensor read is successull\n");
    	 	 	 							tempData = get_temp_value();
    	 	 	 							Updatetemperature(tempData);
    	 	 	 							next_state =  Temp_Sensor_wait_For_PowerOff;
    	 	 	 						}
    	 	 	 						else if(SchedulerEventSet[EventHandleI2CTransferFail])
    	 	 	 						{
    	 	 	 							CORE_DECLARE_IRQ_STATE;
    	 	 	 							CORE_ENTER_CRITICAL();
    	 	 	 							SchedulerEventSet[EventHandleI2CTransferFail]=0;
    	 	 	 							CORE_EXIT_CRITICAL();
    	 	 	 							NVIC_DisableIRQ(I2C0_IRQn);
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
                break;

    	  case gecko_evt_le_connection_closed_id:
    		         gecko_cmd_system_halt(1);
    	 	 	     gecko_cmd_system_set_tx_power(0);
    	 	 	     gecko_cmd_system_halt(0);
    	 	 	     BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0,le_gap_general_discoverable, le_gap_undirected_connectable));
    	 	   break;
	}

  }
}



