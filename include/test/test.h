#ifndef _TEST_H_
#define _TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(TEST_DEBUG)

#if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
#define TEST_BUILTIN_TRAP __builtin_debugtrap()
#else
#define TEST_BUILTIN_TRAP asm("int3")
#endif

#define test_intern_assert(condition)                                                       \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            printf("Assertion failed: [%s %s():%d] \"%s\"\n", __FILE__, __func__, __LINE__, \
                   #condition);                                                             \
            TEST_BUILTIN_TRAP;                                                              \
        }                                                                                   \
    } while (0)
#else
#define test_intern_assert(condition)
#endif // TEST_DEBUG

// TODO: Add manual-registration fallback
#if defined(__clang__) || defined(__gcc__)

#define _TEST_SECTION(name) __attribute__((section(name)))
#define TEST_UNUSED __attribute__((unused))

#else
#error "Compiler not supported :^("
#endif

/// The maximum amount of bytes that will be printed in assertion messages
#define TEST_VALUE_BUFFER_SIZE 32

/// The maximum length of any single message generated by this library
#define TEST_LOG_BUFFER_SIZE 1024

/// Used internally. The result of a single test case
typedef enum {
    test_intern_ResultOk,
    test_intern_ResultPartiallyOk,
    test_intern_ResultSkipped,
    test_intern_ResultFailed,
} test_intern_Result;

/// Used internally. Represents 'state' of the testing process.
typedef enum {
    test_intern_StatusEmpty,
    test_intern_StatusPending,
    test_intern_StatusResult,
    test_intern_StatusStart,
    test_intern_StatusEnd,
    test_intern_StatusContinued,
} test_intern_Status;

typedef struct {
    struct {
        uint32_t line;
        char value_one_buffer[TEST_VALUE_BUFFER_SIZE];
        char value_two_buffer[TEST_VALUE_BUFFER_SIZE];
        char *macro_value_one;
        char *macro_value_two;
        char *macro_name;
    } assertion_info;

    test_intern_Result result;
} test_intern_TestInfo;

/// This  type is used by the automatic test registration process
typedef void (*test_intern_TestFunction)(test_intern_TestInfo *_test_info);
/// A test suit's setup function. Called before every test.
typedef void (*test_SetupFunction)(void);
/// A test suit's teardown function. Called after every test.
typedef void (*test_TeardownFunction)(void);

typedef struct {
    uint32_t line;
    char *name;
    char *suit_name;
    char *file_name;
    test_intern_TestFunction function;
} test_intern_TestCase;

typedef struct {
    char *name;
    test_intern_TestCase **tests;
    uint32_t total_tests;
    test_SetupFunction setup_function;
    test_TeardownFunction teardown_function;
} test_intern_Suit;

typedef struct {
    bool output_is_colored;
    bool verbose;

    FILE *output_stream;
    char *output_stream_path;

    char **target_suits;
    char **target_cases;
} test_Options;

extern bool test_init(int argc, char **argv);
extern void test_exit(void);

/// Log things to the stdout stream (stdout by default)
/// @param fmt printf style format string
/// @param ... arguments for printf style format string
extern void test_log(const char *fmt, ...);

/// Log things to the stderr stram (stderr by default)
/// @param fmt printf style format string
/// @param ... arguments for printf style format string
extern void test_log_error(const char *fmt, ...);

/// Used internally. Report the failure of an assert macro.
extern void test_intern_log_assertion_failed(const test_intern_TestInfo *info);

/// Reset internal statistics (total time, tests executed etc.). This will
/// NOT remove registered tests/suits.
extern void test_reset(void);

/// Generate status report.
extern void test_report(void);

/// Run all the tests belonging suit "name"
extern void test_run_suit(const char *name);

/// Run a single test case
/// @param suit_name The name of test cases suit
/// @param case_name The name of the test case
extern void test_run_case(const char *suit_name, const char *case_name);

/// Start executing all suits and there tests
extern void test_run_all(void);

/// Macro for defining a new suit. Note that you have to define at least one
/// suit
#define SUIT(suit_name, setup, teardown)                                                 \
    const test_intern_Suit _TEST_SECTION("_test_suit_section") test_suit_##suit_name = { \
        .name = #suit_name,                                                              \
        .teardown_function = teardown,                                                   \
        .setup_function = setup,                                                         \
    }

/// Macro for creating a new test case.
/// @param suit_name The name of the suit this test case should be added to.
/// @param test_name The name of this test case
#define TEST(suit_name_, test_name)                                                          \
    static void test_##suit_name_##_##test_name(test_intern_TestInfo *_info);                \
    const test_intern_TestCase _TEST_SECTION("_test_case_section")                           \
        test_case_##test_name##_##suit_name = {.name = #test_name,                           \
                                               .line = __LINE__,                             \
                                               .file_name = __FILE__,                        \
                                               .suit_name = #suit_name_,                     \
                                               .function = test_##suit_name_##_##test_name}; \
    static void test_##suit_name_##_##test_name(TEST_UNUSED test_intern_TestInfo *_test_info)

#define TEST_MAIN       \
    int main(int argc, char **argv) {    \
        test_init(argc, argv);    \
        test_run_all(); \
        test_exit();    \
        return 0;       \
    }

