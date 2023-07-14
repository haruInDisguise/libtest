#include "../src/test.h"

SUIT(assert_suit, NULL, NULL);

TEST(assert_suit, assert_eq) {
    uint32_t value_one = 83;
    test_assert_eq(83, value_one);
    test_assert_eq(value_one, 5);

    char *str_one = "what...";

    test_assert_str_eq("what...", str_one);
    test_assert_str_ne("no", str_one);
}

#define WOA_DUDE(x) #x

TEST(assert_suit, something_strange) {
    char *amazing_string = "totally awesome comparison";
    char *not_so_amazing_string = "this message is not the same";

    test_assert_str_eq(WOA_DUDE(no), not_so_amazing_string);
}
