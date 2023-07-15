#include "../src/test.h"

SUIT(check_suit, NULL, NULL);

TEST(check_suit, check_eq) {
    test_check_eq(1, 1);
    test_check_ne(2, 5);

    char *str_one = "hello";
    test_check_str_ne("hello", str_one);
    test_check_str_eq("goodbye", str_one);
}

TEST(check_suit, what) {

}
