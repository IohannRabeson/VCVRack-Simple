
SOURCES = $(wildcard src/*.cpp) \
		  utils/PulseGate.cpp

FLAGS += -I"."

include ../../plugin.mk

dist: all
	mkdir -p dist/VCVRack-Simple
	cp LICENSE* dist/VCVRack-Simple/
	cp plugin.* dist/VCVRack-Simple/
	cp -R res dist/VCVRack-Simple/
