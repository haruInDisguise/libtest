#ifndef _YAK_TEST_H_
#define _YAK_TEST_H_

/// # ```yak_test``` - yet another unit testing framework
///
/// This library implements a small [stb style](https://github.com/nothings/stb) unit testing
/// framework that tries to be small and easy to use and/or hack.
///
/// ## Features
///
/// - Lightweight and easy to use (hopefully)
/// - [xUnit](https://en.wikipedia.org/wiki/XUnit) inspired framework
/// - Automatic test registration

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef YAK_TEST_DEBUG

    #if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
        #define YAK_TEST_BUILTIN_TRAP __builtin_debugtrap()
    #else
        #define YAK_TEST_BUILTIN_TRAP asm("int3")
    #endif

    #define yak_test_intern_assert(condition)                                                \
        do {                                                                                 \
            if (!(condition)) {                                                              \
                printf(                                                                      \
                    "Assertion failed: [%s %s():%d] \"%s\"\n", __FILE__, __func__, __LINE__, \
                    #condition                                                               \
                );                                                                           \
                YAK_TEST_BUILTIN_TRAP;                                                       \
            }                                                                                \
        } while (0)
#else
    #define yak_test_intern_assert(condition)
#endif // YAK_TEST_DEBUG

// TODO: Add manual-registration fallback
#if defined(__clang__) || defined(__gcc__)

    #define _YAK_TEST_SECTION(name) __attribute__((section(name)))
    #define YAK_TEST_UNUSED         __attribute__((unused))

#else
    #error "Compiler not supported :^("
#endif

/// The maximum amount of bytes that will be printed in assertion messages
#define YAK_TEST_VALUE_BUFFER_SIZE 32

/// The maximum length of any single message generated by this library
#define YAK_TEST_LOG_BUFFER_SIZE 1024

/// Used internally. The result of a single test case
typedef enum {
    yak_test_intern_ResultOk,
    yak_test_intern_ResultPartiallyOk,
    yak_test_intern_ResultSkipped,
    yak_test_intern_ResultFailed,
} yak_test_intern_Result;

/// Used internally. Represents 'state' of the testing process.
typedef enum {
    yak_test_intern_StatusEmpty,
    yak_test_intern_StatusPending,
    yak_test_intern_StatusResult,
    yak_test_intern_StatusStart,
    yak_test_intern_StatusEnd,
    yak_test_intern_StatusContinued,
} yak_test_intern_Status;

typedef struct {
    struct {
        uint32_t line;
        char value_one_buffer[YAK_TEST_VALUE_BUFFER_SIZE];
        char value_two_buffer[YAK_TEST_VALUE_BUFFER_SIZE];
        char *macro_value_one;
        char *macro_value_two;
        char *macro_name;
    } assertion_info;

    bool is_verbose;
    yak_test_intern_Result result;
} yak_test_intern_TestInfo;

/// This  type is used by the automatic test registration process
typedef void (*yak_test_intern_TestFunction)(yak_test_intern_TestInfo *_yak_test_info);
/// A test suit's setup function. Called before every test.
typedef void (*yak_test_SetupFunction)(void);
/// A test suit's teardown function. Called after every test.
typedef void (*yak_test_TeardownFunction)(void);

typedef struct {
    uint32_t line;
    char *name;
    char *suit_name;
    char *file_name;
    yak_test_intern_TestFunction function;
} yak_test_intern_TestCase;

typedef struct {
    char *name;
    yak_test_intern_TestCase **tests;
    uint32_t total_tests;
    yak_test_SetupFunction setup_function;
    yak_test_TeardownFunction teardown_function;
} yak_test_intern_Suit;

extern bool yak_test_init(int argc, char **argv);
extern void yak_test_exit(void);

/// Used internally. Report the failure of an assert macro.
extern void yak_test_intern_log_assertion_failed(const yak_test_intern_TestInfo *info);

/// Reset internal statistics (total time, tests executed etc.). This will
/// NOT remove registered tests/suits.
extern void yak_test_reset(void);

/// Generate status report.
extern void yak_test_report(void);

/// Manually register a suit case
/// TODO: There is no way to optain an instance of yak_test_intern_Suit, i.e.
/// it is impossible to correctly call this function.
extern bool yak_test_register_suit(
    yak_test_intern_Suit *yak_test_suit, yak_test_SetupFunction setup_func,
    yak_test_TeardownFunction teardown_func
);

