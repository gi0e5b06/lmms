IF(LMMS_SUPPORT_LV2)

  INCLUDE(BuildPlugin)

#REMOVE_DEFINITIONS(-std=c++0x)

INCLUDE_DIRECTORIES(
	${LILV_INCLUDE_DIRS}
)

BUILD_PLUGIN(lv2effectgdx LV2EffectGDX.cpp LV2EffectGDXControls.cpp LV2EffectGDXControlDialog.cpp LV2EffectGDXSubPluginFeatures.cpp LV2EffectGDX.h LV2EffectGDXControls.h LV2EffectGDXControlDialog.h LV2EffectGDXSubPluginFeatures.h MOCFILES LV2EffectGDX.h LV2EffectGDXControls.h LV2EffectGDXControlDialog.h EMBEDDED_RESOURCES logo.png)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lv2effectgdx")

#IF(WANT_CAPS)
#ADD_SUBDIRECTORY(caps)
#ENDIF(WANT_CAPS)

ENDIF()
