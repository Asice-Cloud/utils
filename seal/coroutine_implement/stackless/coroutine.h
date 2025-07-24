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
// 因为 co_next 调用之后，它的栈帧并不会被清零，只是逻辑上不再使用而已。
// 因此如果下一次调用依然是 co_next，则栈帧的数据依然时沿用上次的。
// 所以产生了局部变量被“保存”的效果。
// 稍作修改，就破坏了栈帧，证明局部变量并不能被稳定保存。
// 最安全的做法是利用协程结构体存放要在不同状态共享的数据.
// Note:这么一来就变成了，堆上的变量在协程中变为了栈上变量
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
// 局部变量保存在栈帧里，如果我们可以通过计算栈帧的大小和位置，将其暂存，\
// 等重新运行的时候再取出恢复，就可以获得局部变量了
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
