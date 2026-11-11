/**********************************************************************

	    Project 2: Multiprogramming 


  By:
      Jeff Hollingsworth
  e-mail:
      hollings@cs.umd.edu

  File:        proc2.c
  Created on:  3/6/1996
  Contents:    User Processes to be run concurrently
	  1.    Init() : Initial process, creates other ones & terminates
	  2..4  Long() , Ping() , and Pong() are three examplar processes
	  that are to be run concurently.  Long() is a CPU intensive
          job, while Ping() and Pong() bounce back between one another.
	  5.    Wait() : busy wait for a key to be pressed

  Simon Hawkin <cema@cs.umd.edu> 03/16/1998
      - Added progress monitoring output in Long().
  Jeff Hollingsworth <hollings@cs.umd.edu> 2/19/02
      - Re-written for the new project
  David Hovemeyer <daveho@cs.umd.edu> 2/27/04
      - Update for GeekOS 0.2.0


  [2] An enclosed test ("proc2.c", or "encl2.c").

**********************************************************************/

#include <conio.h>
#include <process.h>
#include <sched.h>
#include <sema.h>
#include <string.h>

#if !defined (NULL)
#define NULL 0
#endif

int main(int argc , char ** argv)
{
  int policy = -1;
  int start;
  int elapsed;
  int quantum;
  int scr_sem;			/* sid of screen semaphore */
  int id1, id2, id3, id4;    /* ID of child process */
  int gate;

  if (argc == 3) {
      if (!strcmp(argv[1], "rr")) {
          policy = 0;
      } else if (!strcmp(argv[1], "mlf")) {
          policy = 1;
      } else {
	  Print("usage: %s [rr|mlf] <quantum>\n", argv[0]);
	  Exit(1);
      }
      quantum = atoi(argv[2]);
      Set_Scheduling_Policy(policy, quantum);
  } else {
      Print("usage: %s [rr|mlf] <quantum>\n", argv[0]);
      Exit(1);
  }

  start = Get_Time_Of_Day();
  scr_sem = Create_Semaphore ("screen" , 1)  ;
  gate = Create_Semaphore("short_start", 0);

  P (scr_sem) ;
  Print ("************* Start Workload Generator *********\n");
  V (scr_sem) ;

  P (scr_sem) ;
  Print("PID  Program\n");

  id1 = Spawn_Program ("/c/long.exe", "/c/long.exe");
  Print("%d  /c/long.exe\n", id1);

  id2 = Spawn_Program ("/c/short.exe", "/c/short.exe");
  Print("%d  /c/short.exe (Short0)\n", id2);

  id3 = Spawn_Program ("/c/short.exe", "/c/short.exe");
  Print("%d  /c/short.exe (Short1)\n", id3);

  id4 = Spawn_Program ("/c/short.exe", "/c/short.exe");
  Print("%d  /c/short.exe (Short2)\n", id4);
  assert(id1 > 0 && id2 > 0 && id3 > 0 && id4 > 0);
  Print("Ready??? Go!\n");
  V (scr_sem);
  V(gate);

  Wait(id1);
  Wait(id2);
  Wait(id3);
  Wait(id4);
  Destroy_Semaphore(gate);
  Destroy_Semaphore(scr_sem);

  elapsed = Get_Time_Of_Day() - start;
  Print ("\nTests Completed at %d\n", elapsed) ;
  return 0;
}

