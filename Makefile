all:
	cd mygolib && make
	gcc -DUSE_GOLIB=1 uses_mygolib.c -o with_mygolib libmygolib.so

clean:
	rm -f *.so *.h *~ with_mygolib no_mygolib && cd mygolib && make clean

no_go_lib:
	gcc -DUSE_GOLIB=0 uses_mygolib.c -o no_mygolib
