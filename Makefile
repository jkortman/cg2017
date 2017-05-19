CC = g++
TARGET = cg2017
SOURCES = $(shell echo ./src/*.cpp)
OBJS = $(subst ./src/,./build/,$(SOURCES:.cpp=.o))
CPPFLAGS = -Wall -std=c++11
# Linux
#LIB = `pkg-config --static --libs glfw3` -lGLEW -lGL
# MacOS
LIB = `pkg-config --static --libs glfw3` -lGLEW -framework OpenGL
INC = -I include

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $(INC) -o $(TARGET) $(OBJS) $(LIB)

build/%.o: src/%.cpp
	$(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

.PHONY: all clean refresh

clean:
	rm -r build/*

refresh: clean all

print:
	@echo $(INC)
	@echo "TARGET   := $(TARGET)"
	@echo "SOURCES  := $(SOURCES)"
	@echo "OBJS     := $(OBJS)"
