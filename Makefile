CC = g++
TARGET = assign3part2
SOURCES = $(shell echo ./src/*.cpp)
OBJS = $(subst ./src/,./build/,$(SOURCES:.cpp=.o))
WARN = -Wall -Wextra
CPPFLAGS = $(WARN) -std=c++11
UNAME = $(shell uname)
# Linux
ifeq ($(UNAME),Linux)
	LIB = `pkg-config --static --libs glfw3` -lGLEW -lGL
endif
# MacOS
ifeq ($(UNAME),Darwin)
	LIB = `pkg-config --static --libs glfw3` -lGLEW -framework OpenGL
endif
INC = -I include -I external_files

all: $(TARGET)

LoadShaders.o: external_files/LoadShaders.cpp
	$(CC) $(CPPFLAGS) $(INC) -c -o build/LoadShaders.o $<

$(TARGET): $(OBJS) LoadShaders.o
	$(CC) $(CPPFLAGS) $(INC) -o $(TARGET) $(OBJS) build/LoadShaders.o $(LIB)

build/%.o: src/%.cpp src/%.hpp
	mkdir -p build
	$(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

.PHONY: all clean refresh

clean:
	rm -rf build/* $(TARGET)

refresh: clean all

print:
	@echo $(INC)
	@echo "TARGET   := $(TARGET)"
	@echo "SOURCES  := $(SOURCES)"
	@echo "OBJS     := $(OBJS)"
