ifeq ($(TEST), 1)
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -Llibs -lsdlfox -lvterm
	CFLAGS := -Wall -g `sdl2-config --cflags` -Iincludes/ -ggdb -fsanitize=address,undefined
else
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -lutil -Llibs -lsdlfox -lvterm
	CFLAGS := -Wall -O3 -flto `sdl2-config --cflags` -Iincludes/

endif


all:
	mkdir -p ./build
	cc ${CFLAGS} ${LDFLAGS} src/*.c -o./build/TermSP ${LDLIBS}

clean:
	rm -rv ./build

install:
	sudo cp -v ./build/TermSP /usr/local/bin
