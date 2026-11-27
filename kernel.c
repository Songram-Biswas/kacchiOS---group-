/* kernel.c - Main kernel with Demo Processes & Shell Integration (Roll 59) */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

extern uint8_t __kernel_end[];
#define MAX_INPUT 128

/* ============================================================================
 * Roll 59: Demo Workload Processes (Alpha, Beta, Gamma)
 * ============================================================================ */

void process_alpha(void) {
    for (int i = 1; i <= 3; i++) {
        serial_puts("[Alpha] Running iteration ");
        serial_put_int(i);
        serial_puts("/3 - Computing squares: ");
        serial_put_int(i * i);
        serial_puts("\n");
        schedule_yield();
    }
    serial_puts("[Alpha] Workload complete.\n");
}

void process_beta(void) {
    int a = 0, b = 1;
    for (int i = 1; i <= 3; i++) {
        int next = a + b;
        a = b;
        b = next;
        serial_puts("[Beta]  Running iteration ");
        serial_put_int(i);
        serial_puts("/3 - Fibonacci value: ");
        serial_put_int(b);
        serial_puts("\n");
        schedule_yield();
    }
    serial_puts("[Beta]  Workload complete.\n");
}

void process_gamma(void) {
    for (int i = 1; i <= 3; i++) {
        serial_puts("[Gamma] Running iteration ");
        serial_put_int(i);
        serial_puts("/3 - Testing heap allocation...\n");

        uint32_t* buf = (uint32_t*)kmalloc(64);
        if (buf != NULL) {
            buf[0] = (uint32_t)(i * 100);
            serial_puts("[Gamma] Allocated buffer at ");
            serial_put_hex((uint32_t)buf);
            serial_puts(" (val=");
            serial_put_int((int32_t)buf[0]);
            serial_puts(")\n");
            kfree(buf);
        } else {
            serial_puts("[Gamma] Heap allocation failed!\n");
        }
        schedule_yield();
    }
    serial_puts("[Gamma] Workload complete.\n");
}

/* ============================================================================
 * Helper: Print Shell Help Menu
 * ============================================================================ */
static void print_help(void) {
    serial_puts("\nAvailable Commands:\n");
    serial_puts("  help   - Show this command list\n");
    serial_puts("  mem    - Display memory manager status\n");
    serial_puts("  ps     - Display process table (PCB)\n");
    serial_puts("  spawn  - Create Alpha, Beta, and Gamma demo processes\n");
    serial_puts("  run    - Run the Round-Robin scheduler\n");
    serial_puts("  demo   - Spawn Alpha, Beta, Gamma and run scheduler\n");
    serial_puts("  clear  - Clear the terminal screen\n\n");
}

/* ============================================================================
 * Kernel Main Entry Point
 * ============================================================================ */
void kmain(void) {
    char input[MAX_INPUT];
    int pos = 0;

    serial_init();
    serial_puts("\n========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Roll Group: 55 to 60\n");
    serial_puts("Kernel End Address: ");
    serial_put_hex((uint32_t)__kernel_end);
    serial_puts("\n\n");

    /* Initialize OS subsystems */
    memory_init();
    process_init();
    scheduler_init();

    serial_puts("Type 'help' for commands, or 'demo' to run Alpha/Beta/Gamma.\n\n");

    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;
        while (1) {
            char c = serial_getc();
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            } else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");
            } else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);
            }
        }

        if (pos == 0) {
            continue;
        }

        if (strcmp(input, "help") == 0) {
            print_help();
        } else if (strcmp(input, "mem") == 0) {
            memory_dump();
        } else if (strcmp(input, "ps") == 0) {
            process_dump();
        } else if (strcmp(input, "spawn") == 0) {
            process_create(process_alpha, "Alpha");
            process_create(process_beta, "Beta");
            process_create(process_gamma, "Gamma");
        } else if (strcmp(input, "run") == 0) {
            scheduler_run();
        } else if (strcmp(input, "demo") == 0) {
            process_create(process_alpha, "Alpha");
            process_create(process_beta, "Beta");
            process_create(process_gamma, "Gamma");
            process_dump();
            scheduler_run();
            process_dump();
        } else if (strcmp(input, "clear") == 0) {
            serial_puts("\033[2J\033[H");
        } else {
            serial_puts("Unknown command: ");
            serial_puts(input);
            serial_puts("\nType 'help' for available commands.\n");
        }
    }
}
