INCLUDE(BuildPlugin)

BUILD_PLUGIN(microwave
  Microwave.cpp MicrowaveView.cpp MSynth.cpp MSynth.h
  MOCFILES Microwave.h MicrowaveView.h
  EMBEDDED_RESOURCES *.png)
