.PHONY: all clean

CXX:= $(CROSS)g++

SYSROOT:= $(shell $(CXX) -print-sysroot)
CXXFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags)
LDFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --libs)
LDFLAGS += -lSDL2_image -L$(shell pwd) -llua

DISABLED_WARNINGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-reorder
CXXFLAGS += -g -W -Wall -Wextra -std=c++0x $(DISABLED_WARNINGS) -I../../src
# -O2

SOURCES := $(wildcard ../../src/*.cpp)
SOURCES += $(wildcard ../../src/views/*.cpp)
SOURCES += $(wildcard ../../src/vm/*.cpp)
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
