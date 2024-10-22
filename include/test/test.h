#ifndef TEST_H_
#define TEST_H_

#define _GNU_SOURCE /* fileno() */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef TEST_DEBUG
    #if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
        #define TEST_BUILTIN_TRAP __builtin_debugtrap()
    #else
        #define TEST_BUILTIN_TRAP __asm__("int3")
    #endif

    #define test_intern_assert(condition)                                                    \
        do {                                                                                 \
            if (!(condition)) {                                                              \
                printf(                                                                      \
                    "Assertion failed: [%s %s():%d] \"%s\"\n", __FILE__, __func__, __LINE__, \
                    #condition                                                               \
                );                                                                           \
                TEST_BUILTIN_TRAP;                                                           \
            }                                                                                \
        } while (0)
#else
    #define test_intern_assert(condition)
#endif // TEST_DEBUG

#if defined(__clang__) || defined(__GNUC__)
    #define TEST_CASE_SECTION __attribute__((used, aligned(8), section("test_case_section")))
    #define TEST_SUIT_SECTION __attribute__((used, aligned(8), section("test_suit_section")))
    #define TEST_UNUSED       __attribute__((unused))
#else
    #error "Compiler not supported :^("
#endif

/// The maximum number of allowed filters
#ifndef TEST_FILTER_COUNT_LIMIT
    #define TEST_FILTER_COUNT_LIMIT 8
#endif

/// The maximum size of a single filters
#ifndef TEST_FILTER_SIZE_LIMIT
    #define TEST_FILTER_SIZE_LIMIT 128
#endif

/// The maximum length of any single log message
#ifndef TEST_LOG_BUFFER_SIZE
    #define TEST_LOG_BUFFER_SIZE 1024
#endif

/// Macro for creating a new suit
/// @param suit_name The name of the new suit
/// @param setup The name of the setup function. Called before every test
/// @param setup The name of the teardown function. Called after every test
#define SUIT(suit_name, setup, teardown)                        \
    static const test_intern_SuitData test_suit_##suit_name = { \
        .name = #suit_name,                                     \
        .teardown_function = teardown,                          \
        .setup_function = setup,                                \
    };                                                          \
    TEST_SUIT_SECTION                                           \
    const test_intern_SuitData *test_suit_ptr_##suit_name = &test_suit_##suit_name

/// Macro for creating a new test case.
/// @param suit_name The name of the suit this test case should be added to.
/// @param test_name The name of this test case
#define TEST(suit_name_, test_name)                                            \
    static void test_##suit_name_##_##test_name(test_intern_Result *_result);  \
    static const test_intern_TestCase test_case_##test_name##_##suit_name_ = { \
        .name = #test_name,                                                    \
        .line = __LINE__,                                                      \
        .file_name = __FILE__,                                                 \
        .suit_name = #suit_name_,                                              \
        .function = test_##suit_name_##_##test_name                            \
    };                                                                         \
    TEST_CASE_SECTION                                                          \
    const test_intern_TestCase *test_case_ptr_##test_name##_##suit_name_ =     \
        &test_case_##test_name##_##suit_name_;                                 \
    static void test_##suit_name_##_##test_name(TEST_UNUSED test_intern_Result *_result)

