#pragma once

// Maximum number of points in the netlist buffer
#define MAX_POINTS 4096

#define MIN_NET_SIZE 2
#define MAX_NET_SIZE 2048

// Maximum number of nets in the netlist buffer
#define MAX_INDICES (MAX_POINTS / MIN_NET_SIZE)

// Maximum size of the stepper registry
#define MAX_STEPPERS 4