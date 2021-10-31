#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

void
push(struct PriorityQueue* q, struct proc* p) {
  q->queue[q->front++] = p;
  q->front %= QSIZE;
  if (q->front == q->back) {
    panic("Full queue push");
  }
  p->queuestate = QUEUED;
}

struct proc*
pop(struct PriorityQueue* q)
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
remove(struct PriorityQueue* q, struct proc* p) {
  if (p->queuestate == NOTQUEUED) return;
  for (int i = q->back; i != q->front; i = (i + 1) % QSIZE) {
    if (q->queue[i] == p) {
      p->queuestate = NOTQUEUED;
      for (int j = i + 1; j != q->front; j = (j + 1) % QSIZE) {
        q->queue[(j - 1 + QSIZE) % QSIZE] = q->queue[j];
      }
      q->front = (q->front - 1 + QSIZE) % QSIZE;
      break;
    }
  }
}

int
empty(struct PriorityQueue q) {
  return (q.front - q.back + QSIZE) % QSIZE == 0;
}
