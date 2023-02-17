#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

extern int trace_on;
extern int call_count_history[29];
extern char *call_name_history[29];
int sys_call_ordered[29] = {22, 8, 20, 9, 6, 1, 0, 7, 10, 21, 5, 18, 19, 16, 14, 3, 25, 24, 4, 27, 11, 26, 28, 12, 23, 17, 13, 2, 15};

extern void proclist();
extern void send_message();
extern int receive_message();

//For multicasting
extern void send_to_multi();

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

// user - defined system call
int
sys_getyear(void)
{
  return 2001;
}

int 
sys_add(void)
{ 
  int a, b;

  if(argint(0, &a) < 0) return -1;
  if(argint(1, &b) < 0) return -1;
  
  return a + b;
  
}

int
sys_toggle(void)
{
   int k;
   if(trace_on == 0) trace_on = 1;
   else{
   	trace_on = 0;
   	for(k=0; k<29; k++){
      		call_count_history[k] = 0;
   	}
   }
   //cprintf("Trace on %d\n", trace_on);
    
   return 0;
}

int 
sys_ps(void){

   proclist();
   return 1;
}

int
sys_print_count(void)
{
  int i;
  for(i=0; i<29; i++){
    int num = sys_call_ordered[i];
    if(call_count_history[num] != 0){
      cprintf("%s %d\n", call_name_history[num], call_count_history[num]);
    }
  }
  return 0;
}

int
sys_send(void)
{
  int s_id, r_id; 
  char *message;

  if(argstr(2, &message) < 0) return -1;
  if(argint(0, &s_id) < 0) return -1;
  if(argint(1, &r_id) < 0) return -1;

  // cprintf("%d\n", message);

  send_message(s_id, r_id, message);
  return 0;
}

int sys_recv(void)
{
  char *message;
  
  if(argstr(0, &message) < 0) return -1;

  return receive_message(message);
  //return 0;
}

int sys_send_multi(void){
  int s_id;
  int* r_ids; 
  char *r_ids_char;
  char *message;
  int len = 8;

  if(argint(0, &s_id) < 0) return -1;
  
  //if(argptr2(1, &r_ids, length_of_ptr*sizeof(int)) < 0) return -1;
  if(argptr(1, &r_ids_char, len*sizeof(int)) < 0) return -1;
  r_ids = (int *)r_ids_char;
  
  if(argstr(2, &message) < 0) return -1;
  
  send_to_multi(s_id, r_ids, message);
  return 1;
}
