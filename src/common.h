#pragma once

typedef enum bool {
	false, true
} bool;

// Custom assert that simply returns from the file
void __assert_return(const char * filename, int line, const char * funcname, const char * what_broke);

// Yet another assert function. I swear these will be useful someday
#define assert_return(_condition, ...) if (!(_condition)) {\
	__assert_return(__FILE__, __LINE__, __ASSERT_FUNC, #_condition);\
	return __VA_ARGS__;\
}