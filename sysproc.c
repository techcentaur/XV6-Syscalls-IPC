#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// include IPC
#include "ipc.h"

extern int toggle;
extern int call_count_history[28];
extern char *call_name_history[28];

extern void get_ps();
extern void send_message();
extern void receive_message();

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
  int j;
  for(j=0; j<28; j++){
    if(call_count_history[j] != 0){
      cprintf("%s %d\n", call_name_history[j], call_count_history[j]);
    }
  }
  return 0;
}

int
sys_toggle(void)
{
  int k;
  if(toggle==0){
    toggle = 1;
    for(k=0; k<28; k++){
      call_count_history[k] = 0;
    }
  }
  else{
    toggle = 0;
    for(k=0; k<28; k++){
      call_count_history[k] = 0;
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

int
sys_send(void)
{
  int s_id, r_id; 
  char *message;

  // char *msg = (char *)message;

  if(argstr(2, &message) < 0){
    return -1;
  }

  if(argint(0, &s_id) < 0){
    return -1;
  }
  if(argint(1, &r_id) < 0){
    return -1;
  }
  
  send_message(s_id, r_id, message);
  return 0;
}

int sys_recv(void)
{
  char *message;

  if(argstr(0, &message) < 0){
    return -1;
  }

  receive_message(message);
  return 0;
}