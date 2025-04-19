// Kernel-level Semaphore

#include "ksemaphore.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"
#include "channel.h"
#include "inc/assert.h"
#include "inc/environment_definitions.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/string.h"
#include "inc/types.h"
#include "inc/x86.h"

void init_ksemaphore(struct ksemaphore *ksem, int value, char *name) {
  //[PROJECT'24.MS3]
  // COMMENT THE FOLLOWING LINE BEFORE START CODING
  panic("init_ksemaphore is not implemented yet");
  // Your Code is Here...
}

void wait_ksemaphore(struct ksemaphore *ksem) {
  //[PROJECT'24.MS3]
  // COMMENT THE FOLLOWING LINE BEFORE START CODING
  panic("wait_ksemaphore is not implemented yet");
  // Your Code is Here...
}

void signal_ksemaphore(struct ksemaphore *ksem) {
  //[PROJECT'24.MS3]
  // COMMENT THE FOLLOWING LINE BEFORE START CODING
  panic("signal_ksemaphore is not implemented yet");
  // Your Code is Here...
}