// FIXME: Figure out a prettier solution to determin the type of a variable.
#define _TEST_GET_TYPE_FMT(X)      \
    _Generic((X),                  \
        char: "%c",                \
        unsigned char: "%uz",      \
        short: "%h",               \
        unsigned short: "%uh",     \
        long: "%z",                \
        unsigned long: "%uz",      \
        int: "%d",                 \
        unsigned int: "%u",        \
        long long: "%z",           \
        unsigned long long: "%zu", \
        float: "%f",               \
        double: "%f",              \
        char *: "%s",              \
        default: "%p")

// Helper macro for writing a string to <buffer>
#define _TEST_WRITE_STRING(buffer, size, format, value) snprintf(buffer, size, format, value)

// Helper macro for resolving the type of <value> and writing it to <buffer>
#define _TEST_WRITE_VALUE_TO_BUFFER(buffer, size, value) \
    _TEST_WRITE_STRING(buffer, size, _TEST_GET_TYPE_FMT(value), value)

// Part used in CHECK assertion macros
#define _TEST_ACTION_CHECK                                  \
    do {                                                    \
        _test_info->result = test_intern_ResultPartiallyOk; \
        test_intern_log_assertion_failed(_test_info);       \
        continue;                                           \
    } while (0)

// Part used in ASSERT assertion macros
#define _TEST_ACTION_ASSERT                            \
    do {                                               \
        _test_info->result = test_intern_ResultFailed; \
        test_intern_log_assertion_failed(_test_info);  \
        return;                                        \
    } while (0)

// Main Assertion Macro
#define _TEST_CMP(one, two, macro, CMP_FUNC, ASSERT_TYPE_ACTION)                     \
    do {                                                                             \
        if (!CMP_FUNC(one, two)) {                                                   \
            _test_info->assertion_info.line = __LINE__;                              \
            _test_info->assertion_info.macro_name = #macro;                          \
            _test_info->assertion_info.macro_value_one = #one;                       \
            _test_info->assertion_info.macro_value_one = #two;                       \
            _TEST_WRITE_VALUE_TO_BUFFER(_test_info->assertion_info.value_one_buffer, \
                                        TEST_VALUE_BUFFER_SIZE, one);                \
            _TEST_WRITE_VALUE_TO_BUFFER(_test_info->assertion_info.value_two_buffer, \
                                        TEST_VALUE_BUFFER_SIZE, two);                \
            ASSERT_TYPE_ACTION;                                                      \
        }                                                                            \
    } while (0)

// clang-format off
// Comparison 'functions'
#define _TEST_CMP_EQ(one, two) ((one) == (two))
#define _TEST_CMP_NE(one, two) ((one) != (two))

#define _TEST_CMP_GT(one, two) ((one) > (two))
#define _TEST_CMP_LT(one, two) ((one) < (two))
#define _TEST_CMP_GE(one, two) ((one) >= (two))
#define _TEST_CMP_LE(one, two) ((one) <= (two))

#define _TEST_CMP_STR_EQ(one, two) (strcmp(one, two) == 0)
#define _TEST_CMP_STR_NE(one, two) (strcmp(one, two) != 0)

#define _TEST_CMP_MEM_EQ(one, two, size) (memcmp(one, two, size) == 0)
#define _TEST_CMP_MEM_NE(one, two, size) (memcmp(one, two, size) != 0)

// Assert Macros
#define test_assert(one)                 _TEST_CMP(one, 1, assert, _TEST_CMP_EQ, _TEST_ACTION_ASSERT)
#define test_assert_eq(one, two)         _TEST_CMP(one, two, assert_eq, _TEST_CMP_EQ, _TEST_ACTION_ASSERT)
#define test_assert_ne(one, two)         _TEST_CMP(one, two, assert_ne, _TEST_CMP_NE, _TEST_ACTION_ASSERT)

#define test_assert_string_eq(one, two)         _TEST_CMP(one, two, assert_str_eq, _TEST_CMP_STR_EQ, _TEST_ACTION_ASSERT)
#define test_assert_string_ne(one, two)         _TEST_CMP(one, two, assert_str_ne,_TEST_CMP_STR_NE, _TEST_ACTION_ASSERT)
#define test_assert_memory_eq(one, two, size)   _TEST_CMP(one, two, assert_str_eq, _TEST_CMP_STR_EQ, _TEST_ACTION_ASSERT)
#define test_assert_memory_ne(one, two, size)   _TEST_CMP(one, two, assert_str_ne,_TEST_CMP_STR_NE, _TEST_ACTION_ASSERT)

// Check Macros
#define test_check_string_eq(one, two)     _TEST_CMP(one, two, check_str_eq,_TEST_CMP_STR_EQ, _TEST_ACTION_CHECK)
#define test_check_string_ne(one, two)     _TEST_CMP(one, two, check_str_ne,_TEST_CMP_STR_NE, _TEST_ACTION_CHECK)

#define test_check(one)                 _TEST_CMP(one, 1, check_eq,_TEST_CMP_EQ, _TEST_ACTION_CHECK)
#define test_check_eq(one, two)         _TEST_CMP(one, two, check_eq,_TEST_CMP_EQ, _TEST_ACTION_CHECK)
#define test_check_ne(one, two)         _TEST_CMP(one, two, check_ne,_TEST_CMP_NE, _TEST_ACTION_CHECK)

// clang-format on

#endif
