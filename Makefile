SOURCES = src/Simple.cpp		\
		  src/ClockDivider.cpp	\
		  src/ButtonTrigger.cpp	\
		  src/Recorder.cpp		\
		  utils/PulseGate.cpp	\
		  utils/WavRecorder.cpp	\
		  utils/write_wav.c

include ../../plugin.mk

dist: all
	mkdir -p dist/Simple
	cp LICENSE* dist/Simple/
	cp plugin.* dist/Simple/
	cp -R res dist/Simple/
