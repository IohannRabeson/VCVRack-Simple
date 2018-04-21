SLUG = IO-Simple
VERSION = 0.6.0

FLAGS += -I"."

RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

SOURCES += $(wildcard src/*.cpp utils/*.cpp utils/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) res

include $(RACK_DIR)/plugin.mk

serve:
	jekyll serve --watch --trace -s ./docs -d ./docs/_site
