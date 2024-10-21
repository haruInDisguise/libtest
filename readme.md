# Test framework

Yet another tiny unit testing framework for C.

## Features

- [xUnit](https://en.wikipedia.org/wiki/XUnit) unit testing style
    - Every test case is part of a suit, that can define a 'setup' and 'teardown' function.
- Header only, [STB style](https://github.com/nothings/stb) library
- Automatic test/suit registration
- Filter tests/suits using basic glob patterns
- Lightweight and should (hopefully) be easily extendable/hackable.

Planned features:

- Timing related information

## Usage

Since this is an STB style library, meaning you can just include the test header directly.
The ```TEST_IMPLEMENATION``` macro must only be defined in a single source file.

At least one suit and test case must be defined. Failure to do so will result in a linker error.

A usage pattern could look like the following:

```c
/* file: main.c */
#define TEST_IMPLEMENTATION
#include <test/test.h>

int main(int argc, char **argv) {
    if(!test_setup(argc, argv)) {
        return false;
    }

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

To compile this is example, you would write:
```
cc -Iinclude/test main.c test_stuff.c -o <binary_name>
```

The resulting binary accepts the following options:
```
Usage
      <test_binary> [OPTIONS] ... -- [ARGS]

ARGS
      Arguments that will be ignored by this library

OPTIONS
      The binary build with this library accepts the following options:

      --help
        Display this message

      --list
        Print all registered test cases. Respectes filters.

      --filter <filters>
        A list of filter patterns to selectively execute tests/suits.
        Patterns can contain '*' (matches zero or more characters),
        '?' (matches a single character) or the following characters:
            A-Z, a-z, 0-9, ':', '_'
        To specify multiple patterns, separated them by commas.

      --output <file>
        Redirect library output to a new file.

      --colored (auto|always|never)
        Colorize the output.
```

## Resouces
- https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
- https://stackoverflow.com/questions/4152018/initialize-global-array-of-function-pointers-at-either-compile-time-or-run-time/4152185#4152185
- https://gcc.gnu.org/onlinedocs/gccint/Initialization.html

Existing frameworks that inspired and helped me:
- https://github.com/jasmcaus/tau
- https://github.com/Snaipe/Criterion
- https://github.com/libcheck/check
