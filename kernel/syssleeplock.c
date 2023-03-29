#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"

#define SYSSLEEPLOCK_INIT     0
#define SYSSLEEPLOCK_DELETE   1
#define SYSSLEEPLOCK_ACQUIRE  2
#define SYSSLEEPLOCK_RELEASE  3

struct {
  char sl_active[MAXSLEEPLOCKS];
  struct spinlock lock;
  struct sleeplock sleeplocks[MAXSLEEPLOCKS];
} sys_sleeplock ;

int check_id_lock(int id_lock) {
  if (id_lock >= MAXSLEEPLOCKS || id_lock < 0) { // invalid id_lock
    return -1;
  }

  if (!sys_sleeplock.sl_active[id_lock]) { // cannot interact with unactive
    return -3;
  }

  return 0;
}

void syssleeplock_init() {
  initlock(&sys_sleeplock.lock, "syssleeplock");

  for (int i = 0; i < MAXSLEEPLOCKS; i++) {
    sys_sleeplock.sl_active[i] = 0;
    initsleeplock(&sys_sleeplock.sleeplocks[i], "available_sleeplock");
  }
}

uint64 syssleeplock(int type_of_request, int id_lock) {
  int catcher = 0;

  if (type_of_request == SYSSLEEPLOCK_INIT) {

    acquire(&sys_sleeplock.lock);

    for (int idfree = 0; idfree < MAXSLEEPLOCKS; idfree++) {

      if (!sys_sleeplock.sl_active[idfree]) {

        sys_sleeplock.sl_active[idfree] = 1;
        release(&sys_sleeplock.lock);

        return idfree;
      }
    }

    release(&sys_sleeplock.lock);

    return -2;
  }

  if (type_of_request == SYSSLEEPLOCK_DELETE) {

    acquire(&sys_sleeplock.lock);

    catcher = check_id_lock(id_lock);

    if (catcher < 0) {
      release(&sys_sleeplock.lock);
      return catcher;
    }

    sys_sleeplock.sl_active[id_lock] = 0;
    release(&sys_sleeplock.lock);

    return 0;
  }

  if (type_of_request == SYSSLEEPLOCK_ACQUIRE) {

    acquire(&sys_sleeplock.lock);

    catcher = check_id_lock(id_lock);

    if (catcher < 0) {
      release(&sys_sleeplock.lock);
      return catcher;
    }

    release(&sys_sleeplock.lock);
    acquiresleep(&sys_sleeplock.sleeplocks[id_lock]);

    return 0;
  }

  if (type_of_request == SYSSLEEPLOCK_RELEASE) {

    acquire(&sys_sleeplock.lock);

    catcher = check_id_lock(id_lock);

    if (catcher < 0) {
      release(&sys_sleeplock.lock);
      return catcher;
    }

    release(&sys_sleeplock.lock);
    releasesleep(&sys_sleeplock.sleeplocks[id_lock]);
    return 0;
  }

  // unknown type_of_request
  return -4;
}