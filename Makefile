CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

BUILD_DIR := build

DOMAIN_SRC := $(wildcard src/domain/*.cpp)
INFRA_SRC  := $(wildcard src/infrastructure/*.cpp)
APP_SRC    := src/main.cpp

DOMAIN_OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(DOMAIN_SRC))
INFRA_OBJ  := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(INFRA_SRC))
APP_OBJ    := $(BUILD_DIR)/main.o

TEST_SRC   := $(wildcard tests/*.cpp)
TEST_OBJ   := $(patsubst tests/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRC))

.PHONY: all run test clean

all: mlfq

mlfq: $(APP_OBJ) $(DOMAIN_OBJ) $(INFRA_OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

run: mlfq
	./mlfq

test: unittest
	./unittest

unittest: $(TEST_OBJ) $(DOMAIN_OBJ) $(INFRA_OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) mlfq unittest results.csv