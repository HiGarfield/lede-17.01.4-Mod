#ifndef TIMEDTASK_H_INCLUDED
#define TIMEDTASK_H_INCLUDED

#include "common.h"

typedef int (*TaskFunc)(void *Arg1, void *Arg2);

int TimedTask_Init(void);

int TimedTask_Add(BOOL Persistent,
                  BOOL Asynchronous,
                  int Milliseconds,
                  TaskFunc Func,
                  void *Arg1,
                  void *Arg2,
                  BOOL Immediate
                  );

#endif // TIMEDTASK_H_INCLUDED
