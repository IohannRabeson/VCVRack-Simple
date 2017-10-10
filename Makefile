
SOURCES = $(wildcard src/*.cpp)

include ../../plugin.mk


dist: all
	mkdir -p dist/Simple
	cp LICENSE* dist/Simple/
	cp plugin.* dist/Simple/
	cp -R res dist/Simple/
