INCLUDE(BuildPlugin)

BUILD_PLUGIN(compressor Compressor.cpp CompressorControls.cpp CompressorControlDialog.cpp MOCFILES Compressor.h CompressorControls.h CompressorControlDialog.h ../Eq/EqFader.h EMBEDDED_RESOURCES *.png)
