#include "instructions.h"
#include <string.h>

void can_you_repeat_that(FILE *f) { fprintf(f, "!Err:BadData;"); }

DEF_INSTR(vertcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d:%x;", &cnt, &hash);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  } else if (found == 2) {
    // TODO: Check hash
  }
	// TODO: set vert count info
  return 0;
}

DEF_INSTR(netcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d:%x;", &cnt, &hash);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  } else if (found == 2) {
    // TODO: Check hash
  }
	// TODO set net count info
  return 0;
}

DEF_INSTR(vert) {
  int id, x, y, hash;
  int found = fscanf(f, "%04d:%d,%d:%x;", &id, &x, &y, &hash);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
	// TODO: Set vertex info
	return 0;
}

DEF_INSTR(net) {
	int id, x, y, hash;
  int found = fscanf(f, "%04d:%d,%d:%x;", &id, &x, &y, &hash);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
	// TODO: Set net info
	return 0;
}

DEF_INSTR(echo) {
	// TODO: Print out all vertex and index info to the client
	return 0;
}

DEF_INSTR(run) {
	// TODO: Actually start a run
	return 0;
}

Instruction *instructions[] = {&INSTR(vertcnt), &INSTR(netcnt), &INSTR(vert),
                               &INSTR(net), &INSTR(echo)};

#define INSTR_COUNT (sizeof(instructions) / sizeof(Instruction *))

int execute_instruction(FILE *f) {
  char instr[128];

  if (FETCH_INSTR(f, instr) < 1) {
    // Failed to fetch an instruction, do something
    return -1;
  }
  for (int i = 0; i < INSTR_COUNT; i++) {
    // Assume an instruction is valid if its callback is not null
    // If the ID matches, execute the instruction, passing the file stream
    if (instructions[i]->callback && !strcmp(instructions[i]->id, instr)) {
      return instructions[i]->callback(f);
    }
  }
  can_you_repeat_that(f);
  return -1;
}