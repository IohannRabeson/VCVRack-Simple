SOURCES = src/Simple.cpp			\
		  src/ClockDivider.cpp		\
		  src/ButtonTrigger.cpp		\
		  src/Recorder.cpp			\
		  utils/LightControl.cpp	\
		  utils/PulseGate.cpp		\
		  utils/WavWriter.cpp		\
		  utils/StateMachine.cpp	\
		  utils/write_wav.c

FLAGS += -I"."

include ../../plugin.mk

dist: all
	mkdir -p dist/VCVRack-Simple
	cp LICENSE* dist/VCVRack-Simple/
	cp plugin.* dist/VCVRack-Simple/
	cp -R res dist/VCVRack-Simple/
