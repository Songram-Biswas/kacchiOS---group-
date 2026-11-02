/* kernel.c - Main kernel with Memory Manager Verification (Roll 58) */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"

extern uint8_t __kernel_end[];
#define MAX_INPUT 128

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

    // মেমোরি ম্যানেজার ইনিশিয়ালাইজ করা হলো
    memory_init();

    // মেমোরি কাজ করছে কিনা দেখার জন্য ছোট্ট একটা টেস্ট ডাম্প কল করতে পারো:
    memory_dump();

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
        if (pos > 0) {
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
        }
    }
}