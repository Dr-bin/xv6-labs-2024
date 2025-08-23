// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem, supermem;
// 为链表新增超级页实例

// 新增的init函数，初始化超级页的空闲链表
void superinit(){
  initlock(&supermem.lock, "supermem");
  char *p;
  p = (char*)SUPERPGROUNDUP((uint64)SUPERBASE);
  printf("superinit: start=%p, end=%lx\n", p, PHYSTOP);

  for(; p + SUPERPGSIZE <= (char*)PHYSTOP; p += SUPERPGSIZE){
    printf("superfree: %p\n", p);
    superfree(p);
  }
}

void
kinit()
{
  printf("kinit: freeing range %p to %p\n", end, (void*)SUPERBASE);
  printf("superinit: superbase=%lx, phystop=%lx\n", SUPERBASE, PHYSTOP);
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)SUPERBASE);
  superinit();
}

void 
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  printf("freerange: from %p to %p\n", p, pa_end);
  int count = 0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
    count++;
  }
  printf("freerange: freed %d pages\n", count);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

//free超级页
void superfree(void *pa) {
  // printf("superfree: freeing superpage at %p\n", pa);
  struct run *r;
  // 改成超级页的尺寸
  if(((uint64)pa % SUPERPGSIZE) != 0 || (uint64)pa < SUPERBASE || (uint64)pa >= PHYSTOP)
    panic("superfree");
  // memset，将内存为都置1，表示处于空闲中
  memset(pa, 1, SUPERPGSIZE);
  
  r = (struct run*)pa;
  // 加锁，保证链表插入顺序
  acquire(&supermem.lock);
  r->next = supermem.freelist;
  supermem.freelist = r;
  release(&supermem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void* superalloc(void) {
  struct run *r;
  // 获取锁
  acquire(&supermem.lock);
  r = supermem.freelist;
  if(r){
    // printf("Allocating superpage at: 0x%lx\n", (uint64)r);
    //更新空闲页链表
    supermem.freelist = r->next;
  }
  // else{
  //   printf("superalloc: no free superpages available\n");
  // }
  release(&supermem.lock);

  if(r){
    memset((char*)r, 5, SUPERPGSIZE); // fill with junk
    // printf("superalloc: allocated superpage at %p\n", r);
  }
  return (void*)r; 
}

int count_free_pages() {
  int count = 0;
  acquire(&kmem.lock);
  struct run *r = kmem.freelist;
  while(r) {
    count++;
    r = r->next;
  }
  release(&kmem.lock);
  return count;
}
