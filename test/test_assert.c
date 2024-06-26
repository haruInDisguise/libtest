#include <test/test.h>

SUIT(assert, NULL, NULL);

TEST(assert, assert_eq) {
    int value_one = 5;
    yak_test_assert_eq(value_one, 5);

    char *str_one = "what...";

    yak_test_assert_string_eq("what...", str_one);
    yak_test_assert_string_ne("no", str_one);
}

TEST(assert, something_strange) {
    char *not_so_amazing_string = "this message is not the same";

    yak_test_assert_string_eq("what", not_so_amazing_string);
}
