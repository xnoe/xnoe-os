#include "../kernel/atapio.h"
#include "../kernel/screenstuff.h"

typedef struct {
  uint32_t base_low;
  uint32_t base_high;
  uint32_t length_low;
  uint32_t length_high;
  uint32_t type;
  uint32_t acpi3_extended;
}__attribute__((packed)) e820entry;

void set_bit(uint32_t offset, uint8_t* buffer) {
    uint32_t index = offset / 8;
    uint32_t bit = offset % 8;

    buffer[index] |= (1<<(7 - bit));
}

void main() {
    init_term();
    init_atapio();

    // e820 memory map exists at 0x20000
    e820entry* e820_entries = 0x20000;

    uint8_t* bitmap = 0x100000;

    // Zero out the bitmap.
    for (int i=0; i<0x20000; i++)
        bitmap[i] = 0;
    // Ensure the bitmap data is clear

    for (int i=0; i<0x20000; i++)
        if (bitmap[i])
            printf("Found data in bitmap at %x!\n", (bitmap+i));

    for (int i=0; e820_entries[i].length_low != 0 || e820_entries[i].length_high != 0; i++) {
        e820entry entry = e820_entries[i];
        printf("BIOS-e820: Starting %x%x, length %x%x is %s\n", entry.base_high, entry.base_low, entry.length_high, entry.length_low, entry.type == 1 ? "Available" : "Reserved");

        if (entry.type != 1)
            continue;
        
        uint32_t base = entry.base_low;
        uint32_t length = entry.length_low;

        if (base % 4096) { 
            // Memory isn't page aligned, we need to fix that.
            uint32_t add_offset = 4096 - (base % 4096);
            if (length > add_offset) {
                base += add_offset;
                length -= add_offset;
            }
        }
        uint32_t page_index = base / 4096;

        printf("Page Index: %d\nLength (Pages): %d\n", page_index, length / 4096);

        for (int j=0; length > 4096; length -= 4096, j++) {
            uint32_t index = (page_index+j) / 8;
            uint32_t bit = (page_index+j) % 8;

            bitmap[index] |= (1<<(7 - bit));
        }
    }

    uint8_t* kernel_location = 0x80000;
    load_file("KERNEL  BIN", kernel_location);

    printf("Stage2 success!\n");

    while (1);
    ((void(*)(void))kernel_location)();
}