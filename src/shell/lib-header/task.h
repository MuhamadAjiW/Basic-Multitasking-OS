
#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"

#define MAX_TASKS 64
#define MAX_TASKS_PNAME 8

// To pass to shell
typedef struct task_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t resource_amount;       // Amount of resources used
    char name[MAX_TASKS_PNAME];
} task_info;

typedef struct task_list
{
    task_info info[MAX_TASKS_PNAME];
    uint32_t num_task;
} task_list;

#endif