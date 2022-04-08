#include "process.h"

extern void(*catchall_return)();

AllocTracker::AllocTracker(void* base, uint32_t size, uint32_t count) : page_base(base), page_size(size), alloc_count(count) {}

xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> Process::get_alloc_tracker(uint32_t address) {
  xnoe::linkedlistelem<AllocTracker>* current = this->allocations.start;
  while (current) {
    if (current->elem.page_base <= address && (current->elem.page_base + 4096 * current->elem.page_size) > address) {
      return xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*>(current);
    }
    current = current->next;
  }
  
  return xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*>();
}

Process::Process(uint32_t PID, void* stack, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
: Allocator(page_directory, phys, virt, virt_alloc_base) {
  this->PID = PID;
  this->page_remaining = 0;
  this->last_page_pointer = virt_alloc_base;
  this->stack = stack;
  this->state = Running;
}

Process::Process(uint32_t PID)
: Allocator(new PageDirectory, new PageMap, (uint32_t)0, 3) {
  this->PID = PID;
  this->page_remaining = 0;
  this->last_page_pointer = 0;
  this->stack = this->allocate(0x8000);
  this->kernelStackPtr = (new uint8_t[0x1000]) + 0x1000;
  this->state = Running;
}

Process::Process(uint32_t PID, PageDirectory* inherit, uint32_t inheritBase, uint32_t fh)
: Allocator(new PageDirectory, new PageMap, (uint32_t)0, 3) {
  this->stdout = 0;
  this->stdin = 0;

  this->firstRun = true;

  this->PID = PID;
  this->page_remaining = 0;
  this->last_page_pointer = 0;
  this->state = Running;

  for (int index = inheritBase >> 22; index < 1024; index++)
    this->PD->page_directory[index] = inherit->page_directory[index];

  xnoe::Maybe<ReadWriter*> file = Global::FH->get(fh);
  if (file.is_ok()) {
    ReadWriter* filereader = file.get();
    uint32_t filesize = filereader->size();
    uint8_t* program_data = this->allocate(filesize + 12) + 12;

    this->stack = this->allocate(0x8000);
    this->kernelStackPtr = (new uint8_t[0x1000]) + 0xffc;
    this->kernelStackPtrDefault = this->kernelStackPtr;

    uint32_t pCR3;
    asm ("mov %%cr3, %0" : "=a" (pCR3) :);
    this->PD->select();

    // We also need to initialise ESP and the stack
    uint32_t* stack32 = ((uint32_t)this->kernelStackPtr);
    *(--stack32) = 0x23; // SS
    *(--stack32) = ((uint32_t)this->stack + 0x8000); // ESP
    *(--stack32) = 0x200; // EFLAGS
    *(--stack32) = 27; // CS
    *(--stack32) = (uint32_t)program_data; // EIP
    *(--stack32) = ((uint32_t)this->stack + 0x8000); // EBP

    uint32_t rEBP = stack32;

    *(--stack32) = 0;    // EAX
    *(--stack32) = 0;    // ECX
    *(--stack32) = 0;    // EDX
    *(--stack32) = 0;    // EBX
    *(--stack32) = 0;    // ESP
    *(--stack32) = rEBP; // EBP
    *(--stack32) = 0;    // ESI
    *(--stack32) = 0;    // EDI

    this->kernelStackPtr = stack32;

    filereader->read(filesize, program_data);

    asm ("mov %0, %%cr3" : : "r" (pCR3));
  }
}

Process::~Process() {
  uint32_t pCR3;
  asm ("mov %%cr3, %0" : "=a" (pCR3) :);
  this->PD->select();
  xnoe::linkedlistelem<AllocTracker>* next = allocations.start;
  while (next) {
    xnoe::linkedlistelem<AllocTracker>* active = next;
    next = next->next;

    //printf("Deleted %x\n", active->elem.page_base);

    this->deallocate(active->elem.page_base+1);
  }
  this->deallocate(stack);
  asm ("mov %0, %%cr3" : : "r" (pCR3));
  delete kernelStackPtr;
}

void* Process::allocate(uint32_t size) {
  bool switched_PD = false;
  uint32_t pCR3;
  asm ("mov %%cr3, %0" : "=a" (pCR3) :);
  if (Global::currentProc != this) {
    switched_PD = true;
    this->PD->select();
  }
  void* ptr;
  // Determine if there's enough space to just allocate what's been requested
  if (size < this->page_remaining) {
    xnoe::linkedlistelem<AllocTracker>* alloctracker = this->allocations.end;
    alloctracker->elem.alloc_count += 1;
    ptr = this->last_page_pointer + (4096 - this->page_remaining);
    this->page_remaining -= size;
  } else {
    uint32_t elem_size = sizeof(xnoe::linkedlistelem<AllocTracker>);
    size += elem_size;

    // Determine how many pages we'll allocate, and the remainder;
    uint32_t pages = size / 4096;
    uint32_t remainder = 4096 - (size % 4096);

    ptr = this->Allocator::allocate(size);

    // Update local values
    this->last_page_pointer = ptr + pages * 4096;
    this->page_remaining = remainder;

    // Create allocations entry
    xnoe::linkedlistelem<AllocTracker>* elem = (xnoe::linkedlistelem<AllocTracker>*)ptr;
    elem->next = 0;
    elem->prev = 0;
    elem->elem = AllocTracker(ptr, pages + 1, 1);
    this->allocations.append(elem);

    ptr += elem_size;
  }

  asm ("mov %0, %%cr3" : : "r" (pCR3));

  return ptr;
}

void Process::deallocate(uint32_t virt_addr) {
  xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> alloc_tracker = this->get_alloc_tracker(virt_addr);
  if (alloc_tracker.is_ok()) {
    AllocTracker* ac = &alloc_tracker.get()->elem;
    ac->alloc_count--;
    if (ac->alloc_count == 0) {
      void* base = ac->page_base;
      uint32_t count = ac->page_size;

      this->allocations.remove(alloc_tracker.get());

      for (int i=0; i<count; i++)
        Allocator::deallocate(base + (4096 * i));
      
      this->page_remaining = 0;
    }
  }
}

uint32_t Process::count_allocations(uint32_t address) {
  xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> alloc_tracker = this->get_alloc_tracker(address);

  if (alloc_tracker.is_ok())
    return alloc_tracker.get()->elem.alloc_count;
  else
    return 0;
}