#include "scheduler.h"
#include "process.h"
#include "serial.h"

extern void context_switch(uint32_t* old_esp, uint32_t new_esp);
static uint32_t kernel_esp = 0;

void scheduler_init(void) {
    kernel_esp = 0;
    serial_puts("Scheduler Initialized (Policy: Round-Robin).\n\n");
}

static int find_next_ready(int start_slot) {
    for (int offset = 1; offset <= MAX_PROCESSES; offset++) {
        int idx = (start_slot + offset) % MAX_PROCESSES;
        if (process_table[idx].state == PROCESS_READY) return idx;
    }
    return -1;
}

void schedule_yield(void) {
    int prev_slot = current_slot;
    int next_slot = find_next_ready(prev_slot < 0 ? 0 : prev_slot);

    if (next_slot == -1) {
        if (prev_slot >= 0 && process_table[prev_slot].state == PROCESS_RUNNING) return;
        current_slot = -1;
        context_switch(NULL, kernel_esp);
        return;
    }

    if (prev_slot >= 0 && process_table[prev_slot].state == PROCESS_RUNNING) {
        process_table[prev_slot].state = PROCESS_READY;
    }
    process_table[next_slot].state = PROCESS_RUNNING;
    current_slot = next_slot;

    if (prev_slot >= 0 && process_table[prev_slot].state == PROCESS_READY) {
        context_switch(&process_table[prev_slot].esp, process_table[next_slot].esp);
    } else {
        context_switch(NULL, process_table[next_slot].esp);
    }
}

void scheduler_run(void) {
    int first_slot = find_next_ready(MAX_PROCESSES - 1);
    if (first_slot == -1) { serial_puts("Scheduler: No READY processes!\n"); return; }

    serial_puts("\n=== Starting Round-Robin Scheduler ===\n");
    process_table[first_slot].state = PROCESS_RUNNING;
    current_slot = first_slot;

    context_switch(&kernel_esp, process_table[first_slot].esp);
    serial_puts("=== All Processes Completed. Back to Kernel Shell ===\n\n");
}