/// Manually register a test case
extern bool yak_test_register_case(
    yak_test_intern_Suit *yak_test_suit, yak_test_intern_TestCase *yak_test_case
);

/// Run all the tests belonging to suit `name`
extern bool yak_test_run_suit(const char *name);

/// Execute a single test case
extern bool yak_test_run_case(const char *suit_name, const char *case_name);

/// Start executing all suits and there tests
extern void yak_test_run_all(void);

/// Get option: Output file
extern const char *yak_test_option_get_output_path(void);

/// Get option: Filter
extern const char *yak_test_option_get_filter(void);

/// Macro for defining a new suit. Note that you have to define at least one
/// suit
#define SUIT(suit_name, setup, teardown)                                   \
    const yak_test_intern_Suit _YAK_TEST_SECTION("_yak_test_suit_section") \
        yak_test_suit_##suit_name = {                                      \
            .name = #suit_name,                                            \
            .teardown_function = teardown,                                 \
            .setup_function = setup,                                       \
        }

/// Macro for creating a new test case.
/// @param suit_name The name of the suit this test case should be added to.
/// @param yak_test_name The name of this test case
#define TEST(suit_name_, yak_test_name)                                                   \
    static void yak_test_##suit_name_##_##yak_test_name(yak_test_intern_TestInfo *_info); \
    const yak_test_intern_TestCase _YAK_TEST_SECTION("_yak_test_case_section")            \
        yak_test_case_##yak_test_name##_##suit_name = {                                   \
            .name = #yak_test_name,                                                       \
            .line = __LINE__,                                                             \
            .file_name = __FILE__,                                                        \
            .suit_name = #suit_name_,                                                     \
            .function = yak_test_##suit_name_##_##yak_test_name                           \
        };                                                                                \
    static void yak_test_##suit_name_##_##yak_test_name(                                  \
        YAK_TEST_UNUSED yak_test_intern_TestInfo *_yak_test_info                          \
    )

#define YAK_TEST_MAIN                 \
    int main(int argc, char **argv) { \
        yak_test_init(argc, argv);    \
        yak_test_run_all();           \
        yak_test_exit();              \
        return 0;                     \
    }

// FIXME: I am really not happy about the generic aspect here. A type mismatch produces a
// (in this context) somewhat akward warning message (invalid printf format string).
// It might be better to move to an explicit construct (i.e. a macro for each primitive type).
#define _YAK_TEST_GET_TYPE_FMT(X)  \
    _Generic(                      \
        (X),                       \
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
        default: "%p"              \
    )

#define _YAK_TEST_WRITE_VALUE_TO_BUFFER(buffer, size, value) \
    snprintf(buffer, size, _YAK_TEST_GET_TYPE_FMT(value), value)

#define _YAK_TEST_CMP(one, two, macro, CMP_FUNC)                                                 \
    do {                                                                                         \
        if (!CMP_FUNC(one, two)) {                                                               \
            _yak_test_info->assertion_info.line = __LINE__;                                      \
            _yak_test_info->assertion_info.macro_name = #macro;                                  \
            _yak_test_info->assertion_info.macro_value_one = #one;                               \
            _yak_test_info->assertion_info.macro_value_one = #two;                               \
            _YAK_TEST_WRITE_VALUE_TO_BUFFER(                                                     \
                _yak_test_info->assertion_info.value_one_buffer, YAK_TEST_VALUE_BUFFER_SIZE, one \
            );                                                                                   \
            _YAK_TEST_WRITE_VALUE_TO_BUFFER(                                                     \
                _yak_test_info->assertion_info.value_two_buffer, YAK_TEST_VALUE_BUFFER_SIZE, two \
            );                                                                                   \
            _yak_test_info->result = yak_test_intern_ResultFailed;                               \
            yak_test_intern_log_assertion_failed(_yak_test_info);                                \
        }                                                                                        \
    } while (0)

// clang-format off
#define _YAK_TEST_CMP_EQ(one, two) ((one) == (two))
#define _YAK_TEST_CMP_NE(one, two) ((one) != (two))

