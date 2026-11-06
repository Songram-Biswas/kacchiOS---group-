#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES 16

typedef enum {
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_TERMINATED
} ProcessState;

typedef struct {
    uint32_t pid;
    ProcessState state;
    uint32_t esp;
    void* stack_base;
    char name[32];
} PCB;

/* Process Manager Functions */
void process_init(void);
int process_create(void (*entry_point)(void), const char* name);
void process_exit(void);
int process_terminate(uint32_t pid);
void process_dump(void);

/* Shared process table */
extern PCB process_table[MAX_PROCESSES];
extern int current_slot;

#endif