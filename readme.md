# Test framework

Yet another tiny unit testing framework for C.

## Features

- This framework follows the [xUnit](https://en.wikipedia.org/wiki/XUnit) unit testing style
    - Every test case is part of a suit, that can define a 'setup' and 'teardown' function. They are invoked before and after every test case respectively.
- Automatic test/suit registration
- Assertions: When an assertion fails, the affected test case fails
- Checks: When a check fails, execution continues and the failure gets printed
- Detailed logging of failed assertions/checks
- Detailed test results
- Simplicity. This library is very lightweight and should (hopefully) be easily extendable/hackable.

## TODO
[ ] Colored output
[ ] Logging to a file/custom stream, instead of stdout/stderr
[ ] Fully implement the possibility to skip individual tests/suits
[ ] Timing related information
[ ] "What does a uint testing framework really need?" i.e. the percentage of partially successful tests is not that interesting?

## Building

The library currently only supports linux, but adding support for other platforms should be relatively easy.
Just run '''make''' to build. This will create a static library under the '''build''' directory, and execute and any
tests.
Build and linking example:

``` bash
make
clang -Iinclude main.c build/libtest.a -o run_tests
```

## Getting started

``` c
#include <test/test.h>

void example_setup(void) {
    puts("I am executed before every test of the suit 'example_suit'!");
}

void example_teardown(void) {
    puts("I run after every test!");
}

SUIT(example_suit, example_setup, example_teardown);

TEST(example_suit, test_one) {
    char *some_string = "Hello";
    assert_eq("Hello", some_string);
}

TEST_MAIN
```

## Resouces
- https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
- https://stackoverflow.com/questions/4152018/initialize-global-array-of-function-pointers-at-either-compile-time-or-run-time/4152185#4152185
- https://gcc.gnu.org/onlinedocs/gccint/Initialization.html

Existing frameworks that inspired and helped me:
- https://github.com/jasmcaus/tau
- https://github.com/Snaipe/Criterion
- https://github.com/libcheck/check
