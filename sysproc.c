#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

extern int toggle;
extern int call_count_history[26];
extern char *call_name_history[26];
extern void get_ps();

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_print_count(void)
{
  int i;
  for(i=0; i<25; i++){
    if(call_count_history[i] != 0){
      cprintf("%s %d\n", call_name_history[i], call_count_history[i]);
    }
  }
  return 0;
}

int
sys_toggle(void)
{
  int i;
  if(toggle==0){
    toggle = 1;
  }
  else{
    toggle = 0;
    for(i=0; i<25; i++){
      call_count_history[i] = 0;
    }
  }

  return 0;
}

int
sys_print_toggle(void)
{
  if(toggle == 0){
    cprintf("TOGGLE OFF\n");
  }
  else{
    cprintf("TOGGLE ON\n");
  }

  return 0;
}

int
sys_add(void)
{
  int a; int b;
  argint(0, &a);
  argint(1, &b);
  return a+b;
}

int
sys_ps(void)
{
  get_ps();
  return 0;
}