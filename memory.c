#include "serial.h"

extern uint8_t __kernel_end[];

#define HEAP_SIZE      (1024 * 1024)
#define STACK_SIZE     4096
#define MAX_STACKS     16

typedef struct BlockHeader {
    size_t size;
    uint8_t is_free;
    struct BlockHeader* next;
} BlockHeader;

static uint8_t* heap_start = NULL;
static BlockHeader* free_list = NULL;
static uint8_t* stack_area_start = NULL;
static uint8_t stack_allocated[MAX_STACKS];

void memory_init(void) {
    uint32_t kernel_end_addr = (uint32_t)__kernel_end;
    if (kernel_end_addr % 4 != 0) kernel_end_addr += (4 - (kernel_end_addr % 4));

    heap_start = (uint8_t*)kernel_end_addr;
    free_list = (BlockHeader*)heap_start;
    free_list->size = HEAP_SIZE - sizeof(BlockHeader);
    free_list->is_free = 1;
    free_list->next = NULL;

    stack_area_start = heap_start + HEAP_SIZE;
    for (int i = 0; i < MAX_STACKS; i++) stack_allocated[i] = 0;

    serial_puts("Memory Manager Initialized.\n  Heap Start : ");
    serial_put_hex((uint32_t)heap_start);
    serial_puts("\n  Stack Start: ");
    serial_put_hex((uint32_t)stack_area_start);
    serial_puts("\n\n");
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    if (size % 4 != 0) size += (4 - (size % 4));

    BlockHeader* curr = free_list;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size >= size + sizeof(BlockHeader) + 4) {
                BlockHeader* next_block = (BlockHeader*)((uint8_t*)curr + sizeof(BlockHeader) + size);
                next_block->size = curr->size - size - sizeof(BlockHeader);
                next_block->is_free = 1;
                next_block->next = curr->next;
                curr->size = size;
                curr->next = next_block;
            }
            curr->is_free = 0;
            return (void*)((uint8_t*)curr + sizeof(BlockHeader));
        }
        curr = curr->next;
    }
    serial_puts("kmalloc: Out of memory!\n");
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) return;
    BlockHeader* header = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    header->is_free = 1;

    BlockHeader* curr = free_list;
    while (curr != NULL) {
        if (curr->is_free && curr->next != NULL && curr->next->is_free) {
            curr->size += sizeof(BlockHeader) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void* alloc_stack(void) {
    for (int i = 0; i < MAX_STACKS; i++) {
        if (!stack_allocated[i]) {
            stack_allocated[i] = 1;
            uint8_t* stack_bottom = stack_area_start + (i * STACK_SIZE);
            return (void*)(stack_bottom + STACK_SIZE);
        }
    }
    serial_puts("alloc_stack: Out of stacks!\n");
    return NULL;
}

void free_stack(void* stack_top) {
    if (stack_top == NULL) return;
    uint32_t top = (uint32_t)stack_top;
    uint32_t start = (uint32_t)stack_area_start;
    if (top <= start) return;
    int index = (top - start) / STACK_SIZE - 1;
    if (index >= 0 && index < MAX_STACKS) stack_allocated[index] = 0;
}

void memory_dump(void) {
    BlockHeader* curr = free_list;
    int block_num = 1;
    serial_puts("--- Memory Dump ---\n");
    while (curr != NULL) {
        serial_puts("Block ");
        serial_put_int(block_num++);
        serial_puts(": Size = ");
        serial_put_int(curr->size);
        serial_puts(" bytes | Status: ");
        serial_puts(curr->is_free ? "FREE\n" : "USED\n");
        curr = curr->next;
    }
    serial_puts("-------------------\n");
}
