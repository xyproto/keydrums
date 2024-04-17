.PHONY: clean distclean run

all: keydrums musicradar-drum-samples

musicradar-drum-samples:
	curl -OL 'http://cdn.mos.musicradar.com/audio/samples/musicradar-drum-samples.zip'
	unzip musicradar-drum-samples.zip
	rm musicradar-drum-samples.zip

keydrums.o: main.cpp
	g++ -o $@ -c -std=c++2a -O2 -pipe -fPIC -fno-plt -fstack-protector-strong -Wall -Wshadow -Wpedantic -Wno-parentheses -Wfatal-errors -Wignored-qualifiers -Wvla -pthread -I/usr/include/SDL2 -D_REENTRANT `pkg-config --cflags sdl2` $<

keydrums: keydrums.o
	g++ -o $@ $< `pkg-config --libs sdl2` -lSDL2_image -lSDL2_mixer

run: keydrums musicradar-drum-samples
	./keydrums

clean:
	rm -f keydrums.o *.zip

distclean: clean
	rm -f keydrums musicradar-drum-samples
