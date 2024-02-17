
#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"

#define MAX_TASKS 64
#define MAX_TASKS_PNAME 8

enum ProcState { NULL_TASK, NEW, READY, RUNNING, WAITING, TERMINATED };

// To pass to shell
struct task_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t resource_amount;       // Amount of resources used
    enum ProcState state;           // state
    char name[MAX_TASKS_PNAME];
    
};

struct task_list
{
    struct task_info info[MAX_TASKS];
    uint32_t num_task;
};

#endif