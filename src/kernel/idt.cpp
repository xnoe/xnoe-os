#include "idt.h"

GateEntry idt[256];

void(*gates[256])(frame_struct*);

extern void(*isrs[256])(void);

void set_entry(uint8_t interrupt_number, uint16_t code_segment, void(*handler)(), uint8_t type, uint8_t privilege) {
  uint32_t handler_addr = (uint32_t)handler;
  uint16_t* handler_halves = (uint16_t*)&handler_addr;
  idt[interrupt_number] = (GateEntry) {
    .offset_low = handler_halves[0],
    .selector = code_segment,
    .zero = 0,
    .type = type,
    .zero1 = 0,
    .privilege = privilege,
    .present = 1,
    .offset_high = handler_halves[1]
  };
}

void page_fault(frame_struct* frame, uint32_t err_code) {
  uint32_t problem_address;
  asm("mov %%cr2, %0" : "=a" (problem_address) :);
  Global::kernel->terminal->printf("(EIP %x): Page Fault at %x\n", frame->eip, problem_address);
  if (frame->cs & 3 == 0) {
    Global::kernel->terminal->printf("[FATAL] Kernel Page Fault!!!\n");
    while (1) asm("hlt");
  } else {
    // Print an error message.
    Global::kernel->terminal->printf("PID %d Terminated due to page fault!\n", Global::currentProc->PID);
    // We are currently in the kernel stack for the current process so we need to load the main kernel stack in to esp.
    //Global::kernel->loadPrimaryStack();

    asm volatile ("mov %0, %%esp" ::"m"(Global::kernel->stack));

    // We can now safely delete the current process
    Global::kernel->destroyProcess(Global::currentProc);

    // We want to load the kernel's page directory too.
    //Global::kernel->PD->select();

    //Global::currentProcValid = false;
    asm ("int $0x20"); // Call context switch.
    while (1) asm("hlt");
  }
}

void ignore_interrupt(frame_struct* frame) {}

void gpf(frame_struct* frame, uint32_t err_code) {
  printf("General Protection Fault %x\n", err_code);
  while (1) asm("hlt");
}

void context_switch(frame_struct* frame) {
  // When any interrupt occurs (including context_switch), SS:ESP is set to
  // the values of SS0:ESP0 in Global::tss
  //
  // This means that processes need to track a kernel stack pointer
  // Which is the location of their indivudual kernel stacks.
  //
  // Context switch needs to do two things.
  // #1 update currentProc's kernel stack pointer to be the correct value after
  // data has been pushed on to the stack
  // 
  // #2 load the kernelStackPtr in to esp before popping data and falling through 
  // to iret

  asm ("cli"); // Disable interrupts whilst handling the context switch.

  // Restore eax
  asm ("mov %0, %%eax"::"r"(frame->eax));

  Process* currentProc = 0;
  Process* nextProc = 0;

  if (Global::currentProcValid) {
    currentProc = Global::currentProc;
    // Write current esp to currentProc->kernelStackPtr
    asm ("mov %%esp, %0" : "=a" (currentProc->kernelStackPtr):);
  }

  if (currentProc || !Global::currentProcValid) {
    xnoe::linkedlist<Process*>* processes = &Global::kernel->processes;
    
    // This cursed bit of code first determines if the processes list is longer than 1 and if it is
    // - Determines if it has 2 or more elements
    //   - If it has two, swap the first and last, update prev and next of each to be null or the other item
    //   - If it has more than two, add the start to the end then set start to the second element
    if (processes->start) {
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
          processes->end->next = processes->start;
          processes->start = processes->start->next;
          processes->start->prev = 0;
          xnoe::linkedlistelem<Process*>* tmp = processes->end;
          processes->end = processes->end->next;
          processes->end->next = 0;
          processes->end->prev = tmp;
        }
      }
    }

    // Get the next process.
    if (processes->start)
      nextProc = processes->start->elem;

    if (nextProc == 0) {
      Global::kernel->terminal->printf("[FATAL] No more processes! Halting!\n");
      while (1) asm ("hlt");
    }

    Global::currentProc = nextProc;

    // Select the next processes page directory
    asm volatile ("mov %0, %%cr3" : : "r" (nextProc->PD->phys_addr)); 
    // Restore kernelStackPtr of the new process.
    asm volatile ("mov %0, %%esp" : : "m" (Global::kernel->processes.start->elem->kernelStackPtr));

    // At this point interrupts are disabled till iret so we can safely set
    // Global::tss->esp0 to the new Process's kernelStackPtrDefault

    Global::tss->esp0 = Global::kernel->processes.start->elem->kernelStackPtrDefault;

    // Set the current proc to valid 
    Global::currentProcValid = true;

    uint32_t* espVal;
    asm ("mov %%esp, %0":"=a"(espVal):);
    if (*espVal == 0) {
      asm("add $4, %esp");
      asm("ret");
    }
  }
}

extern uint8_t current_scancode;
extern char decoded;

void syscall(frame_struct* frame) {
  // Syscall ABI:
  // 0: print: Print null terminated string (in esi: char*)
  // 1: getch: Get current keyboard character ASCII (out eax: char)
  // 2: getchPS2: Get current keyboard character PS/2 code (out eax: char)
  // 3: readfile: Load file to location (in esi: char* filename; in edi: uint8_t* buffer)
  // 4: localalloc: LocalAlloc: Allocate under current process (in esi: size; out eax void* ptr)
  // 5: localdelete: LocalDelete: Deallocate under current process (in esi: pointer)
  // 6: filesize: Get file size (in esi: char* filename; out eax size bytes)
  // 7: fork: create process from filename (in esi: char* filename)
  // 8: getPID: returns the current process's PID (out eax: uint32_t)

  uint32_t rval = frame->eax;

  uint32_t esi = frame->esi;
  uint32_t edi = frame->edi;
  switch (frame->eax) {
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
    case 8:
      rval = Global::currentProc->PID;
      break;
    default:
      break;
  }

  frame->eax = rval;
}

void init_idt() {
  idt_desc desc = {.size = 256 * sizeof(GateEntry) - 1, .offset = (uint32_t)idt};
  asm volatile("lidt %0" : : "m" (desc));

  for (int i=0; i<256; i++)
    set_entry(i, 0x08, isrs[i], 0xE);

  for (int i=0; i<256; i++)
    gates[i] = &ignore_interrupt;
  
  gates[0x20] = &context_switch;
  gates[0xd] = &gpf;
  gates[0xe] = &page_fault;
  gates[0x7f] = &syscall;

  idt[0x7f].privilege = 3;

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