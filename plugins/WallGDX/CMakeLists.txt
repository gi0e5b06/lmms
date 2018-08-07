INCLUDE(BuildPlugin)

BUILD_PLUGIN(wallgdx WallGDX.cpp WallGDXControls.cpp WallGDXDialog.cpp MOCFILES WallGDXControls.h WallGDXDialog.h EMBEDDED_RESOURCES logo.png)
