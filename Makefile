all:
	cd mygolib && make
	gcc -DUSE_GOLIB=1 uses_mygolib.c -o with_mygolib libmygolib.so
	gcc -DUSE_GOLIB=0 uses_mygolib.c -o no_mygolib

clean:
	rm -f *.so *.h *~ with_mygolib no_mygolib && cd mygolib && make clean


