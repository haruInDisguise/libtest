#include <test/test.h>

SUIT(assert, NULL, NULL);

TEST(assert, assert_eq) {
    int value_one = 5;
    test_assert_eq(value_one, 5);

    char *str_one = "what...";

    test_assert_str_eq("what...", str_one);
    test_assert_str_ne("no", str_one);
}

TEST(assert, something_strange) {
    char *not_so_amazing_string = "this message is not the same";

    test_assert_str_eq("what", not_so_amazing_string);
}