#define TEST_CMP(type, lhs, rhs, macro, CMP_FUNC)                                       \
    do {                                                                                \
        if (!CMP_FUNC(lhs, rhs)) {                                                      \
            (*_result) = test_intern_ResultFailed;                                      \
            test_log_write("failed at %d: " #macro "(" #lhs ", " #rhs "): ", __LINE__); \
            return;                                                                     \
        }                                                                               \
    } while (0)

#define TEST_CMP_MEM(type, lhs, rhs, size, macro, CMP_FUNC)                             \
    do {                                                                                \
        if (!CMP_FUNC(lhs, rhs, size)) {                                                \
            (*_result) = test_intern_ResultFailed;                                      \
            test_log_write("failed at %d: " #macro "(" #lhs ", " #rhs "): ", __LINE__); \
            return;                                                                     \
        }                                                                               \
    } while (0)

// clang-format off
#define TEST_CMP_EQ(lhs, rhs) ((lhs) == (rhs))
#define TEST_CMP_NE(lhs, rhs) ((lhs) != (rhs))

#define TEST_CMP_GT(lhs, rhs) ((lhs) > (rhs))
#define TEST_CMP_LT(lhs, rhs) ((lhs) < (rhs))
#define TEST_CMP_GE(lhs, rhs) ((lhs) >= (rhs))
#define TEST_CMP_LE(lhs, rhs) ((lhs) <= (rhs))

#define TEST_CMP_STR_EQ(lhs, rhs) (strcmp(lhs, rhs) == 0)
#define TEST_CMP_STR_NE(lhs, rhs) (strcmp(lhs, rhs) != 0)

#define TEST_CMP_MEM_EQ(lhs, rhs, size) (memcmp(lhs, rhs, size) == 0)
#define TEST_CMP_MEM_NE(lhs, rhs, size) (memcmp(lhs, rhs, size) != 0)

// Assert Macros
#define test_assert(lhs)                 TEST_CMP(test_TypeCustom, lhs, 1, assert, TEST_CMP_EQ)
#define test_assert_eq(lhs, rhs)         TEST_CMP(test_TypeCustom, lhs, rhs, assert_eq, TEST_CMP_EQ)
#define test_assert_ne(lhs, rhs)         TEST_CMP(test_TypeCustom, lhs, rhs, assert_ne, TEST_CMP_NE)

#define test_assert_string_eq(lhs, rhs)         TEST_CMP(test_TypeCString, lhs, rhs, assert_str_eq, TEST_CMP_STR_EQ)
#define test_assert_string_ne(lhs, rhs)         TEST_CMP(test_TypeCString, lhs, rhs, assert_str_ne, TEST_CMP_STR_NE)

#define test_assert_memory_eq(lhs, rhs, size)         TEST_CMP_MEM(test_TypeCString, lhs, rhs, size, assert_str_eq, TEST_CMP_MEM_EQ)
#define test_assert_memory_ne(lhs, rhs, size)         TEST_CMP_MEM(test_TypeCString, lhs, rhs, size, assert_str_ne, TEST_CMP_MEM_NE)
// clang-format on

#define test_intern_ResultCount 4
typedef enum {
    test_intern_ResultOk,
    test_intern_ResultPartiallyOk,
    test_intern_ResultSkipped,
    test_intern_ResultFailed,
} test_intern_Result;

typedef void (*test_TestFunction)(test_intern_Result *_state);
typedef void (*test_SetupFunction)(void);
typedef void (*test_TeardownFunction)(void);

typedef struct {
    uint32_t line;
    char *name;
    char *suit_name;
    char *file_name;
    test_TestFunction function;
} test_intern_TestCase;

typedef struct {
    char *name;
    test_SetupFunction setup_function;
    test_TeardownFunction teardown_function;
} test_intern_SuitData;

/// Initialize the testing framework
extern bool test_init(int argc, char **argv);
extern void test_exit(void);

/// Used internally. Write to the libraries log buffer. Used in TEST(...) macro
__attribute__((format(printf, 1, 2))) extern void test_log_write(const char *format, ...);

// TODO: Implement manual test registration
/*extern bool test_register_suit(char *name, test_SetupFunction setup_func, test_TeardownFunction
 * teardown_func);*/
/*extern bool test_register_case(char *suit_name, char *test_name, test_TestFunction func);*/

/// Run all tests, respecting filters specified on the commandline
extern void test_run_all(void);

#endif // TEST_H_

#ifdef TEST_IMPLEMENTATION

#include <errno.h>  /* errno */
#include <malloc.h> /* malloc, calloc */
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h> /* abort, exit */
#include <string.h> /* sterror, strcmp, memset */
#include <unistd.h> /* isatty */

#include <ctype.h> /* isalnum */

#if defined(__clang__) || defined(__GNUC__)
// see:
// https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
extern test_intern_TestCase *__start_test_case_section;
extern test_intern_TestCase *__stop_test_case_section;
    #define TEST_START_CASE_SECTION (__start_test_case_section)
    #define TEST_STOP_CASE_SECTION  (__stop_test_case_section)

extern test_intern_SuitData *__start_test_suit_section;
extern test_intern_SuitData *__stop_test_suit_section;
    #define TEST_START_SUIT_SECTION (__start_test_suit_section)
    #define TEST_STOP_SUIT_SECTION  (__stop_test_suit_section)

#else
    #error "Compiler not supported :^("
#endif

#define COLOR_RESET  "\033[0m"
#define COLOR_DIM    "\033[2m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN   "\033[36m"

typedef struct {
    uint32_t test_count;
    const test_intern_TestCase **test_list;
    const test_intern_SuitData *suit_data;
} test_intern_Suit;

static struct {
    uint32_t total_suits;
    uint32_t total_tests;
    test_intern_Suit *suit_list;
} test_register = { 0 };

static struct {
    uint32_t tests_successful;
    uint32_t tests_attempted;
    uint32_t tests_failed;
    uint32_t tests_partially;
    uint32_t tests_skipped;
    uint32_t suits_failed;
    uint32_t suits_skipped;
} test_runner = { 0 };

static struct {
    uint32_t length;
    uint32_t offset;
    char buffer[TEST_LOG_BUFFER_SIZE];
} log_data = { 0 };

static struct {
    bool colored;
    bool show_help;
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

    char *filter_pattern[TEST_FILTER_COUNT_LIMIT];
    char *filter_pattern_buffer;
    uint32_t filter_pattern_count;
} options = { 0 };

static inline void *test_realloc(void *base, uintptr_t size) {
    void *result = realloc(base, size);
    if (result == NULL) {
        perror("test: ");
        abort();
    }

    return result;
}

static inline void *test_calloc(uint64_t nmem, uint64_t smem) {
    void *result = calloc(nmem, smem);
    if (result == NULL) {
        perror("test: ");
        abort();
    }

    return result;
}

static inline void test_log_clear(void) { log_data.offset = 0; }

__attribute__((format(printf, 1, 2))) void test_log_write(const char *format, ...) {
    test_intern_assert(format != NULL);

    va_list args;
    va_start(args, format);
    vsnprintf(
        log_data.buffer + log_data.offset, TEST_LOG_BUFFER_SIZE - log_data.offset, format, args
    );
    va_end(args);

    fprintf(options.output_stream, "%s", log_data.buffer);
    test_log_clear();
}

/* Test runner */
static void test_runner_report(void) {
    uint32_t failed_percent = (test_runner.tests_failed > 0)
                                  ? (test_runner.tests_failed * 100 / test_register.total_tests)
                                  : 0;
    uint32_t skipped_percent = (test_runner.tests_skipped > 0)
                                   ? (test_runner.tests_skipped * 100 / test_register.total_tests)
                                   : 0;
    uint32_t success_percent =
        (test_runner.tests_successful) > 0
            ? (test_runner.tests_successful * 100 / test_register.total_tests)
            : 0;
    uint32_t partial_percent = (test_runner.tests_partially > 0)
                                   ? (test_runner.tests_partially * 100 / test_register.total_tests)
                                   : 0;

    test_log_write(
        "attempted to run %d out of %d tests\n", test_runner.tests_attempted,
        test_register.total_tests
    );
    test_log_write("successful: %u - %3u%%\n", test_runner.tests_successful, success_percent);
    test_log_write("failed    : %u - %3u%%\n", test_runner.tests_failed, failed_percent);
    test_log_write("partially : %u - %3u%%\n", test_runner.tests_partially, partial_percent);
    test_log_write("skipped   : %u - %3u%%\n", test_runner.tests_skipped, skipped_percent);
}

static void
test_runner_run_test(const test_intern_TestCase *test, const test_intern_SuitData *suit) {
    test_intern_assert(test != NULL);
    test_intern_assert(suit != NULL);

    test_log_write(
        "%s%s @ %d running '%s:%s':%s ", (options.colored) ? COLOR_DIM : "", test->file_name,
        test->line, suit->name, test->name, (options.colored) ? COLOR_RESET : ""
    );

    if (suit->setup_function != NULL) {
        suit->setup_function();
    }

    test_intern_Result result = test_intern_ResultOk;
    test->function(&result);

    if (suit->teardown_function != NULL) {
        suit->teardown_function();
    }

    test_runner.tests_attempted++;

    switch (result) {
    case test_intern_ResultOk:
        test_runner.tests_successful++;
        break;
    case test_intern_ResultPartiallyOk:
        test_runner.tests_partially++;
        break;
    case test_intern_ResultSkipped:
        test_runner.tests_skipped++;
        break;
    case test_intern_ResultFailed:
        test_runner.tests_failed++;
        break;
    }

    static const char *result_strings_colored[] = {
        [test_intern_ResultOk] = COLOR_GREEN "ok" COLOR_RESET,
        [test_intern_ResultPartiallyOk] = COLOR_YELLOW "partially ok" COLOR_RESET,
        [test_intern_ResultSkipped] = COLOR_CYAN "skipped" COLOR_RESET,
        [test_intern_ResultFailed] = COLOR_RED "failed" COLOR_RESET,
    };
    static const char *result_strings_blank[] = {
        [test_intern_ResultOk] = "ok",
        [test_intern_ResultPartiallyOk] = "partially ok",
        [test_intern_ResultSkipped] = "skipped",
        [test_intern_ResultFailed] = "failed",
    };

    const char **result_strings = (options.colored) ? result_strings_colored : result_strings_blank;
    test_log_write("%s\n", result_strings[result]);
}

static void test_runner_run_suit(const test_intern_Suit *suit_data) {
    test_intern_assert(suit_data != NULL);

    for (uint32_t i = 0; i < suit_data->test_count; i++) {
        test_runner_run_test(suit_data->test_list[i], suit_data->suit_data);
    }
}

static test_intern_Suit *test_suit_find_by_name(const char *name) {
    test_intern_assert(name != NULL);

    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        test_intern_Suit *suit_data = &test_register.suit_list[i];

        if (strcmp(suit_data->suit_data->name, name) == 0) {
            return suit_data;
        }
    }

    return NULL;
}

// TODO: Implement manual suit registration
bool test_register_suit(
    char *name, test_SetupFunction setup_func, test_TeardownFunction teardown_func
) {
    (void)name;
    (void)setup_func;
    (void)teardown_func;

    return false;
}

// TODO: Implement manual test registration
bool test_register_case(char *suit_name, char *test_name, test_TestFunction func) {
    (void)suit_name;
    (void)test_name;
    (void)func;

    return false;
}

// Algorithm copied from:
// http://yucoding.blogspot.com/2013/02/leetcode-question-123-wildcard-matching.html
static bool test_wildcard_match(const char *text, const char *pattern) {
    uint32_t text_offset = 0;
    uint32_t pattern_offset = 0;

    int32_t bt_text_offset = -1;
    int32_t bt_pattern_offset = -1;

    while (text_offset < strlen(text)) {
        // Simply match one character
        if (text[text_offset] == pattern[pattern_offset] || pattern[pattern_offset] == '?') {
            pattern_offset++;
            text_offset++;
            continue;
        }

        // Start of a '*' matching sequence
        if (pattern[pattern_offset] == '*') {
            // Store offset for backtrace
            bt_text_offset = text_offset;
            bt_pattern_offset = pattern_offset;
            pattern_offset++;
            continue;
        }

        // Nothing matched. Consume one character and move on
        if (bt_pattern_offset != -1) {
            pattern_offset = bt_pattern_offset + 1;
            text_offset = ++bt_text_offset;
            continue;
        }

        return false;
    }

    // Consume any leading '*'
    while (pattern_offset < strlen(pattern) && pattern[pattern_offset] == '*') {
        pattern_offset++;
    }

    return pattern_offset == strlen(pattern);
}

static bool test_filter_case(const char *suit_name, const char *test_name) {
    static char name_buffer[TEST_FILTER_SIZE_LIMIT] = { 0 };
    memset(name_buffer, 0, sizeof(name_buffer));

    if (strlen(suit_name) + strlen(test_name) + 1 > TEST_FILTER_SIZE_LIMIT) {
        return false;
    }

    memcpy(name_buffer, suit_name, strlen(suit_name));
    name_buffer[strlen(suit_name)] = ':';
    memcpy(name_buffer + strlen(suit_name) + 1, test_name, strlen(test_name));

    for (uint32_t i = 0; i < options.filter_pattern_count; i++) {
        if (test_wildcard_match(name_buffer, options.filter_pattern[i])) {
            return true;
        }
    }

    return false;
}

void test_run_all(void) {
    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        test_intern_Suit *suit = &test_register.suit_list[i];

        for (uint32_t i = 0; i < suit->test_count; i++) {
            bool is_match = test_filter_case(suit->suit_data->name, suit->test_list[i]->name);

            if (!is_match) {
                test_runner.tests_skipped++;
            }

            if (options.filter_pattern_count == 0 || is_match) {
                test_runner_run_test(suit->test_list[i], suit->suit_data);
            }
        }
    }

    test_runner_report();
}

