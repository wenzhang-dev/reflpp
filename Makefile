ENABLE_ASAN ?= 0

CXX ?= g++

CXXFLAGS = -std=c++20 -Wall -g
CXXFLAGS += -I .
CXXFLAGS += -I third-party/fmt/build/include

LDFLAGS = -lpthread
LDFLAGS += -L third-party/fmt/build/lib -lfmt

OPTIMIZATION = -O2

ifeq ($(ENABLE_ASAN), 1)
    CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
    LDFLAGS  += -fsanitize=address
    OPTIMIZATION = -O0
endif

CXXFLAGS += $(OPTIMIZATION)

SRC_DIR := examples
BUILD_DIR := build

SRCS = $(wildcard $(SRC_DIR)/*.cc)
BINS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%,$(SRCS))

all: prepare $(BINS)

prepare:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean prepare
