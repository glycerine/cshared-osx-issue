# cshared-osx-issue

With go 1.5.1, I'm seeing what looks like a bug.

Possibly related: https://github.com/golang/go/issues/11794

###Details:

When I am building a c-shared dynamic library with golang code, it looks
as though loading the shared library into the c-code masks the native signal
handling on OSX.  

This oes not happen on linux. Linux behavior seems correct,
OSX the golang shared object mystifies -- behavior seems buggy.

on darwin-amd64 / OSX 10.10.5 Yosemite:

The places below where you see "C-c C-c" is where I give Ctrl-c to 
the program. It shows up twice due to being inside an emacs buffer.

~~~
$ make  # generates ./with_mygolib
$ make no_go_lib   # generates ./no_mygolib

jaten@Jasons-MacBook-Pro:~/cshared$ ./no_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c
 handleInterrupt called back!
back out of BlockInSelect()! R_interrupts_pending = 1
jaten@Jasons-MacBook-Pro:~/cshared$ ./with_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
mylib.go: in BlockInSelect(): about to select on ctrlC_Chan
  C-c C-c

  I see ctrl-c !!

  C-c C-c

  I see ctrl-c !!

back out of BlockInSelect()! R_interrupts_pending = 0
jaten@Jasons-MacBook-Pro:~/cshared$
~~~

vs
on fedora22 linux amd64

~~~
[jaten@buzz cshared]$ make
cd mygolib && make
make[1]: Entering directory '/home/jaten/cshared/mygolib'
go build -buildmode=c-shared -o ../libmygolib.so mygolib.go
#nm -gU ../libmygolib.so
make[1]: Leaving directory '/home/jaten/cshared/mygolib'
gcc -DUSE_GOLIB=1 uses_mygolib.c -o with_mygolib libmygolib.so
[jaten@buzz cshared]$ make no_go_lib
gcc -DUSE_GOLIB=0 uses_mygolib.c -o no_mygolib
[jaten@buzz cshared]$ ./with_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
mylib.go: in BlockInSelect(): about to select on ctrlC_Chan
  C-c C-c

  I see ctrl-c !!

  C-c C-c

  I see ctrl-c !!

back out of BlockInSelect()! R_interrupts_pending = 0
[jaten@buzz cshared]$ ./no_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c
 handleInterrupt called back!
back out of BlockInSelect()! R_interrupts_pending = 1
[jaten@buzz cshared]$ 

~~~

