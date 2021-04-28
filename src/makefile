# see: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html


TARGET = ../grbl
DIRS = dev libs gui gui/imgui gui/stb_image gui/fonts 
LDLIBS = -lwiringPi -lGL -lglfw -lGLEW -lpthread
# -lm -lrt -lGLU -lglut -lfreetype 

# add to libs for debugging gdb -v -da -Q
CXX = g++

# -g   for debugging
# -Wall -Werror   to show all errors
CXXFLAGS= -std=c++17 -g -Wall -Werror -Wextra
#-pthread  -I/usr/include/freetype2  -I/usr/include/libpng16 

# this ensures that if there is a file called default, all or clean, it will still be compiled
.PHONY: default all clean

default: $(TARGET)
all: default

# substitute '.cpp' with '.o' in any *.cpp 
OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp $(addsuffix /*.cpp, $(DIRS))))
HEADERS = $(wildcard *.h)

# build the executable
%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
# if make is interupted, dont delete any object file
.PRECIOUS: $(TARGET) $(OBJECTS)

# build the objects
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -Wall $(LDLIBS) -o $@ 

clean:
	-rm -f *.o $(addsuffix /*.o, $(DIRS))
	-rm -f $(TARGET)