static void test_list_all(void) {
    test_log_write("All test cases matching the current filters:\n");

    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        test_intern_Suit *suit = &test_register.suit_list[i];

        for (uint32_t i = 0; i < suit->test_count; i++) {
            if (options.filter_pattern_count == 0 ||
                test_filter_case(suit->suit_data->name, suit->test_list[i]->name)) {
                test_log_write("    - %s:%s\n", suit->suit_data->name, suit->test_list[i]->name);
            }
        }
    }
}

static bool test_parse_filter(void) {
    uint32_t length = 0;
    uint32_t filter_count = 0;
    uint32_t filter_start_index = 0;

    options.filter_pattern_buffer = calloc(strlen(options.raw.filter_value) + 1, 1);

    for (uint32_t i = 0; i < strlen(options.raw.filter_value); i++) {
        char current_char = options.raw.filter_value[i];

        if (isalnum(current_char) || current_char == '_' || current_char == ':' ||
            current_char == '?' || current_char == '*') {
            options.filter_pattern_buffer[i] = current_char;
            length++;

        } else if (current_char != ',') {
            fprintf(stderr, "[test]: Pattern contains invalid character\n");
            return false;
        }

        if (current_char == ',' || i + 1 >= strlen(options.raw.filter_value)) {
            /* Check for trailing comma */
            if (length == 0 || (current_char == ',' && i + 1 >= strlen(options.raw.filter_value))) {
                fprintf(stderr, "[test]: Filter pattern can not be empty\n");
                return false;
            }

            if (filter_count >= TEST_FILTER_COUNT_LIMIT) {
                fprintf(stderr, "[test]: Reached filter limit\n");
                return false;
            }

            options.filter_pattern[filter_count] =
                &options.filter_pattern_buffer[filter_start_index];

            filter_start_index = i + 1;
            filter_count++;
            length = 0;
        }
    }

    options.filter_pattern_count = filter_count;

    return true;
}

