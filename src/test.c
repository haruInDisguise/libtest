#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>
#include <test/test.h>

#if defined(__clang__) || defined(__gcc__)
// see:
// https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
extern test_intern_TestCase __start__test_case_section;
extern test_intern_TestCase __stop__test_case_section;
#define _TEST_START_CASE_SECTION &__start__test_case_section
#define _TEST_STOP_CASE_SECTION &__stop__test_case_section

extern test_intern_Suit __start__test_suit_section;
extern test_intern_Suit __stop__test_suit_section;
#define _TEST_START_SUIT_SECTION &__start__test_suit_section
#define _TEST_STOP_SUIT_SECTION &__stop__test_suit_section
#else
#error "Compiler not supported :^("
#endif

#define COLOR_RESET "\033[0m"
#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_DEFAULT "\033[38m"

static struct {
    uint32_t total_suits;
    uint32_t total_tests;
    test_intern_Suit **suits;
} test_register = {0};

static struct {
    uint32_t tests_executed;
    uint32_t tests_successful;
    uint32_t tests_failed;
    uint32_t tests_partially;
    uint32_t tests_skipped;
    uint32_t suits_executed;
    uint32_t suits_failed;
    uint32_t suits_skipped;
} test_runner = {0};

static struct {
    uint32_t length;
    uint32_t offset;
    char *buffer;
} log_data = {0};


static struct {
    bool output_is_colored;
    bool verbose;

    // Argument flag values as they are being read from the commandline
    struct {
        char *colored_flag;
        char *output_flag;
        char *filter_flag;
    } raw;

    FILE *output_stream;

    char **target_suits;
    char **target_cases;
} test_options = {0};

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

static void test_log_init(void) {
    log_data.offset = 0;
    log_data.length = TEST_LOG_BUFFER_SIZE;
    log_data.buffer = test_calloc(1, TEST_LOG_BUFFER_SIZE * sizeof(*log_data.buffer));
}

static void test_log_free(void) { free(log_data.buffer); }

// Reset the log buffers offset, overwriting previous contents
static void test_log_clear(void) {
    log_data.offset = 0;
    // memset(log_data.buffer, 0, log_data.length * sizeof (*log_data.buffer));
}

// Write formatted va_list to log_data.buffer and clear it afterwards.
static void test_log_write(const char *format, va_list *args) {
    test_intern_assert(args != NULL);
    test_intern_assert(format != NULL);
    test_intern_assert(log_data.length - log_data.offset > 0);

    vsnprintf(log_data.buffer + log_data.offset, log_data.length - log_data.offset, format, *args);
    fprintf(test_options.output_stream, "%s", log_data.buffer);
    test_log_clear();
}

// Write a message to <log_stream> with a prefix according to the value of <status>
__attribute__((format(printf, 2, 3))) static void test_log_write_with_status(
    const test_intern_Status status, const char *format, ...) {
    test_intern_assert(format != NULL);

    static const char *status_strings_blank[] = {
        [test_intern_StatusPending] = "PENDING",   [test_intern_StatusResult] = "RESULT ",
        [test_intern_StatusStart] = "RUNNING",     [test_intern_StatusEnd] = "REPORT ",
        [test_intern_StatusContinued] = "-------",
    };
    // static const char *status_string_colored[] = {
    //     [test_intern_StatusPending] = "PENDING" COLOR_RESET,
    //     [test_intern_StatusResult]  = "RESULT " COLOR_RESET,
    //     [test_intern_StatusStart]   = "RUNNING" COLOR_RESET,
    //     [test_intern_StatusEnd]     = COLOR_CYAN    "REPORT " COLOR_RESET,
    //     [test_intern_StatusContinued] = "-------",
    // };

    if (status == test_intern_StatusEmpty) {
        log_data.offset = 0;
    } else {
        log_data.offset =
            snprintf(log_data.buffer, log_data.length, "[ %s ] ", status_strings_blank[status]);
    }

    va_list args;
    va_start(args, format);

    test_log_write(format, &args);

    va_end(args);
}

