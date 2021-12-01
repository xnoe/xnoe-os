#include "idt.h"

GateEntry idt[256];

void set_entry(uint8_t interrupt_number, uint16_t code_segment, void* handler, uint8_t type) {
  uint32_t handler_addr = (uint32_t)handler;
  uint16_t* handler_halves = (uint16_t*)&handler_addr;
  idt[interrupt_number] = (GateEntry){
    .offset_low = handler_halves[0],
    .selector = code_segment,
    .zero = 0,
    .type = type,
    .offset_high = handler_halves[1]
  };
}

__attribute__((interrupt)) void interrupt_20(interrupt_frame* frame) {
//  printf("Interrupt 20 received!!\n");
  outb(0x20, 0x20);
}

__attribute__((interrupt)) void page_fault(interrupt_frame* frame, uint32_t err_code) {
  uint32_t problem_address;
  asm("mov %%cr2, %0" : "=a" (problem_address) :);
  printf("(EIP %x): Page Fault at %x\n", frame->eip, problem_address);
  while (1) asm("hlt");
}

__attribute__((interrupt)) void ignore_interrupt(interrupt_frame* frame) {
  outb(0x20, 0x20);
}

__attribute__((interrupt)) void gpf(interrupt_frame* frame, uint32_t err_code) {
  printf("General Protection Fault %d\n", err_code);
  while (1) asm("hlt");
}

__attribute__((interrupt)) void context_switch(interrupt_frame* frame) {
  asm ("cli"); // Disable interrupts whilst handling the context switch.
  asm ("pusha"); // Push registers to the stack
  //asm ("pushf"); // Push flags to the stack

  Process* currentProc = Global::currentProc;
  Process* nextProc = 0;
  if (currentProc) {
    xnoe::linkedlist<Process*>* processes = &Global::kernel->processes;
    
    // This cursed bit of code first determines if the processes list is longer than 1 and if it is
    // - Determines if it has 2 or more elements
    //   - If it has two, swap the first and last, update prev and next of each to be null or the other item
    //   - If it has more than two, swap the first and last, then swap their next and prevs, and set the 
    //     other value to null
    if (processes->start->next != 0) {
      if (processes->end->prev == processes->start) {
        xnoe::linkedlistelem<Process*>* tmp = processes->start;
        processes->start = processes->end;
        processes->end = tmp;

        processes->start->prev = 0;
        processes->end->next = 0;
        processes->end->prev = processes->start;
        processes->start->next = processes->end;
      } else {
        xnoe::linkedlistelem<Process*>* tmp = processes->start;
        processes->start = processes->end;
        processes->end = tmp;

        processes->start->next = processes->end->next;
        processes->end->prev = processes->start->prev;

        processes->start->prev = 0;
        processes->end->next = 0;

        processes->start->next->prev = processes->start;
        processes->end->prev->next = processes->end;
      }
    }

    // Get the next process.
    nextProc = processes->start->elem;

    Global::currentProc = nextProc;

    uint32_t cESP;
    asm volatile ("mov %%esp, %0" : "=a" (cESP) :);
    currentProc->esp = cESP; // Store the current ESP of the current process process.

    outb(0x20, 0x20);

    // Select the next processes page directory
    asm volatile ("mov %0, %%cr3" : : "r" (nextProc->PD->phys_addr)); 
    // Restore ESP of the new process.
    asm volatile ("mov %0, %%esp" : : "m" (Global::kernel->processes.start->elem->esp));

    //asm ("popf"); // Pop flags
    asm ("popa"); // Restore registers
    asm ("mov -0xc(%ebp), %eax"); // Restore the initial eax
    // Clear the garbage that was on the stack from previous switch_context call.
    asm ("mov %ebp, %esp");
    asm ("pop %ebp"); // Pop EBP
    asm ("iret"); // Manually perform iret.
  }
}

extern uint8_t current_scancode;
extern char decoded;

__attribute__((interrupt)) void syscall(interrupt_frame* frame) {
  // Syscall ABI:
  // 0: print: Print null terminated string (in esi: char*)
  // 1: getch: Get current keyboard character ASCII (out eax: char)
  // 2: getchPS2: Get current keyboard character PS/2 code (out eax: char)
  // 3: readfile: Load file to location (in esi: char* filename; in edi: uint8_t* buffer)
  // 4: localalloc: LocalAlloc: Allocate under current process (in esi: size; out eax void* ptr)
  // 5: localdelete: LocalDelete: Deallocate under current process (in esi: pointer)
  // 6: filesize: Get file size (in esi: char* filename; out eax size bytes)
  // 7: fork: create process from filename (in esi: char* filename)

  uint32_t* ebp;
  asm("mov %%ebp, %0" : "=a" (ebp) :);
  uint32_t eax = *(ebp-4);
  uint32_t rval = eax;

  uint32_t syscall_number;
  uint32_t esi;
  uint32_t edi;
  asm("mov %%esi, %0" : "=a" (esi) :);
  asm("mov %%edi, %0" : "=a" (edi) :);
  switch (eax) {
    case 0:
      Global::kernel->terminal->printf("%s", (char*)esi);
      break;
    case 1:
      rval = decoded;
      break;
    case 2:
      rval = current_scancode;
      break;
    case 3:
      load_file(esi, edi);
      break;
    case 4:
      rval = Global::currentProc->allocate(esi);
      break;
    case 5:
      Global::currentProc->deallocate(esi);
      break;
    case 6:
      rval = file_size(esi);
      break;
    case 7: {
      Global::kernel->createProcess(esi);
      break;
    }
    default:
      break;
  }

  asm volatile ("mov %0, %%eax" : : "m" (rval));
  asm ("mov %ebp, %esp");
  asm ("pop %ebp");
  asm ("iret");
}

void init_idt() {
  idt_desc desc = {.size = 256 * sizeof(GateEntry) - 1, .offset = (uint32_t)idt};
  asm volatile("lidt %0" : : "m" (desc));
  for (int i=0; i<256; i++)
    set_entry(i, 0x08, &ignore_interrupt, 0x8E);
  
  set_entry(0x20, 0x08, &context_switch, 0x8E);
  set_entry(0xD, 0x08, &gpf, 0x8E);
  set_entry(0xE, 0x08, &page_fault, 0x8E);
  set_entry(0x7f, 0x08, &syscall, 0x8E);

  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x00);
  outb(0xA1, 0x00);
}

void enable_idt() {
  asm ("sti");
}