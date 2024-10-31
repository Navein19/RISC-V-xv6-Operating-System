#ifndef __UTHREAD_H__
#define __UTHREAD_H__

#include <stdbool.h>

#define MAXULTHREADS 100
#define USTACKSIZE 4096

enum ulthread_state {
  FREE,
  RUNNABLE,
  YIELD,
};

enum ulthread_scheduling_algorithm {
  ROUNDROBIN,   
  PRIORITY,     
  FCFS,         // first-come-first serve
};

// Separate context structure to save the thread's register state
struct context {
  uint64 ra;    // Return address
  uint64 sp;    // Stack pointer

  // Callee-saved registers
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct ulthread 
{
  enum ulthread_state state; // do hold the thread state either FREE, RUNNABLE or YIELD
  int thread_id; //unique thread ID
  uint64* stackPointer;
  int priority;
  struct context context;
  long long int last_time_shceduled;
};



#endif