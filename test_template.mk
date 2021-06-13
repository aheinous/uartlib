.DELETE_ON_ERROR:

# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
# TARGET_EXEC :=
# TEST_NAME := ezp_msg_buffer_test
# MSG_TABLE := foobarpingpong.h

# TEST_NAME := ezp_msg_buffer_test
# MSG_TABLE := foobarpingpong.h

CC := gcc-11
CXX := g++-11

BUILD_ROOT := ./build
BUILD_DIR := $(BUILD_ROOT)/$(TEST_NAME)
SRC_DIRS := ./src
INC_DIRS := ./include ./tests/msg_tables ./tests/include
TEST_DIR := ./tests/src

TEST_EXEC := $(BUILD_DIR)/$(TEST_NAME)

CFLAGS := -Wall -g -fdiagnostics-color=always -O0
CXXFLAGS := -Wall -g -fdiagnostics-color=always -O0
CPPFLAGS := '-DEZP_MSG_TABLE="$(MSG_TABLE)"' -DEZP_DEBUG -DEZP_USR_LOG_FUNC -DEZP_USR_ERROR_FUNC

# OMIT := ./test/fakes/ezp_byte_buffer.c
# OMIT := $(shell echo $(OMIT)| xargs -n 1 basename)

# Find all the C and C++ files we want to compile
# SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c | grep -v -f <(echo $(OMIT)))
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c )


SRCS := $(shell ./prune.sh '$(SRCS)' '$(EXTRA_SRCS)')
SRCS += $(EXTRA_SRCS:%=./tests/%)


TEST_SRCS := $(TEST_DIR)/$(TEST_NAME).cpp
# MAIN_SRC := $(TEST_DIR)/catch_main.cpp
MAIN_SRC := $(MAIN:%=$(TEST_DIR)/%)



# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
MAIN_OBJ := $(MAIN_SRC:%=$(BUILD_ROOT)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)
DEPS += $(TEST_OBJS:.o=.d)
DEPS += $(MAIN_OBJ:.o=.d)


# Every folder in ./src will need to be passed to GCC so that it can find header files
# INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS += $(INC_FLAGS) -MMD -MP

# The final build step.
# $(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
# 	$(CXX) $(OBJS) -o $@ $(LDFLAGS)



$(TEST_EXEC): $(TEST_OBJS) $(OBJS) $(MAIN_OBJ)
	$(CXX) $(TEST_OBJS) $(OBJS) $(MAIN_OBJ) -o $@ $(LDFLAGS)


# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@$(CC) $(CPPFLAGS) $(CFLAGS) -E $< -o $@.i


# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@




.PHONY: clean print run

run: $(TEST_EXEC)
	$(TEST_EXEC)

print:
	@echo "objs:" $(OBJS) "\n"
	@echo "test objs:" $(TEST_OBJS) "\n"
	@echo "srcs:" $(SRCS) "\n"
	@echo "PRUNED_SRCS" $(PRUNED_SRCS) "\n"
	@echo "test_srcs:" $(TEST_SRCS) "\n"
	@echo "omit:" $(OMIT) "\n"

clean:
	@rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)