/*
 * File : Le_timer.h
 * Author: Santhosh Thummanapalli
 * Created on 1/29/2019
 *
 *Description: This is header file used for function declarations  for the le_timer
 *
 *
 */

#ifndef LE_TIMER_H_
#define LE_TIMER_H_
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_letimer.h"

#define LED_ONTIME    175
#define TOTAL_PERIOD  2250
#define LowestEnergyMode EM2

typedef enum EnergyModeVal{
	EM0,
	EM1,
	EM2,
	EM3,
	EM4
}Energystate;



uint32_t PrescalerValCal(uint32_t period);
void letimer_init();

#endif
