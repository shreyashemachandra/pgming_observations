/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description         Author
 *  --------  -------   -----------         ------ 
 *  17/11/15    01      Copied as it is     Shreyas
 *                      from linux vpu
 *  24/11/15    02      get_dec_work_addr   Shreyas
 *                      prototype added
 ******************************************************************************/ 

#include<INTEGRITY.h>

#ifndef __VPU__IO__H
#define __VPU__IO__H

/*!
 * @brief  vpu memory description structure
 */
typedef struct vpu_mem_desc {
	int size;		/*!requested memory size */
	unsigned long phy_addr;	/*!physical memory address allocated */
	unsigned long cpu_addr;	/*!cpu addr for system free usage */
	unsigned long virt_uaddr;/* user space address */
} vpu_mem_desc;

typedef struct iram_t {
        unsigned long start;
        unsigned long end;
} iram_t;

#ifndef	true
#define true	1
#endif
#ifndef	false
#define false	0
#endif

typedef void (*vpu_callback) (int status);

int IOSystemShutdown(void);
int IOGetPhyMem(vpu_mem_desc * buff);
int IOFreePhyMem(vpu_mem_desc * buff);
int IOGetVirtMem(vpu_mem_desc * buff);
int IOFreeVirtMem(vpu_mem_desc * buff);
int IOGetVShareMem(int size);
int IOWaitForInt(int timeout_in_ms);
int IOPhyMemCheck(unsigned long phyaddr, const char *name);
int IOGetIramBase(iram_t * iram);
int IOClkGateSet(int on);
int IOGetPhyShareMem(vpu_mem_desc * buff);
int IOSysSWReset(void);
int IOLockDev(int on);

int IOSystemInit(void);

Value VpuWriteReg(Address, Value);
Value VpuReadReg(Address);

void ResetVpu(void);
int isVpuInitialized(void);
RetCode get_dec_work_addr(vpu_mem_desc *);

#endif