/* Test runner */
static void test_runner_report(void) {
    uint32_t tests_failed_percent =
        (test_runner.tests_failed > 0)
            ? (test_runner.tests_failed * 100 / test_register.total_tests)
            : 0;
    uint32_t tests_skipped_percent =
        (test_runner.tests_skipped > 0)
            ? (test_runner.tests_skipped * 100 / test_register.total_tests)
            : 0;

    uint32_t tests_successful_percent =
        (test_runner.tests_successful) > 0
            ? (test_runner.tests_successful * 100 / test_runner.tests_executed)
            : 0;
    uint32_t tests_partial_percent =
        (test_runner.tests_partially > 0)
            ? (test_runner.tests_partially * 100 / test_runner.tests_executed)
            : 0;
    uint32_t tests_executed_percent =
        (test_runner.tests_executed > 0)
            ? (test_runner.tests_executed * 100 / test_register.total_tests)
            : 0;

    // clang-format off
    test_log_write_with_status(test_intern_StatusEnd, "=== Testing Results ===\n");
    test_log_write_with_status(test_intern_StatusContinued, "Successful:   %u/%u (%u%%)\n", test_runner.tests_executed, test_register.total_tests, tests_executed_percent);
    test_log_write_with_status(test_intern_StatusContinued, "- Totally   : %u - %u%%\n", test_runner.tests_successful, tests_successful_percent);
    test_log_write_with_status(test_intern_StatusContinued, "- Partially : %u - %u%%\n", test_runner.tests_partially, tests_partial_percent);
    test_log_write_with_status(test_intern_StatusContinued, "Failed  : %u - %3u%%\n", test_runner.tests_failed, tests_failed_percent);
    test_log_write_with_status(test_intern_StatusContinued, "Skipped : %u - %3u%%\n", test_runner.tests_skipped, tests_skipped_percent);
    // clang-format on
}

static void test_runner_run_test(const test_intern_TestCase *test, const test_intern_Suit *suit) {
    test_intern_assert(test != NULL);
    test_intern_assert(suit != NULL);

    test_intern_TestInfo info = {.result = test_intern_ResultOk};

    test_log_write_with_status(test_intern_StatusStart, "%s => %s:%s\n", test->file_name,
                               suit->name, test->name);

    if (suit->setup_function != NULL) {
        suit->setup_function();
    }

    test->function(&info);

   // test_intern_assert(info.result <= test_intern_ResultPartiallyOk);
   // test_intern_assert(info.assertion_info.macro_value_one != NULL);
   // test_intern_assert(info.assertion_info.macro_value_two != NULL);

    if (suit->teardown_function != NULL) {
        suit->teardown_function();
    }

    switch (info.result) {
        case test_intern_ResultOk:
            test_runner.tests_successful++;
            test_runner.tests_executed++;
            break;
        case test_intern_ResultPartiallyOk:
            test_runner.tests_partially++;
            test_runner.tests_executed++;
            break;
        case test_intern_ResultSkipped:
            test_runner.tests_skipped++;
            break;
        case test_intern_ResultFailed:
            test_runner.tests_failed++;
            break;
    }

    static const char *result_strings_colored[] = {
        [test_intern_ResultOk] = COLOR_GREEN "OK" COLOR_RESET,
        [test_intern_ResultPartiallyOk] = COLOR_YELLOW "PARTIALLY OK" COLOR_RESET,
        [test_intern_ResultSkipped] = COLOR_CYAN "SKIPPED" COLOR_RESET,
        [test_intern_ResultFailed] = COLOR_RED "FAILED" COLOR_RESET,
    };
    static const char *result_strings_blank[] = {
        [test_intern_ResultOk] = "OK",
        [test_intern_ResultPartiallyOk] = "PARTIALLY OK",
        [test_intern_ResultSkipped] = "SKIPPED",
        [test_intern_ResultFailed] = "FAILED",
    };

    const char **result_strings =
        test_options.output_is_colored ? result_strings_colored : result_strings_blank;
    test_log_write_with_status(test_intern_StatusResult, "%s\n", result_strings[info.result]);
}