#define _YAK_TEST_CMP_GT(one, two) ((one) > (two))
#define _YAK_TEST_CMP_LT(one, two) ((one) < (two))
#define _YAK_TEST_CMP_GE(one, two) ((one) >= (two))
#define _YAK_TEST_CMP_LE(one, two) ((one) <= (two))

#define _YAK_TEST_CMP_STR_EQ(one, two) (strcmp(one, two) == 0)
#define _YAK_TEST_CMP_STR_NE(one, two) (strcmp(one, two) != 0)

#define _YAK_TEST_CMP_MEM_EQ(one, two, size) (memcmp(one, two, size) == 0)
#define _YAK_TEST_CMP_MEM_NE(one, two, size) (memcmp(one, two, size) != 0)

// Assert Macros
#define yak_test_assert(one)                 _YAK_TEST_CMP(one, 1, assert, _YAK_TEST_CMP_EQ)
#define yak_test_assert_eq(one, two)         _YAK_TEST_CMP(one, two, assert_eq, _YAK_TEST_CMP_EQ)
#define yak_test_assert_ne(one, two)         _YAK_TEST_CMP(one, two, assert_ne, _YAK_TEST_CMP_NE)

#define yak_test_assert_string_eq(one, two)         _YAK_TEST_CMP(one, two, assert_str_eq, _YAK_TEST_CMP_STR_EQ)
#define yak_test_assert_string_ne(one, two)         _YAK_TEST_CMP(one, two, assert_str_ne,_YAK_TEST_CMP_STR_NE)
#define yak_test_assert_memory_eq(one, two, size)   _YAK_TEST_CMP(one, two, assert_str_eq, _YAK_TEST_CMP_STR_EQ)
#define yak_test_assert_memory_ne(one, two, size)   _YAK_TEST_CMP(one, two, assert_str_ne,_YAK_TEST_CMP_STR_NE)
// clang-format on

#endif

// Implementation start: src/test.c
#ifdef YAK_TEST_IMPLEMENTATION

#include <errno.h> /* errno */
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h> /* abort, exit */
#include <malloc.h> /* malloc, calloc */
#include <string.h> /* sterror, strcmp, memset */
#include <unistd.h> /* isatty */

#if defined(__clang__) || defined(__gcc__)
// see:
// https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
extern yak_test_intern_TestCase __start__yak_test_case_section;
extern yak_test_intern_TestCase __stop__yak_test_case_section;
    #define _YAK_TEST_START_CASE_SECTION &__start__yak_test_case_section
    #define _YAK_TEST_STOP_CASE_SECTION  &__stop__yak_test_case_section

extern yak_test_intern_Suit __start__yak_test_suit_section;
extern yak_test_intern_Suit __stop__yak_test_suit_section;
    #define _YAK_TEST_START_SUIT_SECTION &__start__yak_test_suit_section
    #define _YAK_TEST_STOP_SUIT_SECTION  &__stop__yak_test_suit_section
#else
    #error "Compiler not supported :^("
#endif

#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_DEFAULT "\033[38m"

static struct {
    uint32_t total_suits;
    uint32_t total_tests;
    yak_test_intern_Suit **suits;
} yak_test_register = { 0 };

static struct {
    uint32_t tests_executed;
    uint32_t tests_successful;
    uint32_t tests_failed;
    uint32_t tests_partially;
    uint32_t tests_skipped;
    uint32_t suits_executed;
    uint32_t suits_failed;
    uint32_t suits_skipped;
} yak_test_runner = { 0 };

static struct {
    uint32_t length;
    uint32_t offset;
    char *buffer;
} log_data = { 0 };

static struct {
    bool colored;
    bool verbose;
    bool print_list;

    // Argument flag values as they are being read from the commandline
    // before any parsing takes place
    struct {
        char *colored_value;
        char *output_value;
        char *filter_value;
    } raw;

    FILE *output_stream;

    char **target_suits;
    char **target_cases;
} options = { 0 };

static inline void *yak_test_realloc(void *base, uintptr_t size) {
    void *result = realloc(base, size);
    if (result == NULL) {
        perror("test: ");
        abort();
    }

    return result;
}

static inline void *yak_test_calloc(uint64_t nmem, uint64_t smem) {
    void *result = calloc(nmem, smem);
    if (result == NULL) {
        perror("test: ");
        abort();
    }

    return result;
}

