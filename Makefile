CC = clang

OPTION_BUILD_DEBUG ?= true
OPTION_BUILD_ASAN ?= true

BUILD_PATH = build

CFLAGS = -Wall -Wextra -Wconversion -Wno-sign-conversion -Wno-unused-function -Wpedantic \
		 -Iinclude
LFLAGS =

ifeq ($(OPTION_BUILD_ASAN), true)
CFLAGS += -fsanitize=address,undefined
LFLAGS += -fsanitize=address,undefined
endif

ifeq ($(OPTION_BUILD_DEBUG), true)
CFLAGS += -O0 -g3 -DTEST_DEBUG
else
CFLAGS += -O2
endif

TEST_SRC = test/main.c test/test_assert.c
TEST_OBJ := $(patsubst %.c,$(BUILD_PATH)/%.o,$(TEST_SRC))

test: $(TEST_OBJ)
	$(CC) $(LFLAGS) $(TEST_OBJ) -o build/main

dev:: clean
	@mkdir -p $(BUILD_PATH)
	bear --output $(BUILD_PATH)/compile_commands.json -- $(MAKE)

clean:
	rm -rf build

.PHONY: clean test dev
.SUFFIXES:

$(TEST_OBJ): $(BUILD_PATH)/%.o: %.c include/test/test.h
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@
