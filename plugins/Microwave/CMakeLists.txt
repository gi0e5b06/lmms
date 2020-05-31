INCLUDE(BuildPlugin)

BUILD_PLUGIN(microwave Microwave.cpp Microwave.h MOCFILES Microwave.h EMBEDDED_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.png")

