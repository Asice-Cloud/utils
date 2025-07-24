#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <vector>
using namespace std;

constexpr uint32_t STACK_SIZE = 4 * 1024 * 1024; // 4MiB

struct saved_regs_t {
  uint64_t rbx;
  uint64_t rbp;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
};

struct context_t {
  saved_regs_t saved_regs;
};

struct coroutine_t {
  uintptr_t stack;
  void (*entry)(void *);
  void *arg;
  struct context_t *context;
  char *name;
};

// initial the context before we switch context
static coroutine_t main_co = {
    .stack = 0, .entry = nullptr, .arg = nullptr, .context = nullptr};

static coroutine_t *current_co = &main_co;
static vector<coroutine_t *> exited_co;
void coroutine_yield();

__attribute__((naked)) void my_switch(context_t **ret_from, context_t *to) {
  // push, pop 指令也是可以的,
  __asm__("sub $48, %rsp \n\t"
          "mov %rbx, 0(%rsp)\n\t"
          "mov %rbp, 8(%rsp)\n\t"
          "mov %r12, 16(%rsp)\n\t"
          "mov %r13, 24(%rsp)\n\t"
          "mov %r14, 32(%rsp)\n\t"
          "mov %r15, 40(%rsp)\n\t"

          // from->saved_regs = rsp
          "mov %rsp, 0(%rdi)\n\t");
  __asm__("mov %rsi, %rsp\n\t"
          "mov 0(%rsp), %rbx\n\t"
          "mov 8(%rsp), %rbp\n\t"
          "mov 16(%rsp), %r12\n\t"
          "mov 24(%rsp), %r13\n\t"
          "mov 32(%rsp), %r14\n\t"
          "mov 40(%rsp), %r15\n\t"
          "add $48, %rsp\n\t");

  __asm__("ret");
}

extern "C" void coroutine_exit() {
  coroutine_t *current = current_co;
  exited_co.push_back(current);
  cout << format("coroutine_exit: {}\n", current->name);
  coroutine_yield();
}

__attribute__((naked)) void coroutine_bootstrap() {
  __asm__("pop %rax\n\t" // 将入口地址放在了栈顶
                         // 取出entry
          "mov 8(%rax), %rbx \n\t"
          // arg
          "mov 16(%rax), %rdi \n\t"
          // call
          "call *%rbx\n\t"

          "call coroutine_exit\n\t");
}

coroutine_t *make_coroutine(void (*entry)(void *), void *arg,
                            const char *name = "NULL") {
  uintptr_t stack_end = (uintptr_t)malloc(STACK_SIZE);
  memset((void *)stack_end, 0, STACK_SIZE);

  uintptr_t stack_start = stack_end + STACK_SIZE;

  uintptr_t rsp = stack_start;
  rsp -= sizeof(saved_regs_t);

  coroutine_t *co = (coroutine_t *)rsp;

  *co = (coroutine_t){.stack = stack_end,
                      .entry = entry,
                      .arg = arg,
                      .context = nullptr,
                      .name = (char *)name};

  rsp -= 8;
  *(uintptr_t *)rsp = (uintptr_t)co;

  rsp -= 8;
  *(uintptr_t *)rsp = (uintptr_t)coroutine_bootstrap;

  rsp -= sizeof(saved_regs_t);
  co->context = (context_t *)rsp;

  co->context->saved_regs.rbp = rsp + sizeof(saved_regs_t) + 8 * 2;

  return co;
}

void coroutine_destroy(coroutine_t *co) { free((void *)co->stack); }

void coroutine_gc() {
  for (auto co : exited_co) {
    coroutine_destroy(co);
  }
  exited_co.clear();
}

void coroutine_yield() { my_switch(&current_co->context, main_co.context); }

void coroutine_resume(coroutine_t *co) {
  current_co = co;
  my_switch(&main_co.context, co->context);
}

void test_coroutine_func(void *arg) {
  int *n = (int *)arg;
  cout << format("test_coroutine_func: {}\n", *n);
  coroutine_yield();
  cout << "test_coroutine_func: after yield\n";
}

int main(int argc, char *argv[]) {
  int n = 1;
  int m = 2;
  coroutine_t *co = make_coroutine(test_coroutine_func, &n, "co1");
  coroutine_t *co2 = make_coroutine(test_coroutine_func, &m, "co2");
  coroutine_resume(co);
  coroutine_resume(co2);
  cout << "main: after resume\n";
  coroutine_resume(co);
  coroutine_resume(co2);

  coroutine_gc();
  return 0;
}