static void test_runner_run_suit(const test_intern_Suit *suit) {
    test_intern_assert(suit != NULL);

    for (uint32_t i = 0; i < suit->total_tests; i++) {
        test_runner_run_test(suit->tests[i], suit);
    }

    test_runner.suits_executed++;
}

static void test_runner_reset(void) { memset(&test_runner, 0, sizeof(test_runner)); }

/* Helper */
static test_intern_Suit *test_suit_find_by_name(const char *name) {
    test_intern_assert(name != NULL);

    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        test_intern_Suit *suit = test_register.suits[i];

        if (strcmp(suit->name, name) == 0) {
            return suit;
        }
    }

    return NULL;
}

void test_intern_log_assertion_failed(const test_intern_TestInfo *test_info) {
    test_intern_assert(test_info != NULL);

    const char *value_one = test_info->assertion_info.value_one_buffer;
    const char *value_one_macro = test_info->assertion_info.macro_value_one;
    const char *value_two = test_info->assertion_info.value_two_buffer;
    const char *value_two_macro = test_info->assertion_info.macro_value_two;
    const char *macro_name = test_info->assertion_info.macro_name;
    const uint32_t line = test_info->assertion_info.line;

    const char *message_prefix =
        test_info->result == test_intern_ResultFailed ? "assertion failed: " : "check failed: ";

    test_log_write_with_status(test_intern_StatusPending, "%s\"%s\" != \"%s\" at %u",
                               message_prefix, value_one, value_two, line);
    test_log_write_with_status(test_intern_StatusEmpty, " -> %s(%s, %s)\n", macro_name,
                               value_one_macro, value_two_macro);
}

void test_run_case(const char *suit_name, const char *case_name) {
    test_intern_assert(case_name != NULL);
    test_intern_assert(suit_name != NULL);

    const test_intern_TestCase *test_case = NULL;
    const test_intern_Suit *suit = test_suit_find_by_name(suit_name);

    if (suit == NULL) {
        fprintf(stderr, "test_run_case(): Unable to find suit\n");
        exit(1);
    }

    for (uint32_t i = 0; i < suit->total_tests; i++) {
        test_case = suit->tests[i];
        if (strcmp(case_name, test_case->name)) {
            test_runner_run_test(test_case, suit);

            return;
        }
    }

    // TODO: Log to file
    fprintf(stderr, "test_run_case(): Unable to find test \"%s\" in suit \"%s\"", case_name,
            suit_name);
    exit(1);
}

void test_run_suit(const char *name) {
    test_intern_assert(name != NULL);
    test_intern_Suit *suit = test_suit_find_by_name(name);
    test_runner_run_suit(suit);
}

void test_run_all(void) {
    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        test_runner_run_suit(test_register.suits[i]);
    }

    test_runner_report();
}

static bool test_parse_arguments(int argc, char **argv) {
    test_intern_assert(argv != NULL);
    test_intern_assert(argc > 0);

    memset(&test_options, 0, sizeof(test_options));

    // I am planning to remove libtest's dependency on libc,
    // and therefore I want to avoid more extensive things like
    // getopt or gnu's getopt_long etc.
    for (int32_t i = 1; i < argc; i++) {
        char **second_argument_target = NULL;
        bool is_valid_argument = false;

        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            is_valid_argument = true;
            test_options.verbose = true;
        } else if (strcmp(argv[i], "--list") == 0) {
            is_valid_argument = true;
        } else if (strcmp(argv[i], "--colored") == 0) {
            is_valid_argument = true;
            second_argument_target = &test_options.raw.colored_flag;
        } else if (strcmp(argv[i], "--output") == 0) {
            is_valid_argument = true;
            second_argument_target = &test_options.raw.output_flag;
        } else if (strcmp(argv[i], "--filter") == 0) {
            is_valid_argument = true;
            second_argument_target = &test_options.raw.filter_flag;
        }

        if (is_valid_argument == false) {
            fprintf(stderr, "unrecognized flag: %s\n", argv[i]);
            return false;
        }

        if(second_argument_target != NULL) {
            if(i + 1 >= argc) {
                fprintf(stderr, "missing option for flag: %s\n", argv[i]);
                return false;
            }

            *second_argument_target = argv[i + 1];
            i += 1;
        }
    }

    return true;
}