static inline void yak_test_log_init(void) {
    log_data.offset = 0;
    log_data.length = YAK_TEST_LOG_BUFFER_SIZE;
    log_data.buffer = yak_test_calloc(1, YAK_TEST_LOG_BUFFER_SIZE * sizeof(*log_data.buffer));
}

static inline void yak_test_log_free(void) { free(log_data.buffer); }

static inline void yak_test_log_clear(void) {
    log_data.offset = 0;
    // memset(log_data.buffer, 0, log_data.length * sizeof (*log_data.buffer));
}

static void yak_test_log_write(const char *format, va_list *args) {
    yak_test_intern_assert(args != NULL);
    yak_test_intern_assert(format != NULL);
    yak_test_intern_assert(log_data.length - log_data.offset > 0);

    vsnprintf(log_data.buffer + log_data.offset, log_data.length - log_data.offset, format, *args);
    fprintf(options.output_stream, "%s", log_data.buffer);
    yak_test_log_clear();
}

__attribute__((format(printf, 2, 3))) static void
yak_test_log_write_with_status(const yak_test_intern_Status status, const char *format, ...) {
    yak_test_intern_assert(format != NULL);

    static const char *status_strings_blank[] = {
        [yak_test_intern_StatusPending] = "PENDING",   [yak_test_intern_StatusResult] = "RESULT ",
        [yak_test_intern_StatusStart] = "RUNNING",     [yak_test_intern_StatusEnd] = "REPORT ",
        [yak_test_intern_StatusContinued] = "-------",
    };
    // static const char *status_string_colored[] = {
    //     [yak_test_intern_StatusPending] = "PENDING" COLOR_RESET,
    //     [yak_test_intern_StatusResult]  = "RESULT " COLOR_RESET,
    //     [yak_test_intern_StatusStart]   = "RUNNING" COLOR_RESET,
    //     [yak_test_intern_StatusEnd]     = COLOR_CYAN    "REPORT " COLOR_RESET,
    //     [yak_test_intern_StatusContinued] = "-------",
    // };

    if (status == yak_test_intern_StatusEmpty) {
        log_data.offset = 0;
    } else {
        log_data.offset =
            snprintf(log_data.buffer, log_data.length, "[ %s ] ", status_strings_blank[status]);
    }

    va_list args;
    va_start(args, format);

    yak_test_log_write(format, &args);

    va_end(args);
}

/* Test runner */
static void yak_test_runner_report(void) {
    uint32_t tests_failed_percent =
        (yak_test_runner.tests_failed > 0)
            ? (yak_test_runner.tests_failed * 100 / yak_test_register.total_tests)
            : 0;
    uint32_t tests_skipped_percent =
        (yak_test_runner.tests_skipped > 0)
            ? (yak_test_runner.tests_skipped * 100 / yak_test_register.total_tests)
            : 0;

    uint32_t tests_successful_percent =
        (yak_test_runner.tests_successful) > 0
            ? (yak_test_runner.tests_successful * 100 / yak_test_runner.tests_executed)
            : 0;
    uint32_t tests_partial_percent =
        (yak_test_runner.tests_partially > 0)
            ? (yak_test_runner.tests_partially * 100 / yak_test_runner.tests_executed)
            : 0;
    uint32_t tests_executed_percent =
        (yak_test_runner.tests_executed > 0)
            ? (yak_test_runner.tests_executed * 100 / yak_test_register.total_tests)
            : 0;

    // clang-format off
    yak_test_log_write_with_status(yak_test_intern_StatusEnd,       "=== Testing Results ===\n");
    yak_test_log_write_with_status(yak_test_intern_StatusContinued, "Successful:   %u/%u (%u%%)\n", yak_test_runner.tests_executed, yak_test_register.total_tests, tests_executed_percent);
    yak_test_log_write_with_status(yak_test_intern_StatusContinued, "- Totally   : %u - %u%%\n", yak_test_runner.tests_successful, tests_successful_percent);
    yak_test_log_write_with_status(yak_test_intern_StatusContinued, "- Partially : %u - %u%%\n", yak_test_runner.tests_partially, tests_partial_percent);
    yak_test_log_write_with_status(yak_test_intern_StatusContinued, "Failed  : %u - %3u%%\n", yak_test_runner.tests_failed, tests_failed_percent);
    yak_test_log_write_with_status(yak_test_intern_StatusContinued, "Skipped : %u - %3u%%\n", yak_test_runner.tests_skipped, tests_skipped_percent);
    // clang-format on
}