static void print_help(void) {
    const char *help_text =
        "Usage\n"
        "      <test_binary> [OPTIONS] ... -- [ARGS]\n"
        "\n"
        "ARGS\n"
        "      Arguments that will be ignored by this library\n"
        "\n"
        "OPTIONS\n"
        "      The binary build with this library accepts the following options:\n"
        "\n"
        "      --help\n"
        "        Display this message\n"
        "\n"
        "      --list\n"
        "        Print all registered test cases. Respectes filters.\n"
        "\n"
        "      --filter <filters>\n"
        "        A list of filter patterns to selectively execute tests/suits.\n"
        "        Patterns can contain '*' (matches zero or more characters),\n"
        "        '?' (matches a single character) or the following characters:\n"
        "            A-Z, a-z, 0-9, ':', '_'\n"
        "        To specify multiple patterns, separated them by commas.\n"
        "\n"
        "      --output <file>\n"
        "        Redirect library output to a new file.\n"
        "\n"
        "      --colored (auto|always|never)\n"
        "        Colorize the output.\n";

    printf("%s", help_text);
}

static bool test_parse_arguments(int argc, char **argv) {
    test_intern_assert(argv != NULL);
    test_intern_assert(argc > 0);

    memset(&options, 0, sizeof(options));

    for (int32_t i = 1; i < argc; i++) {
        char **second_argument_target = NULL;
        bool is_valid_argument = false;

        if (strcmp(argv[i], "--") == 0) {
            return true;
        } else if (strcmp(argv[i], "--help") == 0) {
            is_valid_argument = true;
            options.show_help = true;
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
            fprintf(stderr, "[test]: Unrecognized flag: %s\n", argv[i]);
            return false;
        }

        if (second_argument_target != NULL) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[test]: Missing option for flag: %s\n", argv[i]);
                return false;
            }

            *second_argument_target = argv[i + 1];
            i += 1;
        }
    }

    return true;
}

