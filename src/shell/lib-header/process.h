
#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_PROCESS 64
#define MAX_PROCESS_PNAME 8

enum ProcState { NULL_PROCESS, NEW, READY, RUNNING, WAITING, TERMINATED };

// To pass to shell
struct process_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t resource_amount;       // Amount of resources used
    enum ProcState state;           // state
    char name[MAX_PROCESS_PNAME];
    
};

struct process_list
{
    struct process_info info[MAX_PROCESS];
    uint32_t num_process;
};

#endif