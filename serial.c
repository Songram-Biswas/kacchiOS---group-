/* serial.c - Serial port driver (COM1) */
#include "serial.h"
#include "io.h"

#define COM1 0x3F8   /* I/O port base address for COM1 */

/*
You can find more information here: https://caro.su/msx/ocm_de1/16550.pdf

Your Keyboard
    ↓
Terminal (stdin)
    ↓
QEMU (-serial stdio)
    ↓
Emulated COM1 port (0x3F8)
    ↓
serial_getc() reads from COM1
    ↓
Your OS receives the character

If you want real keyboard input, you'd need to add a keyboard driver.
*/

void serial_init(void) {
    outb(COM1 + 1, 0x00);    /* Disable interrupts */
    outb(COM1 + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
    outb(COM1 + 0, 0x03);    /* Divisor low byte (38400 baud) */
    outb(COM1 + 1, 0x00);    /* Divisor high byte */
    outb(COM1 + 3, 0x03);    /* 8 bits, no parity, 1 stop bit */
    outb(COM1 + 2, 0xC7);    /* Enable FIFO, clear, 14-byte threshold */
    outb(COM1 + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    if (c == '\n') {
        serial_putc('\r');  /* Add carriage return */
    }
    while (!is_transmit_empty());
    outb(COM1, c);
}

void serial_puts(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}

static int serial_received(void) {
    return inb(COM1 + 5) & 0x01;
}

char serial_getc(void) {
    while (!serial_received());
    return inb(COM1);
}
void serial_put_int(int32_t num) {
    char buf[12];
    int i=0;

    if (num==0 ){
        serial_putc('-');
    }
    /* Jodi negative number hoy, age '-' print koro ebong positive বানিয়ে nao */
    if (num < 0) {
        serial_putc('-');
        num = -num;
    }
    /* Shesher dik theke ekta ekta kore digit ber kore array-te rakho */
    while (num > 0) {
        int digit = num % 10;
        buf[i++] = '0' + digit;
        num = num / 10;
    }

    /* Array-te digit-gulo ulto ache, tai shesh theke shurur dike print koro */
    while (i > 0) {
        i--;
        serial_putc(buf[i]);
    }
}

/* 32-bit Hexadecimal বা Memory Address print korar function (jemon: 0x00104000) */
void serial_put_hex(uint32_t num) {
    const char* hex_chars = "0123456789ABCDEF";
    int i;

    serial_puts("0x");

    /* 32-bit number-e 8-ta 4-bit (hex digit) thake: 28, 24, 20, 16, 12, 8, 4, 0 */
    for (i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (num >> i) & 0xF;
        serial_putc(hex_chars[nibble]);
    }

}