static bool test_parse_options(void) {
    char *option = options.raw.output_value;
    char *flag = "--output";
    if (option != NULL) {
        if (access(option, F_OK) == 0) {
            fprintf(stderr, "[test]: Output file already exists: %s\n", option);
            return false;
        }

        FILE *output_file = fopen(option, "w");
        if (output_file == NULL) {
            fprintf(
                stderr, "[test]: Failed to open output file: %s: %s\n", option, strerror(errno)
            );
            return false;
        }

        options.output_stream = output_file;
    } else {
        options.output_stream = stdout;
    }

    flag = "--colored";
    if (options.raw.colored_value == NULL) {
        options.raw.colored_value = "auto";
    }
    option = options.raw.colored_value;

    if (strcmp(option, "auto") == 0) {
        options.colored = isatty(fileno(options.output_stream)) ? true : false;
    } else if (strcmp(option, "always") == 0) {
        options.colored = true;
    } else if (strcmp(option, "never") == 0) {
        options.colored = false;
    } else {
        goto invalid_option;
    }

    option = options.raw.filter_value;
    flag = "--filter";
    if (option != NULL) {
        if (!test_parse_filter()) {
            return false;
        }
    }

    return true;
invalid_option:
    fprintf(stderr, "[test]: Invalid option for flag '%s': %s\n", flag, option);
    return false;
}

