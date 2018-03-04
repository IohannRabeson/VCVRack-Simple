SLUG = IO-Simple
VERSION = 0.6.0dev

FLAGS += -I"."

RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

ifeq ($(ARCH), win)
LDFLAGS += /mingw64/lib/libsamplerate.a
endif

ifeq ($(ARCH), mac)
LDFLAGS += $(shell pkg-config --variable=libdir samplerate)/libsamplerate.a
endif

ifeq ($(ARCH), lin)
LDFLAGS += $(shell pkg-config --variable=libdir samplerate)/libsamplerate.a
endif

SOURCES += $(wildcard src/*.cpp utils/*.cpp utils/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) res

include $(RACK_DIR)/plugin.mk

serve:
	jekyll serve --watch --trace -s ./docs -d ./docs/_site
