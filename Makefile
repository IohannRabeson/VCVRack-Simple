SOURCES = src/Simple.cpp			\
		  src/ClockDivider.cpp		\
		  src/ButtonTrigger.cpp		\
		  src/Recorder.cpp			\
		  utils/LightControl.cpp	\
		  utils/PulseGate.cpp		\
		  utils/WavWriter.cpp		\
		  utils/StateMachine.cpp	\
		  utils/write_wav.c

include ../../plugin.mk

FLAGS += -I"./"

dist: all
	mkdir -p dist/Simple
	cp LICENSE* dist/Simple/
	cp plugin.* dist/Simple/
	cp -R res dist/Simple/
