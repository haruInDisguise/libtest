#include <test/test.h>

SUIT(testing, NULL, NULL);
TEST(testing, string) {
    test_assert_string_eq("what", "what");
    test_assert_string_ne("what", "not what");
}

TEST(testing, generic) {
    test_assert(1 == 1);
    test_assert(1 != 2);

    test_assert_eq(1, 1);
    test_assert_ne(1, 2);

    int value_one = 1;
    int value_two = 2;
    int value_three = 3;

    test_assert(value_one < value_two);
    test_assert(value_three > value_two);

    test_assert_eq(value_one, value_two - 1);
    test_assert_ne(value_three, value_two);
}

TEST(testing, memory) {
    int buffer_one[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int buffer_two[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int buffer_three[8] = {234, 53, 23465, 234};

    test_assert_memory_eq(buffer_one, buffer_two, sizeof(buffer_one));
    test_assert_memory_ne(buffer_one, buffer_three, sizeof(buffer_one));
}

SUIT(testing_false, NULL, NULL);
TEST(testing_false, one_eq_two) {
    test_assert(1 == 2);
}

TEST(testing_false, string_ne) {
    test_assert_string_eq("hello", "goodbye");
}
