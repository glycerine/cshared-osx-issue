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
  signal(SIGINT,  handleInterrupt);
  
  printf("about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.\n");

#if USE_GOLIB
  BlockInSelect();
#else
  sleep(10);
#endif
  
  printf("back out of BlockInSelect()! R_interrupts_pending = %d\n", R_interrupts_pending);
  return 0;
}
