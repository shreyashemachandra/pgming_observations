/*******************************************************************************	
 *	
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.	
 *	
 *  REVISION HISTORY	
 *	
 *  dd/mm/yy  Version   Description         Author	
 *  --------  -------   -----------         ------ 	
 *  17/11/15    01      copied vpu_debug.h  Shreyas	
 *                      from linux	
 *  24/11/15    02      modified ENTER_FUNC	
 *                      EXIT_FUNC           Shreyas	
 ******************************************************************************/
#ifndef __VPU_DEBUG_H
#define __VPU_DEBUG_H

#include "vpu_lib.h"

extern int vpu_lib_dbg_level;


#define err_msg(fmt, arg...) do { if (vpu_lib_dbg_level >= 1)		\
    printf("[ERR]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); else \
    printf("[ERR]\t" fmt, ## arg);	\
} while (0)
#define info_msg(fmt, arg...) do { if (vpu_lib_dbg_level >= 1)		\
    printf("[INFO]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg);  else \
    printf("[INFO]\t" fmt, ## arg); \
} while (0)
#define warn_msg(fmt, arg...) do { if (vpu_lib_dbg_level >= 1)		\
    printf("[WARN]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); else \
    printf("[WARN]\t" fmt, ## arg);	\
} while (0)


#define dprintf(level, fmt, arg...)     if (vpu_lib_dbg_level >= level) \
                                                                 printf("[DEBUG]\t%s:%d " fmt, __FILE__, __LINE__, ## arg)

#define ENTER_FUNC(func) dprintf(4, "enter %s()\n", func)
#define EXIT_FUNC(func) dprintf(4, "exit %s()\n", func)

void dump_regs(Uint32 base, int cnt);
void dump_reg(Uint32 base, char *regname);
#endif

/* Error Throw */
#define ETE( n ) { Error __et_e = ( n ); if ( __et_e != Success ) { printf("%s:%d\n\t\"%s\"\n\tfailed with error (%d)\n", __FILE__, __LINE__, #n, __et_e ); exit(__et_e); }}
