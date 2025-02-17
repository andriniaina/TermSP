ifeq ($(TEST), 1)
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -Llibs -lsdlfox -lvterm
	CFLAGS := -Wall -g `sdl2-config --cflags` -Iincludes/ -ggdb -fsanitize=address,undefined
else
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -lutil -Llibs -lsdlfox -lvterm
	CFLAGS := -Wall -O3 -flto `sdl2-config --cflags` -Iincludes/

endif

ifdef ${SCREENRC}
	DEF='-D"'${SCREENRC}'"'
else
	DEF='-DSCREENRC="/mnt/SDCARD/Apps/Terminal/.screenrc"'
endif

all:
	mkdir -p ./build
	aarch64-linux-gnu-gcc ${CFLAGS} ${DEF} ${LDFLAGS} src/*.c -o./build/TermSP ${LDLIBS} -DSTDC_HEADERS -I/usr/include/aarch64-linux-gnu --prefix=/userdata/libs

clean:
	rm -rv ./build

install:
	sudo cp -v ./build/TermSP /usr/local/bin
