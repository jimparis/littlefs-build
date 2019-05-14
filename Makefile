all: build

test: build
	./build test1.wav test2.wav

build: build.o lfs.o lfs_util.o

lfs.o: lfs.h lfs_util.h

lfs_util.o: lfs_util.h

build.o: lfs.h lfs_util.h

clean::
	rm -f *.o build flash.img
