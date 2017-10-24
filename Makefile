SOURCES = src/Simple.cpp					\
		  src/ClockDivider.cpp				\
		  src/ButtonTrigger.cpp				\
		  src/Recorder.cpp					\
		  src/Clock.cpp						\
		  utils/LightControl.cpp			\
		  utils/PulseGate.cpp				\
		  utils/WavWriter.cpp				\
		  utils/StateMachine.cpp			\
		  utils/Path.cpp					\
		  utils/FourteenSegmentDisplay.cpp	\
		  utils/write_wav.c


FLAGS += -I"."

include ../../plugin.mk

ifeq ($(ARCH), lin)
	LDFLAGS += -L../../dep/lib -lglfw
endif

ifeq ($(ARCH), mac)
	LDFLAGS += -L../../dep/lib -lglfw
endif

ifeq ($(ARCH), win)
	LDFLAGS += -L../../dep/lib -lglfw3dll
endif

dist: all
	mkdir -p dist/VCVRack-Simple
	cp LICENSE* dist/VCVRack-Simple/
	cp plugin.* dist/VCVRack-Simple/
	cp -R res dist/VCVRack-Simple/
