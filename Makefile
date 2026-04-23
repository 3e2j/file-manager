CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -pedantic -fPIC -Iinclude $(shell pkg-config --cflags Qt6Widgets)
LDFLAGS := $(shell pkg-config --libs Qt6Widgets)
TARGET := build/file-manager
SOURCES := \
	file-manager.cpp \
	src/app/app_controller.cpp \
	src/core/file_entry.cpp \
	src/core/file_system.cpp \
	src/ui/ui.cpp

.PHONY: all run clean

all: $(TARGET)

build:
	@mkdir -p build

$(TARGET): $(SOURCES) | build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -rf build
