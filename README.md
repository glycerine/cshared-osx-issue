# cshared-osx-issue

With go 1.5.1 and at tip, I'm seeing what looks like a bug when c-shared .so libraries that do signal.Notify(c, os.Interrupt) are loaded in a host program that handles SIGINT itself.

Update: tried with tip *on OSX and linux*, go version devel +79a3b56 Thu Oct 22 21:19:43 2015 +0000 darwin/amd64, and I see the same thing.

Possibly related: https://github.com/golang/go/issues/11794

###Discussion:

When I am building a c-shared dynamic library with golang code, it looks
as though loading the shared library into the c-code masks the native signal
handling on OSX.  And on linux.

I was led to investigate because when I loaded
a c-shared built .so library into the R statistical analysis 
environment, set-up handlers with signal.Notify(), and then pressed ctrl-c:
 it panics/crashes on OSX, but
works fine on Linux. See the last stack dump in this repo for the full details of that panic.

This repo is an attempt to reduce/isolate that issue into a minimal test case. I've not been successful yet in reproducing the difference between OSX and Linux, and I suspect this is due to variation in the signal handling code either in the Golang runtime or the R runtime.

Nonethless, while I cannot reproduce the crash/panic in a minimal test case (it reproduces easily in you want to go to the trouble of doing an R source install and compiling my R library 'rmq'), I do observe that signal handling under OSX and linux appears to be disabled by loading the golang based c-shared library, and I strongly suspect that this is a part of the mechanism of the crash. Moreover it seems like a bug in its own right.

I think this is related to https://github.com/golang/go/issues/11794.

### details

on darwin-amd64 / OSX 10.10.5 Yosemite:

The places below where you see "C-c C-c" is where I give Ctrl-c to 
the program. It shows up twice due to being inside an emacs buffer.

~~~
$ make  # generates ./with_mygolib
$ make no_go_lib   # generates ./no_mygolib

jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$ make
cd mygolib && make
go build -buildmode=c-shared -o ../libmygolib.so mygolib.go
#nm -gU ../libmygolib.so
gcc -DUSE_GOLIB=1 uses_mygolib.c -o with_mygolib libmygolib.so
jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$ ./with_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
mylib.go: in BlockInSelect(): about to select on ctrlC_Chan
  C-c C-c

  I see ctrl-c !!
                           ## what?? why no handleInterrupts() call!??
  C-c C-c

  I see ctrl-c !!

back out of BlockInSelect()! R_interrupts_pending = 0 
jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$ ./no_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c
 handleInterrupt called back!
back out of BlockInSelect()! R_interrupts_pending = 1
jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$   C-c C-c
jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$ go version
go version devel +79a3b56 Thu Oct 22 21:19:43 2015 +0000 darwin/amd64
jaten@Jasons-MacBook-Pro:~/cshared-osx-issue$ 
~~~

The same thing happens on fedora22 linux amd64, go1.5.1. And at tip 79a3b56.

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

back out of BlockInSelect()! R_interrupts_pending = 0   ## what??? 
[jaten@buzz cshared]$ ./no_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c
 handleInterrupt called back!
back out of BlockInSelect()! R_interrupts_pending = 1   ## seems to work when no c-shared go lib present.
[jaten@buzz cshared]$ 

~~~

### the originating (and more elaborate) problem: the panic stack trace

Here is the stack trace from the crash when running under R on OSX. Source code is here: https://github.com/glycerine/rmq

The above code is a minimal test case for this more elaborate issue found in context of running under R.

~~~
$ R
R version 3.2.2 (2015-08-14) -- "Fire Safety"
Copyright (C) 2015 The R Foundation for Statistical Computing
Platform: x86_64-apple-darwin14.5.0 (64-bit)

R is free software and comes with ABSOLUTELY NO WARRANTY.
You are welcome to redistribute it under certain conditions.
Type 'license()' or 'licence()' for distribution details.

  Natural language support but running in an English locale

R is a collaborative project with many contributors.
Type 'contributors()' for more information and
'citation()' on how to cite R or R packages in publications.

Type 'demo()' for some demos, 'help()' for on-line help, or
'help.start()' for an HTML browser interface to help.
Type 'q()' to quit R.

> options(STERM='iESS', editor='emacsclient')
> require(rmq)
Loading required package: rmq
> require(testthat)
Loading required package: testthat
> handler = function(x) {
+   print(paste("handler called back with argument x = ", paste(collapse=" ",sep=" ",x)))
+   reply=list()
+   reply$hi = "there!"
+   reply$yum = c(1.1, 2.3)
+   reply$input = x
+   reply
+ }
> options(error=recover)
> r = .Call("ListenAndServe", "127.0.0.1:9090", handler, new.env(), package="rmq")
ListenAndServe listening on address '127.0.0.1:9090'...
  C-c C-cfatal error: unexpected signal during runtime execution
[signal 0xb code=0x1 addr=0x8 pc=0x10dd976d9]

runtime stack:
runtime.throw(0x10e425c20, 0x2a)
	/usr/local/go/src/runtime/panic.go:527 +0x90
runtime.sigpanic()
	/usr/local/go/src/runtime/sigpanic_unix.go:12 +0x5a
runtime.sighandler(0xc820032000, 0x0, 0x0, 0x3)
	/usr/local/go/src/runtime/signal_amd64x.go:76 +0x139

