#pragma once

#define DEBUG_ENABLED

// Maximum number of points in the netlist buffer
#define MAX_POINTS 4096

#define MIN_NET_SIZE 2
#define MAX_NET_SIZE 2048

// Maximum number of nets in the netlist buffer
#define MAX_INDICES (MAX_POINTS / MIN_NET_SIZE)

// Maximum size of the stepper registry
#define MAX_STEPPERS 4

// USART Echo Mode (echo all input recieved)
// #define ECHO_MODE

// Maximum number of concurrent processes
#define PROCESSES 16

// #define USE_IRQ

#define DEFAULT_UM_PER_STEP 50

// Rail offset to the edge of the probe area
// TODO: Actually measure these
#define RAIL_OFFSET 80000
#define RAIL_OFFSET_L (RAIL_OFFSET - 10000)
#define RAIL_OFFSET_R (RAIL_OFFSET + 10000)
#define RAIL_OFFSET 10000
#define PROBE_LEN 181250
#define BED_HIGH_UM 10000
#define BED_OFFSET_Y 150000
#define BED_WIDTH (PROBE_LEN - RAIL_OFFSET)
#define RAIL_LEN 306000