static void
yak_test_runner_run_test(const yak_test_intern_TestCase *test, const yak_test_intern_Suit *suit) {
    yak_test_intern_assert(test != NULL);
    yak_test_intern_assert(suit != NULL);

    yak_test_intern_TestInfo info = { .result = yak_test_intern_ResultOk };

    yak_test_log_write_with_status(
        yak_test_intern_StatusStart, "%s => %s:%s\n", test->file_name, suit->name, test->name
    );

    if (suit->setup_function != NULL) {
        suit->setup_function();
    }

    test->function(&info);

    // yak_test_intern_assert(info.result <= yak_test_intern_ResultPartiallyOk);
    // yak_test_intern_assert(info.assertion_info.macro_value_one != NULL);
    // yak_test_intern_assert(info.assertion_info.macro_value_two != NULL);

    if (suit->teardown_function != NULL) {
        suit->teardown_function();
    }

    switch (info.result) {
    case yak_test_intern_ResultOk:
        yak_test_runner.tests_successful++;
        yak_test_runner.tests_executed++;
        break;
    case yak_test_intern_ResultPartiallyOk:
        yak_test_runner.tests_partially++;
        yak_test_runner.tests_executed++;
        break;
    case yak_test_intern_ResultSkipped:
        yak_test_runner.tests_skipped++;
        break;
    case yak_test_intern_ResultFailed:
        yak_test_runner.tests_failed++;
        break;
    }

    static const char *result_strings_colored[] = {
        [yak_test_intern_ResultOk] = COLOR_GREEN "OK" COLOR_RESET,
        [yak_test_intern_ResultPartiallyOk] = COLOR_YELLOW "PARTIALLY OK" COLOR_RESET,
        [yak_test_intern_ResultSkipped] = COLOR_CYAN "SKIPPED" COLOR_RESET,
        [yak_test_intern_ResultFailed] = COLOR_RED "FAILED" COLOR_RESET,
    };
    static const char *result_strings_blank[] = {
        [yak_test_intern_ResultOk] = "OK",
        [yak_test_intern_ResultPartiallyOk] = "PARTIALLY OK",
        [yak_test_intern_ResultSkipped] = "SKIPPED",
        [yak_test_intern_ResultFailed] = "FAILED",
    };

    const char **result_strings =
        options.colored ? result_strings_colored : result_strings_blank;
    yak_test_log_write_with_status(
        yak_test_intern_StatusResult, "%s\n", result_strings[info.result]
    );
}

static void yak_test_runner_run_suit(const yak_test_intern_Suit *suit) {
    yak_test_intern_assert(suit != NULL);

    for (uint32_t i = 0; i < suit->total_tests; i++) {
        yak_test_runner_run_test(suit->tests[i], suit);
    }

    yak_test_runner.suits_executed++;
}

static void yak_test_runner_reset(void) { memset(&yak_test_runner, 0, sizeof(yak_test_runner)); }

static yak_test_intern_Suit *yak_test_suit_find_by_name(const char *name) {
    yak_test_intern_assert(name != NULL);

    for (uint32_t i = 0; i < yak_test_register.total_suits; i++) {
        yak_test_intern_Suit *suit = yak_test_register.suits[i];

        if (strcmp(suit->name, name) == 0) {
            return suit;
        }
    }

    return NULL;
}

void yak_test_intern_log_assertion_failed(const yak_test_intern_TestInfo *yak_test_info) {
    yak_test_intern_assert(yak_test_info != NULL);

    const char *value_one = yak_test_info->assertion_info.value_one_buffer;
    const char *value_one_macro = yak_test_info->assertion_info.macro_value_one;
    const char *value_two = yak_test_info->assertion_info.value_two_buffer;
    const char *value_two_macro = yak_test_info->assertion_info.macro_value_two;
    const char *macro_name = yak_test_info->assertion_info.macro_name;
    const uint32_t line = yak_test_info->assertion_info.line;

    const char *message_prefix =
        yak_test_info->result == yak_test_intern_ResultFailed ? "assertion failed" : "check failed";

    yak_test_log_write_with_status(
        yak_test_intern_StatusPending, "%s (%u): \"%s\" != \"%s\"", message_prefix, line, value_one,
        value_two
    );
    yak_test_log_write_with_status(
        yak_test_intern_StatusEmpty, " -> %s(%s, %s)\n", macro_name, value_one_macro,
        value_two_macro
    );
}

