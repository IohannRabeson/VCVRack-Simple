SLUG = IO-Simple
VERSION = 0.6.0dev

FLAGS += -I"."
LDFLAGS += -lsamplerate

SOURCES += $(wildcard src/*.cpp utils/*.cpp utils/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

serve:
	jekyll serve --watch --trace -s ./docs -d ./docs/_site
