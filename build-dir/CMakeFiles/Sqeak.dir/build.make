# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/Desktop/Projects/Sqeak/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/Desktop/Projects/Sqeak/build-dir

# Include any dependencies generated for this target.
include CMakeFiles/Sqeak.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Sqeak.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Sqeak.dir/flags.make

CMakeFiles/Sqeak.dir/common.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/common.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/common.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Sqeak.dir/common.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/common.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/common.cpp

CMakeFiles/Sqeak.dir/common.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/common.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/common.cpp > CMakeFiles/Sqeak.dir/common.cpp.i

CMakeFiles/Sqeak.dir/common.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/common.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/common.cpp -o CMakeFiles/Sqeak.dir/common.cpp.s

CMakeFiles/Sqeak.dir/gcreader.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gcreader.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gcreader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Sqeak.dir/gcreader.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gcreader.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gcreader.cpp

CMakeFiles/Sqeak.dir/gcreader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gcreader.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gcreader.cpp > CMakeFiles/Sqeak.dir/gcreader.cpp.i

CMakeFiles/Sqeak.dir/gcreader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gcreader.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gcreader.cpp -o CMakeFiles/Sqeak.dir/gcreader.cpp.s

CMakeFiles/Sqeak.dir/main.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/main.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Sqeak.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/main.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/main.cpp

CMakeFiles/Sqeak.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/main.cpp > CMakeFiles/Sqeak.dir/main.cpp.i

CMakeFiles/Sqeak.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/main.cpp -o CMakeFiles/Sqeak.dir/main.cpp.s

CMakeFiles/Sqeak.dir/settings.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/settings.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/settings.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/Sqeak.dir/settings.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/settings.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/settings.cpp

CMakeFiles/Sqeak.dir/settings.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/settings.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/settings.cpp > CMakeFiles/Sqeak.dir/settings.cpp.i

CMakeFiles/Sqeak.dir/settings.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/settings.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/settings.cpp -o CMakeFiles/Sqeak.dir/settings.cpp.s

CMakeFiles/Sqeak.dir/dev/joystick.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/dev/joystick.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/dev/joystick.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/Sqeak.dir/dev/joystick.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/dev/joystick.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/dev/joystick.cpp

CMakeFiles/Sqeak.dir/dev/joystick.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/dev/joystick.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/dev/joystick.cpp > CMakeFiles/Sqeak.dir/dev/joystick.cpp.i

CMakeFiles/Sqeak.dir/dev/joystick.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/dev/joystick.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/dev/joystick.cpp -o CMakeFiles/Sqeak.dir/dev/joystick.cpp.s

CMakeFiles/Sqeak.dir/libs/geos.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/libs/geos.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/libs/geos.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/Sqeak.dir/libs/geos.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/libs/geos.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/libs/geos.cpp

CMakeFiles/Sqeak.dir/libs/geos.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/libs/geos.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/libs/geos.cpp > CMakeFiles/Sqeak.dir/libs/geos.cpp.i

CMakeFiles/Sqeak.dir/libs/geos.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/libs/geos.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/libs/geos.cpp -o CMakeFiles/Sqeak.dir/libs/geos.cpp.s

CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/libs/qrcodegen.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/libs/qrcodegen.cpp

CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/libs/qrcodegen.cpp > CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.i

CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/libs/qrcodegen.cpp -o CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.s

CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gui/filebrowser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gui/filebrowser.cpp

CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gui/filebrowser.cpp > CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.i

CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gui/filebrowser.cpp -o CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.s

CMakeFiles/Sqeak.dir/gui/frames.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gui/frames.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gui/frames.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/Sqeak.dir/gui/frames.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gui/frames.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gui/frames.cpp

CMakeFiles/Sqeak.dir/gui/frames.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gui/frames.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gui/frames.cpp > CMakeFiles/Sqeak.dir/gui/frames.cpp.i

CMakeFiles/Sqeak.dir/gui/frames.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gui/frames.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gui/frames.cpp -o CMakeFiles/Sqeak.dir/gui/frames.cpp.s

CMakeFiles/Sqeak.dir/gui/gui.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gui/gui.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gui/gui.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/Sqeak.dir/gui/gui.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gui/gui.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gui/gui.cpp

CMakeFiles/Sqeak.dir/gui/gui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gui/gui.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gui/gui.cpp > CMakeFiles/Sqeak.dir/gui/gui.cpp.i

CMakeFiles/Sqeak.dir/gui/gui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gui/gui.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gui/gui.cpp -o CMakeFiles/Sqeak.dir/gui/gui.cpp.s

CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gui/imgui_custommodules.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gui/imgui_custommodules.cpp

CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gui/imgui_custommodules.cpp > CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.i

CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gui/imgui_custommodules.cpp -o CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.s

CMakeFiles/Sqeak.dir/gui/viewer.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/gui/viewer.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/gui/viewer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/Sqeak.dir/gui/viewer.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/gui/viewer.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/gui/viewer.cpp

CMakeFiles/Sqeak.dir/gui/viewer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/gui/viewer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/gui/viewer.cpp > CMakeFiles/Sqeak.dir/gui/viewer.cpp.i