bool yak_test_run_case(const char *suit_name, const char *case_name) {
    yak_test_intern_assert(case_name != NULL);
    yak_test_intern_assert(suit_name != NULL);

    const yak_test_intern_TestCase *yak_test_case = NULL;
    const yak_test_intern_Suit *suit = yak_test_suit_find_by_name(suit_name);

    if (suit == NULL) {
        fprintf(options.output_stream, "yak_test_run_case(): Unable to find suit\n");
        return false;
    }

    for (uint32_t i = 0; i < suit->total_tests; i++) {
        yak_test_case = suit->tests[i];
        if (strcmp(case_name, yak_test_case->name)) {
            yak_test_runner_run_test(yak_test_case, suit);

            return true;
        }
    }

    fprintf(
        options.output_stream, "yak_test_run_case(): Unable to find test \"%s\" in suit \"%s\"", case_name,
        suit_name
    );
    return false;
}

bool yak_test_run_suit(const char *name) {
    yak_test_intern_assert(name != NULL);
    yak_test_intern_Suit *suit = yak_test_suit_find_by_name(name);

    if(suit == NULL) {
        return false;
    }

    yak_test_runner_run_suit(suit);
    return true;
}

void yak_test_run_all(void) {
    for (uint32_t i = 0; i < yak_test_register.total_suits; i++) {
        yak_test_runner_run_suit(yak_test_register.suits[i]);
    }

    yak_test_runner_report();
}

// Algorithm copied from:
// http://yucoding.blogspot.com/2013/02/leetcode-question-123-wildcard-matching.html
static bool yak_test_match_wildcard(char *const match, char *const pattern) {
    char *pattern_seek_ptr = pattern;
    char *match_seek_ptr = match;
    char *star_match_index = NULL;
    char *star_pattern_index = NULL;

    // We always match atleast one character. Globs '*' is equvalent to regex '+' operator
    while (*pattern) {
        // '?' simply matches exactly one character
        if (*match_seek_ptr == '?' || *pattern == *match_seek_ptr) {
            pattern_seek_ptr++;
            match_seek_ptr++;
            continue;
        }

        // '*' pattern start
        if (*pattern_seek_ptr == '*') {
            // remember old position, in case of a mismatch
            star_match_index = match_seek_ptr;
            star_pattern_index = pattern_seek_ptr;
            // just increment the match ptr because the pattern might only match one character
            match_seek_ptr++;
            continue;
        }

        // Nothing matches
        if (star_pattern_index) {
            // '*' matched one character
            match_seek_ptr = star_match_index + 1;
            pattern_seek_ptr = star_pattern_index + 1;
            star_match_index++;
            continue;
        }

        return false;
    }

    while(*pattern_seek_ptr == '*') pattern_seek_ptr ++;

    return true;
}

static bool yak_test_parse_arguments(int argc, char **argv) {
    yak_test_intern_assert(argv != NULL);
    yak_test_intern_assert(argc > 0);

    memset(&options, 0, sizeof(options));

    for (int32_t i = 1; i < argc; i++) {
        char **second_argument_target = NULL;
        bool is_valid_argument = false;

        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            is_valid_argument = true;
            options.verbose = true;
        } else if (strcmp(argv[i], "--list") == 0) {
            is_valid_argument = true;
            options.print_list = true;
        } else if (strcmp(argv[i], "--colored") == 0) {
            is_valid_argument = true;
            second_argument_target = &options.raw.colored_value;
        } else if (strcmp(argv[i], "--output") == 0) {
            is_valid_argument = true;
            second_argument_target = &options.raw.output_value;
        } else if (strcmp(argv[i], "--filter") == 0) {
            is_valid_argument = true;
            second_argument_target = &options.raw.filter_value;
        }

        if (is_valid_argument == false) {
            fprintf(stderr, "unrecognized flag: %s\n", argv[i]);
            return false;
        }

        if (second_argument_target != NULL) {
            if (i + 1 >= argc) {
                fprintf(stderr, "missing option for flag: %s\n", argv[i]);
                return false;
            }

            *second_argument_target = argv[i + 1];
            i += 1;
        }
    }

    return true;
}

