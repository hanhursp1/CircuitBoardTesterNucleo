#include "gcodes.h"
#include "stepper.h"
#include <stdio.h>

extern Stepper railLeft;
extern Stepper railRight;

Gcode gcode_decode(FILE *stream) {
	Gcode result;
	int arg_idx = 0;
	char c = fgetc(stream);
	// While the stream isn't null
	while (c != '\0') {
		// Ignore whitespace
		while (c == ' ') c = fgetc(stream);
		// If we get a newline, break!
		if (c == '\n') break;
		// If we exceed the maximum amount of args, then break early and
		// maybe do some error handling.
		if (arg_idx >= GCODE_MAX_ARGS) {
			break;
		}
		// Store the id and value in the current index.
		result.args[arg_idx].id = c;
		fscanf(stream, "%d", &result.args[arg_idx].value);
		// Get the next character
		c = fgetc(stream);
		arg_idx++;
	}
	result.num_args = arg_idx;
	return result;
}