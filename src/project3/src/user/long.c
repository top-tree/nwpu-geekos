
#include "libuser.h"
#include "process.h"

int main(int argc, char **argv)
{
  int scr_sem;		/* id of screen semaphore */
  int start, elapsed;

  start = Get_Time_Of_Day();
  scr_sem = Create_Semaphore ("screen" , 1) ;   /* register for screen use */

  while (Get_Time_Of_Day() - start < 20)
      ;
  elapsed = Get_Time_Of_Day() - start;
  P (scr_sem) ;
  Print("Process Long is done at time: %d\n", elapsed) ;
  V(scr_sem);


  return 0;
}