static bool test_parse_options(void) {
    char *option = test_options.raw.output_flag;
    char *flag = "--output";
    if(option != NULL) {
        FILE *output_file = fopen(option, "w+");
        if(output_file == NULL) {
            fprintf(stderr, "Failed to open file: %s: %s\n", option, strerror(errno));
            goto invalid_option;
        }

        test_options.output_stream = output_file;
    } else {
        test_options.output_stream = stdout;
    }

    option = test_options.raw.colored_flag;
    flag = "--colored";
    if(test_options.raw.colored_flag != NULL) {
        if(strcmp(option, "auto") == 0) {
            // FIXME: fileno() returns -1 on error, which might cause issues with isatty
            test_options.output_is_colored = isatty(fileno(test_options.output_stream)) ? true : false;
        } else if(strcmp(option, "always") == 0) {
            test_options.output_is_colored = true;
        } else if(strcmp(option, "never") == 0) {
            test_options.output_is_colored = false;
        } else {
            goto invalid_option;
        }
    } else {
        test_options.output_is_colored = false;
    }

    option = test_options.raw.filter_flag;
    flag = "--filter";
    if(option == NULL) {
        /* TODO */
    }

    return true;
invalid_option:
    fprintf(stderr, "invalid option for flag '%s': %s\n", flag, option);
    return false;
}

bool test_init(int argc, char **argv) {
    if (test_parse_arguments(argc, argv) == false) {
        return false;
    }

    if (test_parse_options() == false) {
        return false;
    }

    test_log_init();
    test_runner_reset();

    memset(&test_register, 0, sizeof(test_register));

    // Register all suits
    uint32_t suit_index = 0;
    for (test_intern_Suit *suit = _TEST_START_SUIT_SECTION; suit < _TEST_STOP_SUIT_SECTION;
         suit++) {
        test_register.suits = test_realloc(
            test_register.suits, sizeof(test_intern_Suit * [test_register.total_suits + 1]));

        test_register.suits[suit_index++] = suit;
        test_register.total_suits++;
    }

    test_intern_Suit *last_suit = NULL;
    test_intern_Suit *current_suit = NULL;

    // Register all cases
    for (test_intern_TestCase *test_case = _TEST_START_CASE_SECTION;
         test_case < _TEST_STOP_CASE_SECTION; test_case++) {
        // It is (probably) likely that adjacent tests belong to the same suit
        if ((current_suit == NULL) ||
            (last_suit != NULL && strcmp(last_suit->name, test_case->suit_name) != 0)) {
            current_suit = test_suit_find_by_name(test_case->suit_name);
        }

        if (current_suit == NULL) {
            fprintf(stderr, "Unable to find suit \"%s\" for test case \"%s\"\n",
                    test_case->suit_name, test_case->name);
            return false;
        }

        current_suit->tests = test_realloc(
            current_suit->tests, sizeof(test_intern_TestCase * [current_suit->total_tests + 1]));

        current_suit->tests[current_suit->total_tests] = test_case;
        current_suit->total_tests++;
        test_register.total_tests++;
        last_suit = current_suit;
    }

    return true;
}

void test_exit(void) {
    if(test_options.output_stream != NULL && test_options.raw.output_flag != NULL) {
        fclose(test_options.output_stream);
    }

    test_log_free();

    for (uint32_t i = 0; i < test_register.total_suits; i++) {
        free(test_register.suits[i]->tests);
    }

    free(test_register.suits);
}
