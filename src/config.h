#pragma once

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

#define DEFAULT_UM_PER_STEP 100

// Rail offset to the edge of the probe area
#define RAIL_OFFSET_L 100000
#define RAIL_OFFSET_R 100000
#define PROBE_LEN 1500000
#define BED_HIGH_UM 100000