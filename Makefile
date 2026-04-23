CXX := g++
BASE_CXXFLAGS := -std=c++23 -Wall -Wextra -pedantic -fPIC -Iinclude -Werror
APP_CXXFLAGS := $(BASE_CXXFLAGS) $(shell pkg-config --cflags Qt6Widgets)
APP_LDFLAGS := $(shell pkg-config --libs Qt6Widgets)

BUILD_DIR := build
APP_TARGET := $(BUILD_DIR)/file-manager
TEST_TARGET := $(BUILD_DIR)/file-manager-tests

CORE_SOURCES := \
	src/core/file_entry.cpp \
	src/core/file_system.cpp
APP_SOURCES := \
	file-manager.cpp \
	src/app/app_controller.cpp \
	$(CORE_SOURCES) \
	src/ui/ui.cpp

TEST_MAIN := tests/test_main.cpp
TEST_CASE_SOURCES := $(sort $(wildcard tests/core/test_*.cpp))

.PHONY: all run test clean

all: $(APP_TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(APP_TARGET): $(APP_SOURCES) | $(BUILD_DIR)
	$(CXX) $(APP_CXXFLAGS) $(APP_SOURCES) -o $@ $(APP_LDFLAGS)

run: all
	./$(APP_TARGET)

$(TEST_TARGET): $(TEST_MAIN) $(TEST_CASE_SOURCES) $(CORE_SOURCES) | $(BUILD_DIR)
	$(CXX) $(BASE_CXXFLAGS) $^ -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)
