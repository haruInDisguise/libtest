#include <test/test.h>

SUIT(assert, NULL, NULL);

TEST(assert, assert_eq) {
    int value_one = 5;
    do {
        if (!((value_one) == (5))) {
            _test_info->assertion_info.line = 7;
            _test_info->assertion_info.macro_name = "assert_eq";
            _test_info->assertion_info.macro_value_lhs = "value_one";
            _test_info->assertion_info.macro_value_rhs = "5";
            _test_info->result = test_intern_ResultFailed;
            test_intern_log_assertion_failed(_test_info);
        }
    } while (0);

    char *str_one = "what...";

    test_assert_string_eq("what...", str_one);
    test_assert_string_ne("no", str_one);
}

TEST(assert, something_strange) {
    char *not_so_amazing_string = "this message is not the same";

    test_assert_string_eq("what", not_so_amazing_string);
}
