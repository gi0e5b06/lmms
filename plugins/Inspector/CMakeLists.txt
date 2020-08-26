INCLUDE(BuildPlugin)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")

BUILD_PLUGIN(inspector
	Inspector.cpp Inspector.h
	InspectorView.cpp InspectorView.h
	MOCFILES
	Inspector.h InspectorView.h
	EMBEDDED_RESOURCES
	logo.png
)
