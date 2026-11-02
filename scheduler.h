#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"

void scheduler_init(void);
void schedule_yield(void);
void scheduler_run(void);

#endif