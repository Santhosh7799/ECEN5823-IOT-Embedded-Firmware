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
#include "gpio.h"
#include "sleep.h"
#include "letimer.h"
#include "i2c_temp.h"
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

//static void delayApproxOneSecond(void)
//{
//	/**
//	 * Wait loops are a bad idea in general!  Don't copy this code in future assignments!
//	 * We'll discuss how to do this a better way in the next assignment.
//	 */
//	volatile int i;
//	for (i = 0; i < 3500000; ) {
//		  i=i+1;
//	}
//}


int main(void)
{

  float* Temp_value;

  int a;
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

  /* Infinite loop */
  while (1) {
      if(Allow_temp==1)
     {
    	  Allow_temp=0;
    	 a = get_temp_value(Temp_value);
      }
      else
      {
    	  //EMU_EnterEM3(true);
	      SLEEP_Sleep();
      }
  }
}
