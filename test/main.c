#include <stdint.h>
#include <stdio.h>

#include "../src/test.h"

int main(void) {
    test_init();

    test_run_all();

    return 0;
}
