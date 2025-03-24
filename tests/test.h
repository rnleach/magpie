#ifndef _MAGPIE_TESTS_
#define _MAGPIE_TESTS_
//
// For testing, ensure we have some debugging tools activated.
//

// We must have asserts working for the tests to work.
#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <stdio.h>
#include "../build/magpie.h"

void magpie_sys_memory_tests(void);

#endif
