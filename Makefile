SLUG = IO-Simple
VERSION = 0.6.0dev

FLAGS += -I"."

RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

SOURCES += $(wildcard src/*.cpp utils/*.cpp utils/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) res

# Static libs
libsamplerate := dep/lib/libsamplerate.a
OBJECTS += $(libsamplerate)

# Dependencies
$(shell mkdir -p dep)
DEP_LOCAL := dep
DEPS += $(libsamplerate)

$(libsamplerate):
	cd dep && $(WGET) http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz
	cd dep && $(UNTAR) libsamplerate-0.1.9.tar.gz
	cd dep/libsamplerate-0.1.9 && $(CONFIGURE)
	cd dep/libsamplerate-0.1.9 && $(MAKE)
	cd dep/libsamplerate-0.1.9 && $(MAKE) install


include $(RACK_DIR)/plugin.mk

serve:
	jekyll serve --watch --trace -s ./docs -d ./docs/_site
