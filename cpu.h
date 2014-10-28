#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "defs.h"

// spool up the cpu, create ram and load the program
extern bool init_cpu(unsigned len, uint32_t *program);

// runs a single instruction and prints the state of the cpu
extern bool step();

#endif  // CPU_H
