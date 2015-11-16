#include <INTEGRITY.h>
#include <stdio.h>
 #include "vpu_lib.h"
 #include "vpu_io.h"
 #include "vpu_debug.h"
  
 void dump_regs(Uint32 base, int cnt)
 {        
    int i;
  
//     if (vpu_lib_dbg_level >= 6) {
         for (i=0; i<cnt; i++) {
             if ((i%8)==0)
                 printf("\n 0x%08lx:   ", base+i*4);
             printf("0x%lx, ", VpuReadReg(base+i*4));
         }
         printf("\n");
  //   }
 } 
