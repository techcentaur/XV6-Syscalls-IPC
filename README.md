# XV6-Syscalls-IPC

**Introducing new system calls and inter-process-communication in xv6**

**xv6** is a teaching operating system, that is most likely be helpful for students in learning the basics of operating system. It contains the most important and basic parts of an operating system. See more about it [here](https://pdos.csail.mit.edu/6.828/2012/xv6.html).

## How to add a system call that can be called in xv6's shell?
	- `sysproc.c`: Add the real implementation of your method here (or can do in `sysfile.c` as well)
	- `syscall.c`: External define the function that connect the shell and the kernel, use the position defined in `syscall.h` to add the function to the system call vector
	- `syscall.h`: Define the position of the system call vector that connect to your implementation
	- `user.h`: Define the function that can be called through the shell
	- `usys.S`: Use the macro to define connect the call of user to the system call function

See a similar type of question on stack overflow [here](https://stackoverflow.com/questions/8021774/how-do-i-add-a-system-call-utility-in-xv6).

## Implementing Syscalls in xv6

1. Implementing a system call trace: It will have two states: ON and OFF. Let us define it with a new system call `sys_print_count`. It will print the list of system calls that have been invoked since the last transition to the ON state with their counts.

For e.g.:

sys_fork 10 </br>
sys_read 12 </br>
sys_write 1 </


```C
// actual function goes in `sysproc.c`
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


// add one more system call in `user.h`
int print_count(void);
// and in `usys.S`
SYSCALL(print_count)
// and in syscall.h: With a unqiue integer (incremented from the previous one)
#define SYS_print_count 22 // (22nd system call)



// Initialise variables as extern in sysproc.c, so that we can use it inter-file
extern int call_count_history[25];
extern char *call_name_history[25];



/* modify syscall.c to count the number of times a sys_call has called */
// 1. Initialise variables
int toggle = 0;
int call_count_history[25] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char *call_name_history[25] = {"sys_fork", "sys_exit", "sys_wait", "sys_pipe", "sys_read", "sys_kill", "sys_exec", "sys_fstat", "sys_chdir", "sys_dup", "sys_getpid", "sys_sbrk", "sys_sleep", "sys_uptime", "sys_open", "sys_write", "sys_mknod", "sys_unlink", "sys_link", "sys_mkdir", "sys_close", "sys_print_count"};


void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
  
    // add up in number of times a sys call has appeared
    call_count_history[num-1] += 1;
	// ...

    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}


```