CMakeFiles/Sqeak.dir/gui/viewer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/gui/viewer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/gui/viewer.cpp -o CMakeFiles/Sqeak.dir/gui/viewer.cpp.s

CMakeFiles/Sqeak.dir/functions/functions.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/functions/functions.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/functions/functions.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/Sqeak.dir/functions/functions.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/functions/functions.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/functions/functions.cpp

CMakeFiles/Sqeak.dir/functions/functions.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/functions/functions.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/functions/functions.cpp > CMakeFiles/Sqeak.dir/functions/functions.cpp.i

CMakeFiles/Sqeak.dir/functions/functions.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/functions/functions.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/functions/functions.cpp -o CMakeFiles/Sqeak.dir/functions/functions.cpp.s

CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/sketch/gcodebuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/sketch/gcodebuilder.cpp

CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/sketch/gcodebuilder.cpp > CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.i

CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/sketch/gcodebuilder.cpp -o CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.s

CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/sketch/sketch.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building CXX object CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/sketch/sketch.cpp

CMakeFiles/Sqeak.dir/sketch/sketch.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/sketch/sketch.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/sketch/sketch.cpp > CMakeFiles/Sqeak.dir/sketch/sketch.cpp.i

CMakeFiles/Sqeak.dir/sketch/sketch.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/sketch/sketch.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/sketch/sketch.cpp -o CMakeFiles/Sqeak.dir/sketch/sketch.cpp.s

CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o: CMakeFiles/Sqeak.dir/flags.make
CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o: /home/pi/Desktop/Projects/Sqeak/src/sketch/toolsettings.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building CXX object CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o -c /home/pi/Desktop/Projects/Sqeak/src/sketch/toolsettings.cpp

CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Desktop/Projects/Sqeak/src/sketch/toolsettings.cpp > CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.i

CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Desktop/Projects/Sqeak/src/sketch/toolsettings.cpp -o CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.s

# Object files for target Sqeak
Sqeak_OBJECTS = \
"CMakeFiles/Sqeak.dir/common.cpp.o" \
"CMakeFiles/Sqeak.dir/gcreader.cpp.o" \
"CMakeFiles/Sqeak.dir/main.cpp.o" \
"CMakeFiles/Sqeak.dir/settings.cpp.o" \
"CMakeFiles/Sqeak.dir/dev/joystick.cpp.o" \
"CMakeFiles/Sqeak.dir/libs/geos.cpp.o" \
"CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o" \
"CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o" \
"CMakeFiles/Sqeak.dir/gui/frames.cpp.o" \
"CMakeFiles/Sqeak.dir/gui/gui.cpp.o" \
"CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o" \
"CMakeFiles/Sqeak.dir/gui/viewer.cpp.o" \
"CMakeFiles/Sqeak.dir/functions/functions.cpp.o" \
"CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o" \
"CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o" \
"CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o"

# External object files for target Sqeak
Sqeak_EXTERNAL_OBJECTS =

/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/common.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gcreader.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/main.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/settings.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/dev/joystick.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/libs/geos.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/libs/qrcodegen.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gui/filebrowser.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gui/frames.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gui/gui.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gui/imgui_custommodules.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/gui/viewer.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/functions/functions.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/sketch/gcodebuilder.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/sketch/sketch.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/sketch/toolsettings.cpp.o
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/build.make
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/local/lib/libgeos_c.so
/home/pi/Desktop/Projects/Sqeak/Sqeak: glcore/libGLCore.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: sketcher/libSketcher.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: grbl/libGRBL.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/local/lib/libgeos_c.so
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/lib/arm-linux-gnueabihf/libGL.so
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/lib/arm-linux-gnueabihf/libGLEW.so
/home/pi/Desktop/Projects/Sqeak/Sqeak: glcore/deps/imgui/libImGui.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/lib/arm-linux-gnueabihf/libglfw.so.3.2
/home/pi/Desktop/Projects/Sqeak/Sqeak: sketcher/deps/constraintsolver/libConstraintSolver.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: sketcher/deps/constraintsolver/libslvs/liblibslvs.a
/home/pi/Desktop/Projects/Sqeak/Sqeak: /usr/lib/libwiringPi.so
/home/pi/Desktop/Projects/Sqeak/Sqeak: CMakeFiles/Sqeak.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Linking CXX executable /home/pi/Desktop/Projects/Sqeak/Sqeak"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Sqeak.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Sqeak.dir/build: /home/pi/Desktop/Projects/Sqeak/Sqeak

.PHONY : CMakeFiles/Sqeak.dir/build

CMakeFiles/Sqeak.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Sqeak.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Sqeak.dir/clean

CMakeFiles/Sqeak.dir/depend:
	cd /home/pi/Desktop/Projects/Sqeak/build-dir && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/Desktop/Projects/Sqeak/src /home/pi/Desktop/Projects/Sqeak/src /home/pi/Desktop/Projects/Sqeak/build-dir /home/pi/Desktop/Projects/Sqeak/build-dir /home/pi/Desktop/Projects/Sqeak/build-dir/CMakeFiles/Sqeak.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Sqeak.dir/depend

