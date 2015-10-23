#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// forward declaration. BlockInSelect() is 
// defined in the c-shared library in mygolib/mygolib.go
void BlockInSelect();

// set my the handleInterrupt() signal handler.
static int R_interrupts_pending = 0;

static void handleInterrupt(int dummy)
{
    R_interrupts_pending = 1;
    printf("\n handleInterrupt called back!\n"); // problem: this is never hit when the mygolib is linked.
    //exit(1);
    signal(SIGINT, handleInterrupt);
}


int main() {
  // race condition here:
  sleep(1); // OSX: with this sleep, then the following signal() will take effect, overriding what the cshared library is doing. Without the sleep, the cshared library reset of the SIGINT handler happens after the following signal() call.
  signal(SIGINT,  handleInterrupt);
  
  printf("about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.\n");

  BlockInSelect();
  printf("back from BlockInSelect\n");
  signal(SIGINT,  handleInterrupt);
  sleep(10);
  
  printf("after sleep 10, back out of BlockInSelect()! R_interrupts_pending = %d\n", R_interrupts_pending);
  return 0;
}

/* LINUX:

[jaten@buzz cshared-osx-issue]$ make
cd mygolib && make
make[1]: Entering directory '/home/jaten/cshared-osx-issue/mygolib'
go build -buildmode=c-shared -o ../libmygolib.so mygolib.go
#nm -gU ../libmygolib.so
make[1]: Leaving directory '/home/jaten/cshared-osx-issue/mygolib'
gcc -DUSE_GOLIB=1 uses_mygolib.c -o with_mygolib libmygolib.so
gcc -DUSE_GOLIB=0 uses_mygolib.c -o no_mygolib
[jaten@buzz cshared-osx-issue]$ ./no_mygolib 
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c
 handleInterrupt called back!   # good, SIGINT working
back out of BlockInSelect()! R_interrupts_pending = 1
[jaten@buzz cshared-osx-issue]$ ./with_mygolib
about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.
  C-c C-c[jaten@buzz cshared-osx-issue]$ # bad, SIGINT not working
*/

