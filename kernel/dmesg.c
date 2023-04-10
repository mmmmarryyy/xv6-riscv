#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"

#define SIZE_OF_DMESG_BUFFER (PGSIZE * 2)

struct {
  int position;
  struct spinlock lock;
  char buffer[SIZE_OF_DMESG_BUFFER];
} dmesg_buffer;

void dmesg_init() {
  initlock(&dmesg_buffer.lock, "dmesg spinlock");
  dmesg_buffer.position = 0;
}

void write_string(const char *message) {
  int length; 

  if (strlen(message) < SIZE_OF_DMESG_BUFFER - dmesg_buffer.position - 1) {
    length = strlen(message);
  } else {
    length = SIZE_OF_DMESG_BUFFER - dmesg_buffer.position - 1;
  } 

  for (int ind = 0; ind < length; ind++) {
    dmesg_buffer.buffer[dmesg_buffer.position + ind] = message[ind];
  }

  dmesg_buffer.position += length;
}

void tostring(char str[], int num, int len) {
    int i, rem;
    
    for (i = 0; i < len; i++) {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

void write_uint(uint number) {
  if (number == 0) {
    return write_string("0");
  }

  char str_buff[16];
  char rev_str_buff[16];

  rev_str_buff[0] = '0';
  int pointer = 1;

  while (number > 0 && pointer < 16) {
    rev_str_buff[pointer++] = '0' + (number % 10);
    number /= 10;
  }

  for (int i = pointer - 1; i >= 0; i--) {
    str_buff[pointer-i] = rev_str_buff[i];
  }
  
  str_buff[pointer+1] = '0';

  write_string(str_buff);
}

void pr_msg (const char *message) {   
  acquire(&dmesg_buffer.lock);

  acquire(&tickslock);
  uint tick = ticks;
  release(&tickslock);

  write_string("Time: ");
  write_uint(tick);
  write_string(" ticks; ");
  write_string(message);
  write_string("\n");
   
  release(&dmesg_buffer.lock);
}

uint64 
sys_dmesg(void) 
{
  for (int i = 0; i < dmesg_buffer.position; i++) {
    consputc(dmesg_buffer.buffer[i]);
  }
  return 0;
}