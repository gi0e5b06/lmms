INCLUDE(BuildPlugin)

BUILD_PLUGIN(synthgdx
  SynthGDX.cpp SynthGDX.h
  SynthGDXView.cpp SynthGDXView.h
  ModulatorView.cpp ModulatorView.h
  OscillatorView.cpp OscillatorView.h
  MOCFILES
  SynthGDX.h
  SynthGDXView.h
  ModulatorView.h
  OscillatorView.h
  EMBEDDED_RESOURCES
  *.png)
