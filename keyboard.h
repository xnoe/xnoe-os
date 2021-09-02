#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include "io.h"
#include "screenstuff.h"
#include "idt.h"

void init_keyboard();
void readline(int max, char* buffer);

#endif