INCLUDE(BuildPlugin)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")

BUILD_PLUGIN(scoregdx
	ScoreGDX.cpp ScoreGDX.h
	ScoreGDXView.cpp ScoreGDXView.h
	MOCFILES
	ScoreGDX.h ScoreGDXView.h
	EMBEDDED_RESOURCES
	logo.png
)
