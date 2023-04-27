#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"

#define SIZE_OF_DMESG_BUFFER (PGSIZE * 2)

static struct spinlock lock;
static const char *digits = "0123456789ABCDEF";

static char dmesg_buffer[SIZE_OF_DMESG_BUFFER];
static int begin_ptr = 0;
static int end_ptr = 0;

static int flags[3] = {1, 1, 1};

static int words_counter = 0;

void dmesg_init() {
    initlock(&lock, "dmesg spinlock");
}

int check_switch() {
   return flags[0];
}

int check_inter() {
   return flags[1];
}

int check_syscall() {
   return flags[2];
}

int update_begin_ptr() {
    int prev = begin_ptr;
    begin_ptr += 1;
    begin_ptr %= SIZE_OF_DMESG_BUFFER;
    return prev;
}

int update_end_ptr() {
    int prev = end_ptr;
    end_ptr += 1;
    end_ptr %= SIZE_OF_DMESG_BUFFER;
    return prev;
}

static void my_write_char(char c) {
    dmesg_buffer[update_end_ptr()] = c;

    if (begin_ptr == end_ptr) {
        begin_ptr++;

        if (words_counter == 0) {
            return;
        }

        while (dmesg_buffer[update_begin_ptr()] != '\0') {
            continue;
        }

        words_counter--;
    }
}

static void my_write_ptr(uint64 ptr) {
    my_write_char('0'); my_write_char('x');

    for (int i = 0; i < (sizeof(uint64) * 2); i++) {
        my_write_char(digits[(ptr >> (sizeof(uint64) * 8 - 4 * (i+1))) & 15]);
    }
}

void my_write_int(int number, int base) {
    if (number < 0) {
        my_write_char('-');
        number *= -1;
    }

    if (number == 0) {
        my_write_char('0');
        return;
    }

    char str_buff[16];
    int pointer = 0;

    while (number > 0 && pointer < 16) {
        str_buff[pointer++] = digits[number % base];
        number /= base;
    }

    pointer--;

    while (pointer > 0) {
        my_write_char(str_buff[pointer--]);
    }
}

void my_write_string(const char* message) { 
    int len = sizeof(message);
    for (int i = 0; i < len; i++) {
        my_write_char(message[i]);
    }
}

void pr_msg(const char *fmt, ...) {
    acquire(&lock);

    //acquire(&tickslock);
    uint tick = ticks;
    //release(&tickslock);

    my_write_string("Time: ");
    my_write_int(tick, 10);
    my_write_string(" ticks; ");

    va_list message;
    va_start(message, fmt);

    const char *current_string;

    while (*fmt) {
        if (*fmt != '%') {
            my_write_char(*fmt++);
            continue;
        }

        char current_char = *(++fmt);

        if (!current_char) {
            break;
        }

        if (current_char == '%') {
            my_write_char('%');
        } else if (current_char == 'p') {
            my_write_ptr(va_arg(message, uint64));
        } else if (current_char == 'x') {
            my_write_int(va_arg(message, int), 16);
        } else if (current_char == 'd') {
            my_write_int(va_arg(message, int), 10);
        } else if (current_char == 's') {
            if (!(current_string = va_arg(message, const char*))) {
                current_string = "(null)";
            }

            my_write_string(current_string);
        } else {
            my_write_char('%');
            my_write_char(current_char);
        }

        fmt++;
    }

    va_end(message);

    my_write_char('\0');
    words_counter++;

    release(&lock);
}


uint64 sys_dmesg(void)
{
    if (begin_ptr < end_ptr) {
        for (int i = begin_ptr; i < end_ptr; i++) {
            consputc(dmesg_buffer[i]);
        }
    } else {
        for (int i = begin_ptr; i < SIZE_OF_DMESG_BUFFER; i++) {
            consputc(dmesg_buffer[i]);
        }

        for (int i = 0; i < end_ptr; i++) {
            consputc(dmesg_buffer[i]);
        }
    }

    return 0;
}

uint64 sys_set_flags(void)
{
    acquire(&lock);

    argint(0, &flags[0]);
    argint(1, &flags[1]);
    argint(2, &flags[2]);

    release(&lock);

    return 0;
}