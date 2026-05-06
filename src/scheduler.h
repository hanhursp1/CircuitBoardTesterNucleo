#pragma once

#include "common.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#define SCH_TIM TIM1
typedef TIM_TypeDef* Timer;

typedef struct Scheduler {
	Timer t;
} Scheduler;

// Since I don't know how to create process tables or store registers,
// functionality is going to be limited

void scheduling_test();