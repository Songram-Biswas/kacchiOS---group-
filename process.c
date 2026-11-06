
#include "process.h"
#include "memory.h"
#include "string.h"
#include "serial.h"
#include "scheduler.h"

PCB process_table[MAX_PROCESSES];

int current_slot = -1;

static uint32_t next_pid = 1;


/* Initialize the Process Table */
void process_init(void) {

    for (int i = 0; i < MAX_PROCESSES; i++) {

        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
        process_table[i].esp = 0;
        process_table[i].stack_base = NULL;
        process_table[i].name[0] = '\0';
    }

    current_slot = -1;
    next_pid = 1;

    serial_puts("Process Manager Initialized.\n");
}


/* Create a New Process */
int process_create(void (*entry_point)(void), const char* name) {

    if (entry_point == NULL) {
        serial_puts("process_create: Invalid entry point!\n");
        return -1;
    }

    int slot = -1;

    /* Find an available process slot */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == PROCESS_UNUSED ||
            process_table[i].state == PROCESS_TERMINATED) {

            slot = i;
            break;
        }
    }

    if (slot == -1) {
        serial_puts("process_create: Process table full!\n");
        return -1;
    }

    /* Release the previous stack if reusing a terminated slot */
    if (process_table[slot].stack_base != NULL) {

        free_stack(process_table[slot].stack_base);
        process_table[slot].stack_base = NULL;
    }

    /* Allocate a new stack */
    void* stack_top = alloc_stack();

    if (stack_top == NULL) {
        serial_puts("process_create: Failed to allocate stack!\n");
        return -1;
    }

    /* Prepare initial CPU context */
    uint32_t* stk = (uint32_t*)stack_top;

    /* Return address when the process finishes */
    *(--stk) = (uint32_t)process_exit;

    /* Process entry point */
    *(--stk) = (uint32_t)entry_point;

    /* Initialize registers for POPAL */
    for (int i = 0; i < 8; i++) {
        *(--stk) = 0;
    }

    /* Initialize PCB */
    process_table[slot].pid = next_pid++;
    process_table[slot].state = PROCESS_READY;
    process_table[slot].esp = (uint32_t)stk;
    process_table[slot].stack_base = stack_top;

    /* Safely copy process name (maximum 31 characters) */
    const char* source = (name != NULL) ? name : "unnamed";

    int j = 0;

    while (j < 31 && source[j] != '\0') {
        process_table[slot].name[j] = source[j];
        j++;
    }

    process_table[slot].name[j] = '\0';

    serial_puts("Process created: PID ");
    serial_put_int((int32_t)process_table[slot].pid);
    serial_puts(" | Name: ");
    serial_puts(process_table[slot].name);
    serial_puts("\n");

    return (int)process_table[slot].pid;
}


/* Exit the Currently Running Process */
void process_exit(void) {

    if (current_slot < 0 || current_slot >= MAX_PROCESSES) {
        serial_puts("process_exit: Invalid current process!\n");
        return;
    }

    serial_puts("\n[Process ");
    serial_put_int((int32_t)process_table[current_slot].pid);
    serial_puts(" (");
    serial_puts(process_table[current_slot].name);
    serial_puts(") terminated]\n");

    /* Do not free the current stack here */
    process_table[current_slot].state = PROCESS_TERMINATED;

    /* Switch to another READY process or return to the kernel */
    schedule_yield();

    /* A terminated process must never continue execution */
    while (1) {
        __asm__ volatile ("hlt");
    }
}


/* Terminate a Process Using Its PID */
int process_terminate(uint32_t pid) {

    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state != PROCESS_UNUSED &&
            process_table[i].state != PROCESS_TERMINATED &&
            process_table[i].pid == pid) {

            serial_puts("Terminating PID: ");
            serial_put_int((int32_t)pid);
            serial_puts("\n");

            process_table[i].state = PROCESS_TERMINATED;

            /* If this is the currently running process */
            if (i == current_slot) {

                schedule_yield();

                while (1) {
                    __asm__ volatile ("hlt");
                }
            }

            /* A non-running process can safely release its stack */
            if (process_table[i].stack_base != NULL) {

                free_stack(process_table[i].stack_base);
                process_table[i].stack_base = NULL;
            }

            return 0;
        }
    }

    serial_puts("process_terminate: PID not found!\n");

    return -1;
}


/* Display the Process Table */
void process_dump(void) {

    serial_puts("\n--- Process Table (PCB) ---\n");

    int count = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state != PROCESS_UNUSED) {

            count++;

            serial_puts("PID: ");
            serial_put_int((int32_t)process_table[i].pid);

            serial_puts(" | Name: ");
            serial_puts(process_table[i].name);

            serial_puts(" | State: ");

            if (process_table[i].state == PROCESS_READY) {
                serial_puts("READY");
            }
            else if (process_table[i].state == PROCESS_RUNNING) {
                serial_puts("RUNNING");
            }
            else if (process_table[i].state == PROCESS_TERMINATED) {
                serial_puts("TERMINATED");
            }

            serial_puts(" | ESP: ");
            serial_put_hex(process_table[i].esp);

            serial_puts("\n");
        }
    }

    if (count == 0) {
        serial_puts("(No processes created yet)\n");
    }

    serial_puts("---------------------------\n");
}
