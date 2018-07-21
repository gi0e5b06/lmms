IF(WANT_LV2)
  INCLUDE(BuildPlugin)

  # Disable C++11
  #REMOVE_DEFINITIONS(-std=c++0x)

  BUILD_PLUGIN(ilv2gdx EMBEDDED_RESOURCES artwork.png logo.png)
  #LV2InstrumentGDX.cpp LV2InstrumentGDXControls.cpp LV2InstrumentGDXControlDialog.cpp LV2InstrumentGDXSubPluginFeatures.cpp LV2InstrumentGDX.h LV2InstrumentGDXControls.h LV2InstrumentGDXControlDialog.h LV2InstrumentGDXSubPluginFeatures.h MOCFILES LV2InstrumentGDX.h LV2InstrumentGDXControls.h LV2InstrumentGDXControlDialog.h

  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ilv2gdx")

  TARGET_LINK_LIBRARIES(ilv2gdx -ljalv)
  ADD_DEPENDENCIES(ilv2gdx jalv)
ENDIF(WANT_LV2)
