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

void handle_fault(frame_struct* frame) {
  // Clear interrupts, we don't want to perform a context switch whilst handling a fault.
  asm ("cli");
  uint32_t problem_address;
  asm ("mov %%cr2, %0" : "=a" (problem_address):);
  Global::kernel->terminal->printf("(CS %x EIP %x): ", frame->cs, frame->eip);
  switch (frame->gate) {
    case 0: // Divide by zero
      Global::kernel->terminal->printf("Divide by Zero");
      break;
    case 6: // Invalid Opcode
      Global::kernel->terminal->printf("Invalid Opcode");
      break;
    case 13: // GPF
      Global::kernel->terminal->printf("General Protection Fault!");
      break;
    case 14: // Page Fault
      Global::kernel->terminal->printf("Page Fault at %x", problem_address);
      break;
    default:
      Global::kernel->terminal->printf("Unkown Fault!");
      break;
  } 
  Global::kernel->terminal->printf(" Error Code: %x\n", frame->errcode);
  if (!(frame->cs & 3)) {
    Global::kernel->terminal->printf("[FATAL] Kernel Fault!!!\n");
    while (1) asm("hlt");
  } else {
    // Print an error message.
    Global::kernel->terminal->printf("PID %d Terminated due to fault!\n", Global::currentProc->PID);
    asm volatile ("mov %0, %%esp" ::"m"(Global::kernel->globalISRStack));
    Global::kernel->PD->select();

    // We can now safely delete the current process
    Global::kernel->destroyProcess(Global::currentProc);
    
    Global::currentProcValid = false;

    // Go in to an infinite loop
    asm ("sti");
    while (1) asm ("hlt");
  }
}

void ignore_interrupt(frame_struct* frame) {}

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

  xnoe::linkedlist<Process*>* processes = &Global::kernel->processes;

  if (!processes->start) {
    Global::kernel->terminal->printf("[FATAL] No more processes! Halting!\n");
    while (1) asm ("hlt");
  }

  if (Global::currentProcValid)
    Global::currentProc->kernelStackPtr = frame->new_esp;
  
  // This cursed bit of code first determines if the processes list is longer than 1 and if it is
  // - Determines if it has 2 or more elements
  //   - If it has two, swap the first and last, update prev and next of each to be null or the other item
  //   - If it has more than two, add the start to the end then set start to the second element
  do {
    if (Global::currentProc) {
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
    Global::currentProc = processes->start->elem;
  } while (Global::currentProc->state == Suspended);

  
  // Select the next processes page directory
  frame->new_cr3 = Global::currentProc->PD->phys_addr;
  // Restore kernelStackPtr of the new process.
  frame->new_esp = Global::currentProc->kernelStackPtr;

  Global::tss->esp0 = Global::currentProc->kernelStackPtrDefault;

  // Set the current proc to valid
  Global::currentProcValid = true;
}

namespace Timer {
  // counter, default count, function, argument, oneshot
  using TimedEvent = xnoe::tuple<uint32_t, uint32_t, void(*)(frame_struct*, void*), void*, bool>;
  xnoe::linkedlist<TimedEvent> timed_events;
  void tick(frame_struct* frame) {
    xnoe::linkedlistelem<TimedEvent>* current = timed_events.start;
    while (current) {
      TimedEvent t = current->elem;
      uint32_t count = xnoe::get<0>(t);
      if (--count == 0) {
        xnoe::get<2>(t)(frame, xnoe::get<3>(t));
        count = xnoe::get<1>(t);

        if (xnoe::get<4>(t)) {
          xnoe::linkedlistelem<TimedEvent>* prev = current;
          current = current->next;
          timed_events.remove(prev);
          delete prev;
        }
      }
      current->elem = TimedEvent(count, xnoe::get<1>(t), xnoe::get<2>(t), xnoe::get<3>(t), xnoe::get<4>(t));
      current = current->next;
    }
  }

  void register_event(uint32_t milliseconds, void(*function)(frame_struct*, void*), void* auxiliary, bool oneshot=false) {
    timed_events.append(TimedEvent(milliseconds, milliseconds, function, auxiliary, oneshot));
  }
}

void awaken(frame_struct* frame, Process* p) {
  p->state = Running;
}

