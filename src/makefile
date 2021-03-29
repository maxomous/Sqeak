# see: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html


TARGET = ../grbl
DIRS = gui gui/imgui
LIBS = -lwiringPi -lGL -lglfw -lGLEW
# ignore precompiled headers
IGNOREH = pch.hpp	
#-lpthread -lm -lrt -lGL -lGLU -lglut -lGLEW -lfreetype 

# add to libs for debugging gdb -v -da -Q
CC = g++

 
# -g   for debugging
# -Wall -Werror   to show all errors
CFLAGS= -g -Wall -Werror -Wextra
#-pthread  -I/usr/include/freetype2  -I/usr/include/libpng16 

# this ensures that if there is a file called default, all or clean, it will still be compiled
.PHONY: default all clean

default: $(TARGET)
all: default

# substitute '.cpp' with '.o' in any *.cpp 
OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp $(addsuffix /*.cpp, $(DIRS))))
HEADERS = $(filter-out $(IGNOREH), wildcard *.hpp)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# if make is interupted, dont delete any object file
.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
