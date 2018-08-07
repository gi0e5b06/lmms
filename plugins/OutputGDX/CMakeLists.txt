INCLUDE(BuildPlugin)

BUILD_PLUGIN(
  outputgdx
  OutputGDXControls.cpp  OutputGDX.cpp  OutputGDXDialog.cpp
  OutputGDXControls.h  OutputGDXDialog.h  OutputGDX.h
  MOCFILES
  OutputGDXControls.h  OutputGDXDialog.h  OutputGDX.h
  EMBEDDED_RESOURCES
  logo*.png
  )