goroutine 17 [select, locked to thread]:
runtime.gopark(0x10e468a70, 0xc820061e00, 0x10e37e638, 0x6, 0xc82001e718, 0x2)
	/usr/local/go/src/runtime/proc.go:185 +0x163 fp=0xc820061ad0 sp=0xc820061aa8
runtime.selectgoImpl(0xc820061e00, 0x0, 0x18)
	/usr/local/go/src/runtime/select.go:392 +0xa64 fp=0xc820061c78 sp=0xc820061ad0
runtime.selectgo(0xc820061e00)
	/usr/local/go/src/runtime/select.go:212 +0x12 fp=0xc820061c98 sp=0xc820061c78
main.ListenAndServe(0x7ffd733c4178, 0x7ffd730ddd48, 0x7ffd730e3920, 0x7ffd730e3040)
	/Users/jaten/pkg/R-3.2.2/src/library/Recommended/rmq/src/rmq/rmq.go:161 +0x865 fp=0xc820061ed0 sp=0xc820061c98
runtime.call32(0x0, 0x7fff545f43d8, 0x7fff545f4468, 0x20)
	/usr/local/go/src/runtime/asm_amd64.s:437 +0x3e fp=0xc820061ef8 sp=0xc820061ed0
runtime.cgocallbackg1()
	/usr/local/go/src/runtime/cgocall.go:252 +0x10c fp=0xc820061f30 sp=0xc820061ef8
runtime.cgocallbackg()
	/usr/local/go/src/runtime/cgocall.go:177 +0xd7 fp=0xc820061f90 sp=0xc820061f30
runtime.cgocallback_gofunc(0x0, 0x0, 0x0)
	/usr/local/go/src/runtime/asm_amd64.s:801 +0x60 fp=0xc820061fa0 sp=0xc820061f90
runtime.goexit()
	/usr/local/go/src/runtime/asm_amd64.s:1696 +0x1 fp=0xc820061fa8 sp=0xc820061fa0

goroutine 8 [select, locked to thread]:
runtime.gopark(0x10e468a70, 0xc82002f728, 0x10e37e638, 0x6, 0x18, 0x2)
	/usr/local/go/src/runtime/proc.go:185 +0x163
runtime.selectgoImpl(0xc82002f728, 0x0, 0x18)
	/usr/local/go/src/runtime/select.go:392 +0xa64
runtime.selectgo(0xc82002f728)
	/usr/local/go/src/runtime/select.go:212 +0x12
runtime.ensureSigM.func1()
	/usr/local/go/src/runtime/signal1_unix.go:227 +0x323
runtime.goexit()
	/usr/local/go/src/runtime/asm_amd64.s:1696 +0x1

goroutine 6 [syscall]:
os/signal.loop()
	/usr/local/go/src/os/signal/signal_unix.go:22 +0x18
created by os/signal.init.1
	/usr/local/go/src/os/signal/signal_unix.go:28 +0x37

goroutine 9 [IO wait]:
net.runtime_pollWait(0x10edd6950, 0x72, 0xc820014210)
	/usr/local/go/src/runtime/netpoll.go:157 +0x60
net.(*pollDesc).Wait(0xc82011c060, 0x72, 0x0, 0x0)
	/usr/local/go/src/net/fd_poll_runtime.go:73 +0x3a
net.(*pollDesc).WaitRead(0xc82011c060, 0x0, 0x0)
	/usr/local/go/src/net/fd_poll_runtime.go:78 +0x36
net.(*netFD).accept(0xc82011c000, 0x0, 0x10f061038, 0xc8201200e0)
	/usr/local/go/src/net/fd_unix.go:408 +0x27c
net.(*TCPListener).AcceptTCP(0xc820122000, 0xc82003cd88, 0x0, 0x0)
	/usr/local/go/src/net/tcpsock_posix.go:254 +0x4d
net.(*TCPListener).Accept(0xc820122000, 0x0, 0x0, 0x0, 0x0)
	/usr/local/go/src/net/tcpsock_posix.go:264 +0x3d
net/http.(*Server).Serve(0xc82008f040, 0x10f061000, 0xc820122000, 0x0, 0x0)
	/usr/local/go/src/net/http/server.go:1887 +0xb3
vendor/github.com/glycerine/go-tigertonic.(*Server).Serve(0xc82008f040, 0x10f061000, 0xc820122000, 0x0, 0x0)
	/Users/jaten/pkg/R-3.2.2/src/library/Recommended/rmq/src/vendor/github.com/glycerine/go-tigertonic/server.go:160 +0xd8
vendor/github.com/glycerine/go-tigertonic.(*Server).ListenAndServe(0xc82008f040, 0x0, 0x0)
	/Users/jaten/pkg/R-3.2.2/src/library/Recommended/rmq/src/vendor/github.com/glycerine/go-tigertonic/server.go:144 +0x1e2
main.(*WebServer).Start.func1(0xc820017d80)
	/Users/jaten/pkg/R-3.2.2/src/library/Recommended/rmq/src/rmq/web.go:54 +0x25
created by main.(*WebServer).Start
	/Users/jaten/pkg/R-3.2.2/src/library/Recommended/rmq/src/rmq/web.go:60 +0x4e

goroutine 18 [syscall, locked to thread]:
runtime.goexit()
	/usr/local/go/src/runtime/asm_amd64.s:1696 +0x1

Process R exited abnormally with code 2 at Thu Oct 22 13:47:58 2015
$
~~~