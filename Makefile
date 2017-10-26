DIST_NAME=VCVRack-Simple

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
ifndef VERSION
	$(error VERSION must be defined when making distributables)
endif
	mkdir -p dist/$(DIST_NAME)
	cp LICENSE* dist/$(DIST_NAME)/
	cp $(TARGET) dist/$(DIST_NAME)/
	cp -R res dist/$(DIST_NAME)/
	cd dist && zip -5 -r $(DIST_NAME)-$(VERSION)-$(ARCH).zip $(DIST_NAME)
