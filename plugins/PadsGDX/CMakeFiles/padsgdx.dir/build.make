# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_SOURCE_DIR = /home/desnoix/Devel/LMMS/lmms

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/desnoix/Devel/LMMS/lmms

# Include any dependencies generated for this target.
include plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend.make

# Include the progress variables for this target.
include plugins/PadsGDX/CMakeFiles/padsgdx.dir/progress.make

# Include the compile flags for this target's objects.
include plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make

plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/artwork.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/logo.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_off_off.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_off_on.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_on_off.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_on_on.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_pingpong_off.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/loop_pingpong_on.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/reverse_off.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/reverse_on.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/select_file.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/stutter_off.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/stutter_on.png
plugins/PadsGDX/qrc_padsgdx.cpp: plugins/PadsGDX/padsgdx.qrc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating qrc_padsgdx.cpp"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/lib/x86_64-linux-gnu/qt5/bin/rcc --name padsgdx --output /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/qrc_padsgdx.cpp /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/padsgdx.qrc

plugins/PadsGDX/moc_PadsGDX.cpp: plugins/PadsGDX/PadsGDX.h
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating moc_PadsGDX.cpp"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/lib/x86_64-linux-gnu/qt5/bin/moc @/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDX.cpp_parameters

plugins/PadsGDX/moc_PadsGDXView.cpp: plugins/PadsGDX/PadsGDXView.h
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating moc_PadsGDXView.cpp"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/lib/x86_64-linux-gnu/qt5/bin/moc @/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXView.cpp_parameters

plugins/PadsGDX/moc_PadsGDXWaveView.cpp: plugins/PadsGDX/PadsGDXWaveView.h
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Generating moc_PadsGDXWaveView.cpp"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/lib/x86_64-linux-gnu/qt5/bin/moc @/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXWaveView.cpp_parameters

plugins/PadsGDX/padsgdx.qrc: cmake/scripts/GenQrc.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Generating padsgdx.qrc"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/cmake -D OUT_FILE=/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/padsgdx.qrc -D RC_PREFIX=artwork/padsgdx -D "FILES:list=/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/artwork.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/logo.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_off_off.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_off_on.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_on_off.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_on_on.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_pingpong_off.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/loop_pingpong_on.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/reverse_off.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/reverse_on.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/select_file.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/stutter_off.png;/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/stutter_on.png;" -D DIR=/home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX -P /home/desnoix/Devel/LMMS/lmms/cmake/scripts/GenQrc.cmake

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o: plugins/PadsGDX/PadsGDX.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o: plugins/PadsGDX/qrc_padsgdx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/PadsGDX.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDX.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/PadsGDX.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDX.cpp > CMakeFiles/padsgdx.dir/PadsGDX.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/PadsGDX.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDX.cpp -o CMakeFiles/padsgdx.dir/PadsGDX.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o: plugins/PadsGDX/PadsGDXView.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o: plugins/PadsGDX/qrc_padsgdx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXView.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/PadsGDXView.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXView.cpp > CMakeFiles/padsgdx.dir/PadsGDXView.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/PadsGDXView.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXView.cpp -o CMakeFiles/padsgdx.dir/PadsGDXView.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o: plugins/PadsGDX/PadsGDXWaveView.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o: plugins/PadsGDX/qrc_padsgdx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXWaveView.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXWaveView.cpp > CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/PadsGDXWaveView.cpp -o CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o: plugins/PadsGDX/moc_PadsGDX.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDX.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDX.cpp > CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDX.cpp -o CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o: plugins/PadsGDX/moc_PadsGDXView.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXView.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXView.cpp > CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXView.cpp -o CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o: plugins/PadsGDX/moc_PadsGDXWaveView.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXWaveView.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXWaveView.cpp > CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/moc_PadsGDXWaveView.cpp -o CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o


plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o: plugins/PadsGDX/CMakeFiles/padsgdx.dir/flags.make
plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o: plugins/PadsGDX/qrc_padsgdx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o -c /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/qrc_padsgdx.cpp

plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.i"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/qrc_padsgdx.cpp > CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.i

plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.s"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/qrc_padsgdx.cpp -o CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.s

plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.requires:

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.provides: plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.requires
	$(MAKE) -f plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.provides.build
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.provides

plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.provides.build: plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o


# Object files for target padsgdx
padsgdx_OBJECTS = \
"CMakeFiles/padsgdx.dir/PadsGDX.cpp.o" \
"CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o" \
"CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o" \
"CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o"

# External object files for target padsgdx
padsgdx_EXTERNAL_OBJECTS =

plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make
plugins/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5.7.1
plugins/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Xml.so.5.7.1
plugins/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.7.1
plugins/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.7.1
plugins/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Linking CXX shared module ../libpadsgdx.so"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/padsgdx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugins/PadsGDX/CMakeFiles/padsgdx.dir/build: plugins/libpadsgdx.so

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/build

# Object files for target padsgdx
padsgdx_OBJECTS = \
"CMakeFiles/padsgdx.dir/PadsGDX.cpp.o" \
"CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o" \
"CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o" \
"CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o" \
"CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o"

# External object files for target padsgdx
padsgdx_EXTERNAL_OBJECTS =

plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/build.make
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5.7.1
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Xml.so.5.7.1
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.7.1
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.7.1
plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so: plugins/PadsGDX/CMakeFiles/padsgdx.dir/relink.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/desnoix/Devel/LMMS/lmms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Linking CXX shared module CMakeFiles/CMakeRelink.dir/libpadsgdx.so"
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/padsgdx.dir/relink.txt --verbose=$(VERBOSE)

# Rule to relink during preinstall.
plugins/PadsGDX/CMakeFiles/padsgdx.dir/preinstall: plugins/PadsGDX/CMakeFiles/CMakeRelink.dir/libpadsgdx.so

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/preinstall

plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDX.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXView.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/PadsGDXWaveView.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDX.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXView.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/moc_PadsGDXWaveView.cpp.o.requires
plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires: plugins/PadsGDX/CMakeFiles/padsgdx.dir/qrc_padsgdx.cpp.o.requires

.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/requires

plugins/PadsGDX/CMakeFiles/padsgdx.dir/clean:
	cd /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX && $(CMAKE_COMMAND) -P CMakeFiles/padsgdx.dir/cmake_clean.cmake
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/clean

plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend: plugins/PadsGDX/qrc_padsgdx.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend: plugins/PadsGDX/moc_PadsGDX.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend: plugins/PadsGDX/moc_PadsGDXView.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend: plugins/PadsGDX/moc_PadsGDXWaveView.cpp
plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend: plugins/PadsGDX/padsgdx.qrc
	cd /home/desnoix/Devel/LMMS/lmms && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/desnoix/Devel/LMMS/lmms /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX /home/desnoix/Devel/LMMS/lmms /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX /home/desnoix/Devel/LMMS/lmms/plugins/PadsGDX/CMakeFiles/padsgdx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugins/PadsGDX/CMakeFiles/padsgdx.dir/depend
