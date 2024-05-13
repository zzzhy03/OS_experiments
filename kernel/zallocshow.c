// ZHY memory allocator, for user processes,
// kernel stacks, Bytes,
// and pipe buffers. Allocates whole 16MB Memory.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

struct run {
  struct run *next, *prev;
  int siz;
  int isFree;
};

extern struct {
  struct spinlock lock;
  struct run *freelist;
} zmem;

void show(){
  struct run *r;
  acquire(&zmem.lock);
  r = zmem.freelist;
  release(&zmem.lock);
  while(r != 0){
    printf("%p %d %d\n", r, r->siz, r->isFree);
    r = r->next;
  }
  printf("\n");
  return;
}
int zallocshow(){
    show();
    void *p1 = zalloc(34);
    show();
    void *p2 = zalloc(20);
    show();
    void *p3 = zalloc(50);
    show();
    zfree(p1);
    show();
    p1 = zalloc(30);
    show();
    zfree(p1);
    show();
    p1 = zalloc(5);
    show();
    zfree(p3);
    show();
    zfree(p2);
    show();
    zfree(p1);
    show();
    return 0;
}
