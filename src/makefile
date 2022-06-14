# see: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html


TARGET = ../Sqeak

DIRS = \
sketch \
sketch/functions \
dev \
libs \
gui \
glcore \
glcore/deps/imgui \
glcore/deps/stb_image \
glcore/deps/tinyobjloader

BASIC_CLEAN_DIRS = \
sketch \
sketch/functions \
gui \


LDLIBS = -lwiringPi -ldl -lGL -lglfw -lGLEW -lpthread -lgeos_c

CXX = g++

CXXFLAGS= -std=c++17 -g -Wall -Werror -Wextra

# this ensures that if there is a file called default, all or clean, it will still be compiled
.PHONY: default all clean

default: $(TARGET)
all: default

# substitute '.cpp' with '.o' in any *.cpp 
OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp $(addsuffix /*.cpp, $(DIRS))))

# build the executable
%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
# if make is interupted, dont delete any object file
.PRECIOUS: $(TARGET) $(OBJECTS)

# build the objects
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -Wall $(LDLIBS) -o $@ 

clean_basic:
	-rm -f *.o $(addsuffix /*.o, $(BASIC_CLEAN_DIRS))
	-rm -f $(TARGET)
 
clean_full:
	-rm -f *.o $(addsuffix /*.o, $(DIRS))
	-rm -f $(TARGET)
 
