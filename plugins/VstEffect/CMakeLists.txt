IF(LMMS_SUPPORT_VST)

INCLUDE(BuildPlugin)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../vst_base")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/..")

IF(LMMS_BUILD_LINUX)
	LINK_LIBRARIES(-Wl,--enable-new-dtags)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "$ORIGIN")
ELSE(LMMS_BUILD_LINUX)
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${PLUGIN_DIR}")
ENDIF(LMMS_BUILD_LINUX)

BUILD_PLUGIN(vsteffect VstEffect.cpp VstEffectControls.cpp VstEffectControlDialog.cpp VstSubPluginFeatures.cpp VstEffect.h VstEffectControls.h VstEffectControlDialog.h VstSubPluginFeatures.h MOCFILES VstEffectControlDialog.h VstEffectControls.h EMBEDDED_RESOURCES artwork.png logo.png controls.png controls_active.png zoom.png)
SET_TARGET_PROPERTIES(vsteffect PROPERTIES COMPILE_FLAGS "-Wno-attributes")
TARGET_LINK_LIBRARIES(vsteffect -lvstbase)
ADD_DEPENDENCIES(vsteffect vstbase)

ENDIF(LMMS_SUPPORT_VST)