static bool yak_test_parse_options(void) {
    char *option = options.raw.output_value;
    char *flag = "--output";
    if (option != NULL) {
        if(access(option, F_OK) == 0) {
            fprintf(stderr, "Output file already exists: %s\n", option);
            return false;
        }

        FILE *output_file = fopen(option, "w");
        if (output_file == NULL) {
            fprintf(stderr, "Failed to open output file: %s: %s\n", option, strerror(errno));
            return false;
        }

        options.output_stream = output_file;
    } else {
        options.output_stream = stdout;
    }

    option = options.raw.colored_value;
    flag = "--colored";
    if (options.raw.colored_value != NULL) {
        if (strcmp(option, "auto") == 0) {
            // FIXME: fileno() returns -1 on error, which might cause issues with isatty
            options.colored =
                isatty(fileno(options.output_stream)) ? true : false;
        } else if (strcmp(option, "always") == 0) {
            options.colored = true;
        } else if (strcmp(option, "never") == 0) {
            options.colored = false;
        } else {
            goto invalid_option;
        }
    } else {
        options.colored = false;
    }

    option = options.raw.filter_value;
    flag = "--filter";
    if (option == NULL) {
        /* TODO */
    }

    option = options.raw.colored_value;
    flag = "--list";
    if (options.print_list == true) {
        /* TODO */
    }

    return true;
invalid_option:
    fprintf(stderr, "invalid option for flag '%s': %s\n", flag, option);
    return false;
}

bool yak_test_init(int argc, char **argv) {
    if (yak_test_parse_arguments(argc, argv) == false) {
        return false;
    }

    if (yak_test_parse_options() == false) {
        return false;
    }

    yak_test_log_init();
    yak_test_runner_reset();

    memset(&yak_test_register, 0, sizeof(yak_test_register));

    // Register all suits
    uint32_t suit_index = 0;
    for (yak_test_intern_Suit *suit = _YAK_TEST_START_SUIT_SECTION;
         suit < _YAK_TEST_STOP_SUIT_SECTION; suit++) {
        yak_test_register.suits = yak_test_realloc(
            yak_test_register.suits,
            sizeof(yak_test_intern_Suit * [yak_test_register.total_suits + 1])
        );

        yak_test_register.suits[suit_index++] = suit;
        yak_test_register.total_suits++;
    }

    yak_test_intern_Suit *last_suit = NULL;
    yak_test_intern_Suit *current_suit = NULL;

    // Register all cases
    for (yak_test_intern_TestCase *yak_test_case = _YAK_TEST_START_CASE_SECTION;
         yak_test_case < _YAK_TEST_STOP_CASE_SECTION; yak_test_case++) {
        // It is (probably) likely that adjacent tests belong to the same suit
        if ((current_suit == NULL) ||
            (last_suit != NULL && strcmp(last_suit->name, yak_test_case->suit_name) != 0)) {
            current_suit = yak_test_suit_find_by_name(yak_test_case->suit_name);
        }

        if (current_suit == NULL) {
            fprintf(
                options.output_stream, "Unable to find suit \"%s\" for test case \"%s\"\n",
                yak_test_case->suit_name, yak_test_case->name
            );
            return false;
        }

        current_suit->tests = yak_test_realloc(
            current_suit->tests, sizeof(yak_test_intern_TestCase * [current_suit->total_tests + 1])
        );

        current_suit->tests[current_suit->total_tests] = yak_test_case;
        current_suit->total_tests++;
        yak_test_register.total_tests++;
        last_suit = current_suit;
    }

    return true;
}

void yak_test_exit(void) {
    if (options.output_stream != NULL && options.raw.output_value != NULL) {
        fclose(options.output_stream);
    }

    yak_test_log_free();

    for (uint32_t i = 0; i < yak_test_register.total_suits; i++) {
        free(yak_test_register.suits[i]->tests);
    }

    free(yak_test_register.suits);
}

#endif
