INCLUDE_PATHS := -I/usr/include/SDL2 -Ilibvterm-0.2/include
LIBRARY_PATHS := -Llibvterm-0.3.3/.libs -L/root/workspace/TRIMUI-LIBS 
LINK := -Wl,--gc-sections -lsdlfox -lvterm -lutil -lSDL2 
CFLAGS := ${INCLUDE_PATHS} ${LIBRARY_PATHS} ${LINK}

all:
	mkdir -p ./build
	cc -Wall -O3 -flto -march=armv8-a+crc+simd -mtune=cortex-a53 ./src/*.c -o./build/TermSP ${CFLAGS}
clean:
	rm -rv ./build

install:
	sudo cp -v ./build/TermSP /usr/local/bin
