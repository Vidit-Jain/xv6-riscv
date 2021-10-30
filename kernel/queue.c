#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

void
push(struct Queue* q, struct proc* p) {
//  printf("HELLO\n");
  q->queue[q->front++] = p;
  if (q->front == q->back) {
    panic("Full queue push");
  }
  q->front %= QSIZE;
  p->queuestate = QUEUED;
}

struct proc*
pop(struct Queue* q)
{
  if (q->back == q->front) {
    panic("Empty queue pop");
  }
  struct proc* p = q->queue[q->back];
  p->queuestate = NOTQUEUED;
  q->back++;
  q->back %= QSIZE;
  return p;
}

void
remove(struct Queue* q, struct proc* p) {
  if (p->queuestate == NOTQUEUED) return;
  for (int i = q->back; i != q->front - 1; i = (i + 1) % QSIZE) {
    if (q->queue[i] == p) {
      p->queuestate = NOTQUEUED;
      for (int j = i + 1; j != q->front - 1; j = (j + 1) % QSIZE) {
        q->queue[(j - 1 + QSIZE) % QSIZE] = q->queue[j];
      }
      q->front--;
      break;
    }
  }
}

int
empty(struct Queue q) {
  return (q.front - q.back + QSIZE) % QSIZE == 0;
}
