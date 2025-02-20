
CC := aarch64-linux-gnu-gcc    
SYS_INCLUDE_PATH :=  /usr/include/aarch64-linux-gnu
BUILD_FOLDER = build-arm64

CC := cc
SYS_INCLUDE_PATH :=  /usr/include
BUILD_FOLDER = build-x64

TEST := 1
ifeq ($(TEST), 1)
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -Llibs-x64 -lsdlfox -lvterm -lSDL2_image
	CFLAGS := -Wall -g `sdl2-config --cflags` -Iincludes/ -ggdb -fsanitize=address,undefined -DTEST
else
	LDLIBS := -Wl,--gc-sections `sdl2-config --libs` -lutil -Llibs -lsdlfox -lvterm -lSDL2_image
	CFLAGS := -Wall -O3 -flto `sdl2-config --cflags` -Iincludes/

endif

ifdef ${SCREENRC}
	DEF='-D"'${SCREENRC}'"'
else
	DEF='-DSCREENRC="/mnt/SDCARD/Apps/Terminal/.screenrc"'
endif

all:
	mkdir -p ./${BUILD_FOLDER}
	${CC} ${CFLAGS} ${DEF} ${LDFLAGS} src/*.c -o./${BUILD_FOLDER}/TermSP ${LDLIBS} -DSTDC_HEADERS -ldl -lm -I${SYS_INCLUDE_PATH} --prefix=/userdata/libs
	rsync -r fonts ./${BUILD_FOLDER}
	rsync -r res ./${BUILD_FOLDER}
	rsync -r libs-x64 ./${BUILD_FOLDER}
# make && LD_LIBRARY_PATH something -b -f

clean:
	rm -rv ./${BUILD_FOLDER}

install:
	sudo cp -v ./${BUILD_FOLDER}/TermSP /usr/local/bin
