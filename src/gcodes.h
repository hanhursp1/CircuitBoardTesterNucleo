#pragma once

#include "stm32f4xx.h"
#include "common.h"
#include <stdint.h>
#include <stdio.h>

#define GCODE_MAX_ARGS 4

typedef struct GcodeArg {
	char id;
	// TODO: Make it so value can be either an int or a float
	int32_t value;
} GcodeArg;

typedef struct Gcode {
	// Number of arguments supplied
	int num_args;
	// Up to 4 args supported at this time
	GcodeArg args[GCODE_MAX_ARGS];
} Gcode;

Gcode gcode_decode(FILE* stream);
GcodeArg gcode_arg_decode(FILE* stream);

void __gcode_assert(const char * filename, int line, const char * funcname, const char * what_broke);

// Based on the ANSI assert implementation. I've modified it to simply return rather than
// panic or loop forever.
#define gcode_assert(_thing) if (!(_thing)) { \
	__gcode_assert(__FILE__, __LINE__, __ASSERT_FUNC, #_thing);\
	return;\
}