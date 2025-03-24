#include <stdlib.h>
#include <inttypes.h>

#include "test.h"
/*-------------------------------------------------------------------------------------------------
 *
 *                                       Main - Run the tests
 *
 *-----------------------------------------------------------------------------------------------*/
int
main(int argc, char *argv[])
{
    fprintf(stderr, "\n\n***        Starting Tests.        ***\n\n");

    fprintf(stderr, "magpie_sys_memory_tests()..");
    magpie_sys_memory_tests();
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "\n\n*** Tests completed successfully. ***\n\n");

    return EXIT_SUCCESS;
}

#include "sys_memory.c"

