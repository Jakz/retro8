.PHONY: all clean info

CXX := $(CROSS)g++
CC := $(CROSS)gcc

SYSROOT:= $(shell $(CXX) -print-sysroot)
CXXFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags)
LDFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --libs)
LDFLAGS += -lSDL2_image -L$(shell pwd) -flto

DISABLED_WARNINGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-reorder
CXXFLAGS += -g -O2 -W -Wall -Wextra -std=c++0x $(DISABLED_WARNINGS) -I../../src
CCFLAGS +=
# -O2

SRC_BASE := "../../src"
SRC_FOLDERS := $(SRC_BASE) $(SRC_BASE)/views $(SRC_BASE)/vm $(SRC_BASE)/io $(SRC_BASE)/lua
TEST := $(wildcard $(SRC_BASE)/views/*.cpp)
FULL_SOURCES := $(foreach dir, $(SRC_FOLDERS), $(wildcard $(dir)/*.cpp))
SOURCES := $(patsubst $(SRC_BASE)/%, %, $(foreach dir, $(SRC_FOLDERS), $(wildcard $(dir)/*.cpp)))
BINARIES:= $(foreach source, $(SOURCES), $(source:%.cpp=%.o) )

EXECUTABLE:= ./retro8

all: $(EXECUTABLE)

$(EXECUTABLE): $(BINARIES)
	$(CXX) $(BINARIES) -o $@ $(LDFLAGS)
	mkdir -p data

#.cpp.o:
#	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(EXECUTABLE)

info:
	@echo Source base: $(SRC_BASE)
	@echo Source subfolders: $(SRC_FOLDERS)
	@echo Source files: $(TEST)
	@echo Test: $(SRC_BASE)/views/*.cpp