bool test_init(int argc, char **argv) {
    if (!test_parse_arguments(argc, argv)) {
        return false;
    }

    if (options.show_help) {
        print_help();
        return false;
    }

    if (!test_parse_options()) {
        return false;
    }

    memset(&log_data, 0, sizeof(log_data));
    memset(&test_runner, 0, sizeof(test_runner));
    memset(&test_register, 0, sizeof(test_register));

    // Register all suits
    uintptr_t suit_count = &TEST_STOP_SUIT_SECTION - &TEST_START_SUIT_SECTION;
    test_intern_assert(suit_count > 0);
    test_intern_assert(suit_count < (uint32_t)(-1));

    test_register.total_suits = (uint32_t)suit_count;
    test_register.suit_list = test_calloc(sizeof(test_intern_Suit[suit_count]), 1);

    // Register all suits
    for (uint32_t suit_index = 0; suit_index < suit_count; suit_index++) {
        // FIXME: When compiling with GCC-14, UBSan reports: runtime error: load of address
        // <address> with insufficient space for an object of type 'struct test_intern_SuitData *'
        // <address> is the (correct) address of an object of type 'test_intern_SuitData', as is
        // expected.
        test_register.suit_list[suit_index].suit_data = (&TEST_START_SUIT_SECTION)[suit_index];
    }

    test_intern_Suit *last_suit = NULL;
    test_intern_Suit *current_suit = NULL;

    // Register all cases
    for (test_intern_TestCase **test_case_iter = &TEST_START_CASE_SECTION;
         test_case_iter < &TEST_STOP_CASE_SECTION; test_case_iter++) {
        const test_intern_TestCase *test_case = *test_case_iter;

        // It is (probably) likely that adjacent tests belong to the same suit
        if ((current_suit == NULL) ||
            (last_suit != NULL && strcmp(last_suit->suit_data->name, test_case->suit_name) != 0)) {
            current_suit = test_suit_find_by_name(test_case->suit_name);
        }

        if (current_suit == NULL) {
            fprintf(
                options.output_stream, "Unable to find suit \"%s\" for test case \"%s\"\n",
                test_case->suit_name, test_case->name
            );
            return false;
        }

        current_suit->test_list = test_realloc(
            current_suit->test_list, sizeof(test_intern_TestCase * [current_suit->test_count + 1])
        );

        current_suit->test_list[current_suit->test_count] = test_case;
        current_suit->test_count++;
        test_register.total_tests++;
        last_suit = current_suit;
    }

    if (options.print_list) {
        test_list_all();
        test_exit();
        return false;
    }

    return true;
}

void test_exit(void) {
    if (options.output_stream != NULL && options.raw.output_value != NULL) {
        fclose(options.output_stream);
    }

    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        free(test_register.suit_list[i].test_list);
    }

    free(test_register.suit_list);
}

#endif
