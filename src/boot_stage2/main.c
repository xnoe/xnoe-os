#include "../kernel/atapio.h"
#include "../kernel/screenstuff.h"


void main() {
    init_term();
    init_atapio();

    uint8_t* kernel_location = 0x80000;
    load_file("KERNEL  BIN", kernel_location);

    printf("Stage2 success!");

    //while (1);

    ((void(*)(void))kernel_location)();
}