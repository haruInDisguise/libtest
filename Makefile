CC = clang
AR = ar

OPTION_BUILD_TESTS ?= true
OPTION_BUILD_DEBUG ?= false
OPTION_BUILD_ASAN ?= false

LIB_NAME = libtest.a

BUILD_PATH = build

CFLAGS = -Wall -Wextra -Wconversion -Wno-sign-conversion -Wno-unused-function -pedantic \
		 -Iinclude
LFLAGS =

ifeq ($(OPTION_BUILD_ASAN), true)
CFLAGS += -fsanitize=address,undefined
LFLAGS += -fsanitize=address,undefined
endif

ifeq ($(OPTION_BUILD_DEBUG), true)
CFLAGS += -O0 -g3 -DTEST_DEBUG
else
CFLAGS += -O2 -DNDEBUG
endif

LIB_SRC = src/test.c
LIB_OBJ := $(patsubst %.c,$(BUILD_PATH)/%.o,$(LIB_SRC))

TEST_SRC = test/main.c test/test_assert.c test/test_check.c
TEST_OBJ := $(patsubst %.c,$(BUILD_PATH)/%.o,$(TEST_SRC))

ifeq ($(OPTION_BUILD_TESTS), true)
all: test
else
all: build
endif

build: $(LIB_OBJ) include/test/test.h
	$(AR) rcs "$(BUILD_PATH)/$(LIB_NAME)" $(LIB_OBJ)

test: build $(TEST_OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(TEST_OBJ) $(BUILD_PATH)/$(LIB_NAME) -o build/main

dev: clean
	@mkdir -p $(BUILD_PATH)
	bear --output $(BUILD_PATH)/compile_commands.json -- make

clean:
	rm -rf build

.PHONY: clean all build test dev

$(LIB_OBJ) $(TEST_OBJ): $(BUILD_PATH)/%.o: %.c
	echo $(TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@
