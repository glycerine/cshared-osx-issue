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
    signal(SIGINT, handleInterrupt);
}


int main() {
  // race condition here:
  // OSX: with this sleep in place, then the following signal() will take effect, overriding what the cshared library is doing. Without the sleep (sleep commented out), the cshared library reset of the SIGINT handler happens after the following signal() call.
  //sleep(1); 
  sig_t prev;
  sig_t cur;
  prev = signal(SIGINT,  handleInterrupt);
  printf("prev = %p\n", prev);
  int count = 0;
  while(1) {
      cur = signal(SIGINT,  handleInterrupt);
      if (cur != handleInterrupt) {
          break;
        }
      count++;
      if (count > 10000000) {
        break;
        }
  }

  printf("done with loop at count = %d\n", count); // getting 0

  printf("about to call BlockInSelect(), which will exit after receiving 2 ctrl-c SIGINT signals.\n");

  BlockInSelect();
  printf("back from BlockInSelect\n");
  signal(SIGINT,  handleInterrupt);
  sleep(10);
  
  printf("after sleep 10, back out of BlockInSelect()! R_interrupts_pending = %d\n", R_interrupts_pending);
  return 0;
}
