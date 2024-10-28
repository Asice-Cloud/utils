/**
 * Note: only support x86_64 with -O0 flag, and if you find any bug, please let
 * me know.
 */

#ifndef __TINY_CO_H__
#define __TINY_CO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  _CO_STATE_INIT,
  _CO_STATE_RUN,
  _CO_STATE_YIELD,
  _CO_STATE_FIN
} _CoState;

struct Co;

typedef int (*CoTask)(struct Co *co, void *data);

typedef struct Co {
  _CoState state;
  CoTask func;
  void *data;
  int label;
  void *stack;
  size_t stack_size;
  void *stack_base;
} Co;

#define CO_BEGIN(co)                                                           \
  switch ((co)->label) {                                                       \
  case 0:

#define CO_YIELD(co, value)                                                    \
  do {                                                                         \
    (co)->label = __LINE__;                                                    \
    asm volatile("" ::: "memory"); /* Memory barrier to prevent reordering */  \
    asm volatile("mov %%rsp, %0\n\t"                                           \
                 "mov %%rbp, %1"                                               \
                 : "=g"((co)->stack), "=g"((co)->stack_base));                 \
    (co)->stack_size = (char *)(co)->stack_base - (char *)(co)->stack;         \
    (co)->stack = malloc((co)->stack_size);                                    \
    memcpy((co)->stack, (void *)((char *)(co)->stack_base - (co)->stack_size), \
           (co)->stack_size);                                                  \
    (co)->state = _CO_STATE_YIELD;                                             \
    return (value);                                                            \
  case __LINE__:                                                               \
    memcpy((void *)((char *)(co)->stack_base - (co)->stack_size), (co)->stack, \
           (co)->stack_size);                                                  \
    free((co)->stack);                                                         \
    (co)->stack = NULL;                                                        \
    asm volatile("mov %0, %%rsp\n\t"                                           \
                 "mov %1, %%rbp"                                               \
                 :                                                             \
                 : "g"((char *)(co)->stack_base - (co)->stack_size),           \
                   "g"((co)->stack_base));                                     \
    asm volatile("" ::: "memory"); /* Memory barrier to prevent reordering */  \
  } while (0)

#define CO_END(co)                                                             \
  (co)->state = _CO_STATE_FIN;                                                 \
  }

void co_init(Co *co, CoTask func, void *data) {
  co->state = _CO_STATE_INIT;
  co->func = func;
  co->data = data;
  co->label = 0;
  co->stack = NULL;
  co->stack_size = 0;
  co->stack_base = NULL;
}

int co_next(Co *co) {
  if (co->state == _CO_STATE_FIN) {
    return 0;
  }
  co->state = _CO_STATE_RUN;
  return co->func(co, co->data);
}

int co_stop(Co *co) {
  co->state = _CO_STATE_FIN;
  return 0;
}

int co_done(Co *co) { return co->state == _CO_STATE_FIN; }

#endif // __TINY_CO_H__
