# see: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html


TARGET = grbl
LIBS = -lwiringPi 
#-lpthread -lm -lrt -lGL -lGLU -lglut -lGLEW -lfreetype 

# add to libs for debugging gdb -v -da -Q
CC = gcc

# -g   for debugging
# -Wall -Werror   to show all errors
CFLAGS=  -g -Wall -Werror -Wextra
#-pthread  -I/usr/include/freetype2  -I/usr/include/libpng16 

#  -O0 did something for debugging
.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
