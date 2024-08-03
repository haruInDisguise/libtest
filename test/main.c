#ifndef TEST_IMPLEMENTATION
#define TEST_IMPLEMENTATION
#endif
#include <test/test.h>

#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if(test_init(argc, argv) == false) {
        exit(1);
    };

    test_run_all();

    return 0;
}
