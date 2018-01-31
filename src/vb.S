#include <rlx/rlx_cpu_regs.h>

.global bss_call
.ent    bss_call
bss_call:
move    t1, ra
la      sp, __bss_start 
addiu   sp, sp, -32
la      t0, c_init
jalr    t0
nop
move    ra, t1
jr      ra
.space  20 
.end    bss_call
