#pragma once

#include <stdio.h>

// Reference an instruction variable based on fname
#define INSTR(fname) instr_ ## fname
// Define an instruction variable based on fname
#define DEF_INSTR(fname) \
	int fname (FILE* f);\
	Instruction INSTR(fname) = {.callback = fname, .id = #fname}; \
	int fname (FILE* f)


typedef int (*InstructionCallback)(FILE*);

typedef struct Instruction {
	InstructionCallback callback;
	char id[];
} Instruction;

extern Instruction *instructions[];

int execute_instruction(FILE* f);

#define FETCH_INSTR(F, S) fscanf(F, "!%s:", S)