all:
	go build -buildmode=c-shared -o ../libmygolib.so mygolib.go
	#nm -gU ../libmygolib.so

clean:
	rm -f libmygolib.so *~ libmygolib.h

