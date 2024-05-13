// ZHY memory allocator, for user processes,
// kernel stacks, Bytes,
// and pipe buffers. Allocates whole 16MB Memory.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

struct run {
  struct run *next, *prev;
  int siz;
  int isFree;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} zmem;

void
zinit()
{
  initlock(&zmem.lock, "zmem");
  struct run *first = (void*)ZALLOCSTART;
  first->next = first->prev = 0;
  first->isFree = 1;
  first->siz = ZALLOCOFFSET;
  acquire(&zmem.lock);
  zmem.freelist = first;
  release(&zmem.lock);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to zalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void
zfree(void *pa)
{
  struct run *r;
  acquire(&zmem.lock);
  r = zmem.freelist;
  release(&zmem.lock);

  while(r != 0){
    if(( (uint64)pa - ZALLOCINFO) == (uint64)r){
        r->isFree = 1;
        if(r->next != 0 && r->next->isFree == 1){
            r->siz += r->next->siz;
            r->next->siz = 0;
            r->next = r->next->next;
            if(r->next != 0)r->next->prev = r;
        }
        if(r->prev != 0 && r->prev->isFree == 1){
            r->prev->siz += r->siz;
            r->siz = 0;
            r->prev->next = r->next;
            if(r->next != 0)r->next->prev = r->prev;
        }
        return;
    }
    r = r->next;
  }
  panic("zfree");
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
zalloc(int nbyte)
{
  struct run *r;
  acquire(&zmem.lock);
  r = zmem.freelist;
  release(&zmem.lock);

  while(r != 0){
    if(r->isFree==1 && r->siz >= (nbyte + ZALLOCINFO)){
        if(r->siz - (nbyte + ZALLOCINFO) > 24){
            struct run *p = (struct run*)((uint64)r + (nbyte + ZALLOCINFO)); 
            r->isFree = 0; p->isFree = 1;
            p->siz = r->siz - (nbyte + ZALLOCINFO);
            r->siz = nbyte + ZALLOCINFO;
            p->next = r->next; 
            if(r->next != 0)r->next->prev = p;
            r->next = p; p->prev = r;
        }else{
            r->isFree = 0;
        }
        return (void*)((uint64)r+ZALLOCINFO);
    }
    r = r->next;
    
  }
  return 0;
}