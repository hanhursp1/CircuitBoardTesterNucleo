#include "gcodes.h"
#include "stepper.h"
#include <stdio.h>

extern Stepper railLeft;
extern Stepper railRight;

Gcode gcode_decode(FILE *stream) {
	Gcode result;
	int arg_idx = 0;
	char c = fgetc(stream);
	while (c != '\n') {
		if (c == ' ') {
			// Increment the arg
			arg_idx++;
			c = fgetc(stream);
			continue;
		}
		// If
		if (arg_idx >= GCODE_MAX_ARGS) {
			break;
		}
		// Store the id and value in the current index.
		result.args[arg_idx].id = c;
		fscanf(stream, "%d", &result.args[arg_idx].value);
		// Get the next character
		c = fgetc(stream);
	}
	result.num_args = arg_idx;
	return result;
}