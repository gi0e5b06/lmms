INCLUDE(BuildPlugin)

BUILD_PLUGIN(
  inputgdx
  InputGDXControls.cpp  InputGDX.cpp  InputGDXDialog.cpp
  InputGDXControls.h  InputGDXDialog.h  InputGDX.h
  MOCFILES
  InputGDXControls.h  InputGDXDialog.h  InputGDX.h
  EMBEDDED_RESOURCES
  logo*.png
  )
