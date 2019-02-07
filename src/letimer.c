/*
 * File : Le_timer.c
 * Author: Santhosh Thummanapalli
 * Created on 1/29/2019
 *
 *Description: This is the file used for function definitions  for the le_timer
 *
 *
 */

#include "letimer.h"
#include "gpio.h"
#include "sleep.h"


uint32_t PrescalerValCal(uint32_t period)
{
   uint32_t a,b;
   b=0;
   a = (65536000/period);
   if(a >= 32768)
   {
	   b=1;
   }
   else if(32768>a && a>= 16384)
   {
	   b=2;
   }
   else if(a < 16384 && a>= 8192)
   {
	   b=4;
   }
   else
	   printf("please enter value less than 8000");

   return b;
}




void letimer_init()
{
	uint32_t Divider,comp0value,Clockfreq; //comp1value is removed

	CMU_ClockEnable(cmuClock_HFLE, true);

	CMU_OscillatorEnable (cmuOsc_LFXO, true, true);
    CMU_ClockSelectSet (cmuClock_LFA, cmuSelect_LFXO);
	Divider = PrescalerValCal(TOTAL_PERIOD);
	CMU_ClockEnable(cmuClock_LFA,true);
	CMU_ClockDivSet(cmuClock_LETIMER0,Divider);
	CMU_ClockEnable(cmuClock_LETIMER0,true);

	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	letimerInit.comp0Top  = true;
	LETIMER_Init(LETIMER0, &letimerInit );



	Clockfreq = CMU_ClockFreqGet(cmuClock_LFA);


	comp0value=((Clockfreq*TOTAL_PERIOD)/(Divider*1000));


	LETIMER_CompareSet(LETIMER0, 0, comp0value);


	LETIMER_IntEnable(LETIMER0,LETIMER_IF_UF);
	NVIC_EnableIRQ(LETIMER0_IRQn);
	LETIMER_Enable(LETIMER0, true);
}

void timerWaitUs(uint32_t us_wait)
{
	uint32_t Clockfreq,Required_ticks,initial_tickvalue,current_tickvalue;
	Clockfreq = CMU_ClockFreqGet(cmuClock_LFA);
	Required_ticks = us_wait/Clockfreq ;
	initial_tickvalue= LETIMER_CounterGet(LETIMER0);
	do{
		current_tickvalue = LETIMER_CounterGet(LETIMER0);
	}while((current_tickvalue - initial_tickvalue) < Required_ticks);
}



void LETIMER0_IRQHandler(void)
{
   __disable_irq();
  // Get pending flags and clear
  int irq_flags =  LETIMER_IntGet(LETIMER0);
  LETIMER_IntClear(LETIMER0, irq_flags);
 // if((LETIMER_IF_UF & irq_flags))
 // {
  Allow_temp = 1;
 // }
 // GPIO_PinOutToggle(LED0_port, LED0_pin);
  __enable_irq();
}
