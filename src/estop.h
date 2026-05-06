#pragma once

#include "common.h"
void ESTOP_init();

// Emergency Stop callback. returns true if the estop is recoverable.
extern bool ESTOP_Callback() __attribute__((weak));