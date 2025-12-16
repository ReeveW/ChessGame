# =========================
# Project settings
# =========================
PROJECT := chess

CXX := g++
STD := -std=c++17

WARN := -Wall -Wextra -pedantic
INCLUDES := -Iinclude

# Build modes:
#   make            -> release
#   make debug      -> debug
MODE ?= release

ifeq ($(MODE),debug)
  OPT := -O0 -g
  DEFINES := -DDEBUG
else
  OPT := -O2
  DEFINES :=
endif

CXXFLAGS := $(STD) $(WARN) $(OPT) $(DEFINES) $(INCLUDES)

# =========================
# SFML 2.6.1 linking
# (same link flags; version is handled by your installed SFML)
# =========================
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system

# =========================
# GoogleTest linking
# Option A (system-installed gtest):
#   sudo apt install libgtest-dev cmake
#   (on Ubuntu, libgtest-dev may require building libs; see notes below)
# =========================
GTEST_LIBS := -lgtest -lgtest_main -pthread

# =========================
# Directories
# =========================
SRC_DIR := src
INC_DIR := include
TEST_DIR := tests

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep

BIN_DIR := $(BUILD_DIR)/bin
TEST_BIN_DIR := $(BUILD_DIR)/testbin

# =========================
# Sources
# =========================
APP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
APP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/app/%.o,$(APP_SRCS))
APP_DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/app/%.d,$(APP_SRCS))

# For tests: compile tests/*.cpp + compile your src/*.cpp EXCEPT main.cpp
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
LIB_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(APP_SRCS))

TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/test/%.o,$(TEST_SRCS)) \
             $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/lib/%.o,$(LIB_SRCS))
TEST_DEPS := $(patsubst $(TEST_DIR)/%.cpp,$(DEP_DIR)/test/%.d,$(TEST_SRCS)) \
             $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/lib/%.d,$(LIB_SRCS))

APP_BIN := $(BIN_DIR)/$(PROJECT)
TEST_BIN := $(TEST_BIN_DIR)/$(PROJECT)_tests

# Auto-dependency generation
DEPFLAGS := -MMD -MP

# =========================
# Default targets
# =========================
.PHONY: all debug release run test clean valgrind memcheck helgrind

all: release

release:
	@$(MAKE) MODE=release $(APP_BIN)

debug:
	@$(MAKE) MODE=debug $(APP_BIN)

run: $(APP_BIN)
	$(APP_BIN)

# =========================
# Build app
# =========================
$(APP_BIN): $(APP_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SFML_LIBS)

$(OBJ_DIR)/app/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/app $(DEP_DIR)/app
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@
	@mv $(OBJ_DIR)/app/$*.d $(DEP_DIR)/app/$*.d 2>/dev/null || true

# =========================
# Build tests
# =========================
test: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_LIBS)

$(OBJ_DIR)/test/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/test $(DEP_DIR)/test
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@
	@mv $(OBJ_DIR)/test/$*.d $(DEP_DIR)/test/$*.d 2>/dev/null || true

$(OBJ_DIR)/lib/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/lib $(DEP_DIR)/lib
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@
	@mv $(OBJ_DIR)/lib/$*.d $(DEP_DIR)/lib/$*.d 2>/dev/null || true

# =========================
# Valgrind targets
# =========================
valgrind: $(TEST_BIN)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes $(TEST_BIN)

memcheck: $(TEST_BIN)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes $(TEST_BIN)

helgrind: $(TEST_BIN)
	valgrind --tool=helgrind $(TEST_BIN)

# =========================
# Clean
# =========================
clean:
	rm -rf $(BUILD_DIR)

# =========================
# Include auto-generated dependencies
# =========================
-include $(APP_DEPS)
-include $(TEST_DEPS)
