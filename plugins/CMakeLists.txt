SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Enable C++11
#ADD_DEFINITIONS(-std=c++0x)
#ADD_DEFINITIONS(-std=c++11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O9 -std=c++11")

IF(LMMS_BUILD_APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ENDIF()

INCLUDE_DIRECTORIES(
	${SAMPLERATE_INCLUDE_DIRS}
)

SET(PLUGIN_LIST "" CACHE STRING "List of plug-ins to build")

STRING(REPLACE " " ";" PLUGIN_LIST "${PLUGIN_LIST}")

OPTION(LMMS_MINIMAL "Build a minimal list of plug-ins" OFF)

SET(MINIMAL_LIST
	audio_file_processor
	kicker
	triple_oscillator
)

IF(LMMS_MINIMAL)
	IF("${PLUGIN_LIST}" STREQUAL "")
		STRING(REPLACE ";" " " MINIMAL_LIST_STRING "${MINIMAL_LIST}")
		MESSAGE(
"-- Using minimal plug-ins: ${MINIMAL_LIST_STRING}\n"
"   Note: You can specify specific plug-ins using -DPLUGIN_LIST=\"foo bar\""
		)
	ENDIF()
	SET(PLUGIN_LIST ${MINIMAL_LIST} ${PLUGIN_LIST})
ENDIF()

SET(LMMS_LV2_PLUGINS "")
IF(WANT_LV2)
  SET(LMMS_LV2_PLUGINS
    ${LMMS_LV2_PLUGINS}
    #Ilv2GDX
    #jalv
    LV2EffectGDX
    #LV2InstrumentGDX
    #LV2InstrumentGDXUI
    )
ENDIF(WANT_LV2)

IF("${PLUGIN_LIST}" STREQUAL "")
	SET(PLUGIN_LIST
		${MINIMAL_LIST}
                ${LMMS_LV2_PLUGINS}
		Amplifier
		AmplifierGDX
		BassBooster
		bit_invader
		Bitcrush
		carlabase
                carlaeffectbay
                carlaeffectrack
		carlapatchbay
		carlarack
                ChainGDX
		ClickGDX
                CompressorGDX
		CrossoverEQ
		# csound_instr - not ready
		Delay
		DualFilter
		dynamics_processor
		Eq
		Flanger
                FrequencyGDX
		GigPlayer
		HydrogenImport
		# ladspa_browser - not needed
		LadspaEffect
		lb302
                # MatrixGDX
		MidiImport
		MidiExport
		MultitapEcho
		monstro
		nes
		opl2
		organic
                OscilloscopeGDX
                OutputGDX
                PadsGDX
		papu
		patman
		peak_controller_effect
                RandomGDX
		ReverbSC
                ScarifierGDX
		ScatterGDX
		sf2_player
		sfxr
                ShaperGDX
		sid
                SmashGDX
                SpaceGDX
		SpectrumAnalyzer
                SplitGDX
		stereo_enhancer
		stereo_matrix
		stk
                SynthGDX
                VectorGDX
		vestige
		vst_base
		VstEffect
		watsyn
		waveshaper
		vibed
                VideoLineExport
                WallGDX
		xpressive
		zynaddsubfx
	)

ENDIF("${PLUGIN_LIST}" STREQUAL "")

FOREACH(PLUGIN ${PLUGIN_LIST})
	ADD_SUBDIRECTORY(${PLUGIN})
ENDFOREACH()