void syscall(frame_struct* frame) {
  // Syscall ABI:
  // 0: getDentsSize :: char* path -> uint32_t size
  // 1: getDents :: char* path -> uint8_t* buffer -> void
  // 2: exists :: char* path -> bool
  // 3: type :: char* path -> FSType
  // 4: localalloc :: uint32_t size -> void* ptr
  // 5: localdelete :: void* ptr -> void
  // 6: X
  // 7: exec :: void* filehandler -> int PID // Spawns a process and returns its PID.
  // 8: getPID: returns the current process's PID (out eax: uint32_t)
  // 9: getFileHandler :: char* path -> void* // Returns a file handlers for a specific file
  // 10: read :: uint32_t count -> void* filehandler -> uint8_t* outputbuffer -> int read // Reads from a file handler in to a buffer, returns successful read
  // 11: write :: uint32_t count -> void* filehandler -> uint8_t* inputbuffer -> int written // Reads from a buffer in to a file, returns successful written
  // 12: bindToKeyboard :: void -> void // Binds the current process's stdout to the keyboard.
  
  // 13: bindStdout :: int PID -> int filehandler // Returns a filehandler for a CircularRWBuffer binding stdout of another process.
  // 14: bindStdin :: int PID -> int filehandler // Returns a filehandler for a CircularRWBuffer binding stdin of another process.

  // 15: fopen :: char* path -> int filehandler // Returns a filehandler to the file.
  // 16: fclose :: int filehandler -> void // Closes a file handler.

  // 17: kill :: int PID -> void // Destroys a process.

  // 18: sleep :: int time ms -> void // Sleeps the current process for time milliseconds.

  // File handlers:
  // 0: Stdout
  // 1: Stdin
  // 2..7: Reserved
  // _: General use

  uint32_t rval = frame->eax;

  Process* currentProc = Global::currentProc;

  switch (frame->eax) {
    case 0:
      rval = Global::kernel->rootfs->getDentsSize(createPathFromString(frame->ebx));
      break;
    case 1:
      Global::kernel->rootfs->getDents(createPathFromString(frame->ebx), frame->ecx);
      break;
    case 2:
      rval = Global::kernel->rootfs->exists(createPathFromString(frame->ebx));
      break;
    case 3:
      rval = Global::kernel->rootfs->type(createPathFromString(frame->ebx));
      break;
    case 4:
      rval = currentProc->allocate(frame->ebx);
      break;
    case 5:
      currentProc->deallocate(frame->ebx);
      break;
    case 6:
      break;
    case 7: {
      asm("cli");
      Process* p = Global::kernel->createProcess(frame->ebx);
      rval = p->PID;
      asm("sti");
      break;
    }
    case 8:
      rval = currentProc->PID;
      break;
    
    case 9:
      break;

    case 10: {
      if (frame->ecx == 1) {
        ReadWriter* stdin = currentProc->stdin;
        if (!stdin)
          break;
        
        rval = stdin->read(frame->ebx, frame->edx);
      } else {
        xnoe::Maybe<ReadWriter*> fh = Global::FH->get(frame->ecx);
        if (!fh.is_ok()) {
          rval = 0;
          break;
        }
        
        ReadWriter* rw = fh.get();
        rval = rw->read(frame->ebx, frame->edx);
      }
      break;
    }

    case 11: {
      if (frame->ecx == 0) {
        ReadWriter* stdout = currentProc->stdout;
        if (!stdout)
          break;
        
        rval = stdout->write(frame->ebx, frame->edx);
      } else {
        xnoe::Maybe<ReadWriter*> fh = Global::FH->get(frame->ecx);
        if (!fh.is_ok()) {
          rval = 0;
          break;
        }
        
        ReadWriter* rw = fh.get();
        rval = rw->write(frame->ebx, frame->edx);
      }
      break;
    }

    case 12:
      if (currentProc->stdin)
        break;
      
      currentProc->stdin = new CircularRWBuffer(currentProc->PID, 0);
      Global::kernel->KBListeners.append(currentProc);
      break;
    
    case 13: {
      xnoe::Maybe<Process*> pm = Global::kernel->pid_map->get(frame->ebx);
      if (!pm.is_ok())
        break;
      Process* p = pm.get();
      if (!p->stdout) {
        ReadWriter* buffer = new CircularRWBuffer(currentProc->PID, frame->ebx);
        p->stdout = buffer;
        rval = Global::kernel->mapFH(buffer);
      }
      break;
    }

    case 14: {
      xnoe::Maybe<Process*> pm = Global::kernel->pid_map->get(frame->ebx);
      if (!pm.is_ok())
        break;
      Process* p = pm.get();
      if (!p->stdin) {
        ReadWriter* buffer = new CircularRWBuffer(frame->ebx, currentProc->PID);
        p->stdin = buffer;
        rval = Global::kernel->mapFH(buffer);
      }
      break;
    }

    case 15: {
      ReadWriter* file = Global::kernel->rootfs->open(createPathFromString(frame->ebx));
      if (file)
        rval = Global::kernel->mapFH(file);
      break;
    }

    case 16: {
      xnoe::Maybe<ReadWriter*> f = Global::FH->get(frame->ebx);
      if (f.is_ok()) {
        delete f.get();
        Global::kernel->unmapFH(frame->ebx);
      }
      break;
    }

    case 17: {
      asm("cli");
      xnoe::Maybe<Process*> p = Global::kernel->pid_map->get(frame->ebx);
      if (p.is_ok()) {
        Process* proc = p.get();
        Global::kernel->destroyProcess(proc);
      }
      asm("sti");
      break;
    }

    case 18: {
      Global::currentProc->state = Suspended;
      Timer::register_event(frame->ebx, &awaken, (void*)Global::currentProc, true);
      context_switch(frame);
      break;
    }

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
  
  gates[32] = &Timer::tick;
  gates[0] = &handle_fault;
  gates[5] = &handle_fault;
  gates[6] = &handle_fault;
  gates[7] = &handle_fault;
  gates[8] = &handle_fault;
  gates[9] = &handle_fault;
  gates[10] = &handle_fault;
  gates[11] = &handle_fault;
  gates[12] = &handle_fault;
  gates[13] = &handle_fault;
  gates[14] = &handle_fault;
  gates[16] = &handle_fault;
  gates[17] = &handle_fault;
  gates[19] = &handle_fault;
  gates[20] = &handle_fault;
  gates[21] = &handle_fault;
  gates[29] = &handle_fault;
  gates[30] = &handle_fault;
  gates[31] = &handle_fault;
  gates[128] = &syscall;

  idt[128].privilege = 3;

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

  // Program the PIT
  uint16_t counter = 1193;
  uint8_t* _counter = (uint8_t*)&counter;
  outb(0x40, _counter[0]);
  outb(0x40, _counter[1]);

  Timer::register_event(30, &context_switch, 0);
}

void enable_idt() {
  asm ("sti");
}