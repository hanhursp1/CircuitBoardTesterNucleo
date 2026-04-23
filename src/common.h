#pragma once

#include "config.h"

#define PI 3.14159265359

typedef enum bool {
	false, true
} bool;

typedef enum Side {
	Left, Right
} Side;

// Custom assert that simply returns from the file
void __assert_return(const char * filename, int line, const char * funcname, const char * what_broke);

// Yet another assert function. I swear these will be useful someday
#define assert_return(_condition, ...) if (!(_condition)) {\
	__assert_return(__FILE__, __LINE__, __ASSERT_FUNC, #_condition);\
	return __VA_ARGS__;\
}