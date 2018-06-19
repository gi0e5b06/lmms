if(LMMS_HAVE_CARLA)
        INCLUDE(BuildPlugin)
        INCLUDE_DIRECTORIES(${CARLA_INCLUDE_DIRS})
        LINK_DIRECTORIES(${CARLA_LIBRARY_DIRS})
        LINK_LIBRARIES(${CARLA_LIBRARIES})
        BUILD_PLUGIN(carlabase
          carla.cpp
          CarlaEffectControlDialog.cpp
          CarlaEffectControls.cpp
          CarlaEffect.cpp
          #CarlaEffectView.cpp
          MOCFILES
          carla.h
          CarlaEffectControlDialog.h
          CarlaEffectControls.h
          CarlaEffect.h
          #CarlaEffectView.h
          EMBEDDED_RESOURCES
          artwork-patchbay.png artwork-rack.png
          LINK SHARED)
        SET_TARGET_PROPERTIES(carlabase
                PROPERTIES SKIP_BUILD_RPATH TRUE
                BUILD_WITH_INSTALL_RPATH TRUE
                INSTALL_RPATH_USE_LINK_PATH TRUE
                INSTALL_RPATH "${CARLA_RPATH}")
endif(LMMS_HAVE_CARLA)
