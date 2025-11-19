#pragma once

#include "stm32f4xx.h"
#include "common.h"
#include <stdint.h>
#include <stdio.h>

#define GCODE_MAX_ARGS 4

typedef struct GcodeArg {
	char id;
	// TODO: Make it so value can be either an int or a float
	int value;
} GcodeArg;

typedef struct Gcode {
	// Number of arguments supplied
	int num_args;
	// Up to 4 args supported at this time
	GcodeArg args[GCODE_MAX_ARGS];
} Gcode;

Gcode gcode_decode(FILE* stream);
GcodeArg gcode_arg_decode(FILE* stream);