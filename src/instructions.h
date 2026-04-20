#pragma once

#include <stdio.h>

// Reference an instruction variable based on fname
#define INSTR(fname) instr_ ## fname
// Define an instruction variable based on fname
#define DEF_INSTR(fname) \
	int fname (FILE* f);\
	const Instruction INSTR(fname) = {.callback = fname, .id = #fname}; \
	int fname (FILE* f)


typedef int (*InstructionCallback)(FILE*);

typedef struct Instruction {
	const InstructionCallback callback;
	const char id[];
} Instruction;

// Can we get anymore const?
extern const Instruction *const instructions[];

int execute_instruction(FILE* f);

#define FETCH_INSTR(F, S) fscanf(F, "!%s:", S)