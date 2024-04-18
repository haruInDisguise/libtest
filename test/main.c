#ifndef YAK_TEST_IMPLEMENTATION
#define YAK_TEST_IMPLEMENTATION
#endif
#include <test/test.h>

#include <stdlib.h>

int main(int argc, char **argv) {
    if(yak_test_init(argc, argv) == false) {
        exit(1);
    };

    yak_test_run_all();

    return 0;
}
