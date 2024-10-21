# Test framework

Yet another tiny unit testing framework for C.

## Features

- This framework follows the [xUnit](https://en.wikipedia.org/wiki/XUnit) unit testing style
    - Every test case is part of a suit, that can define a 'setup' and 'teardown' function. They are invoked before and after every test case respectively.
- Header only, STB style library
- Automatic test/suit registration
- Useful runtime flags: Colored output, filters, logging to a file, ...
- Simplicity. This library is lightweight and should (hopefully) be easily extendable/hackable.

## TODO
[ ] Timing related information

## Usage

Since this is an STB style, header only library, you can just include the test header directly.
Note that the ```TEST_IMPLEMENATION``` macro must only be defined inside the source file that contains your
projects entry point.
For example:

```c
/* file: main.c */
#define TEST_IMPLEMENTATION
#include <test/test.h>

int main(int argc, char **argv) {
    test_setup(argc, argv);
    test_run_all();
    test_exit();
}

/* file: test_stuff.c */
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
```

## Resouces
- https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
- https://stackoverflow.com/questions/4152018/initialize-global-array-of-function-pointers-at-either-compile-time-or-run-time/4152185#4152185
- https://gcc.gnu.org/onlinedocs/gccint/Initialization.html

Existing frameworks that inspired and helped me:
- https://github.com/jasmcaus/tau
- https://github.com/Snaipe/Criterion
- https://github.com/libcheck/check
