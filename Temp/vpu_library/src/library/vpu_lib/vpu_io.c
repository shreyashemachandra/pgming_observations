/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description         Author
 *  --------  -------   -----------         ------ 
 *  17/11/15    01      getSharedMemory,
 *                      IOSystemInit,
 *                      VpuReadReg,
 *                      VpuWriteReg,        Shreyas
 *                      reg_map
 *                      isVpuinitialized
 *                      functions added
 ******************************************************************************/ 


#include <INTEGRITY.h>
#include <stdlib.h>
#include <stdio.h>
#include <bsp.h>
#include <driver/video/imx6_vpu_iodev.h>
#include <string.h>
#include "vpu_debug.h"
#include "vpu_reg.h"
#include "vpu_io.h"
#include "vpu_lib.h"
#include "vpu_util.h"

/* Global Variables */
static Address vpu_reg_base;
vpu_mem_desc bit_work_addr;
vpu_mem_desc bit_dec_work_addr; // Keeps track of working address for Decoder

/*!
 * Gets the Staring addrss for the Next Requested Memory
 */
RetCode get_dec_work_addr(vpu_mem_desc *mem_desc){
    unsigned long max_phy_addr = bit_work_addr.phy_addr + 0x2000000;
    unsigned long max_virt_addr = bit_work_addr.virt_uaddr + 0x2000000;

    mem_desc->phy_addr = bit_dec_work_addr.phy_addr + bit_dec_work_addr.size;
    bit_dec_work_addr.phy_addr = mem_desc->phy_addr;

    mem_desc->virt_uaddr = bit_dec_work_addr.virt_uaddr + bit_dec_work_addr.size;
    bit_dec_work_addr.virt_uaddr = mem_desc->virt_uaddr;

    if(mem_desc->phy_addr > max_phy_addr ||  mem_desc->virt_uaddr > max_virt_addr){
        mem_desc->phy_addr = 0;
        mem_desc->virt_uaddr = 0;
        printf("Memory reached Max!!\n");
        return RETCODE_FAILURE;
    }
    bit_dec_work_addr.size = mem_desc->size; // Updating Size for the Next Issue
    return RETCODE_SUCCESS;
}

inline Address *reg_map(Address offset){
    return (Address *)(offset + vpu_reg_base);
}

/*!
 * Reads the Register Index passed as an argument
 */
Value VpuReadReg(Address addr){
    Address *reg_addr = reg_map(addr);
    Value ret =  *(volatile Value *) reg_addr;
    printf("ReadReg: %x -> ValueRet: %d\n",addr,ret);
    return ret;
}

/*!
 * Writes data to the Register Index passed as an argument
 */
Value VpuWriteReg(Address addr, Value data){
    Address *reg_addr = reg_map(addr);
    printf("WrittenToReg: %x -> ValueWritten: %d\n",addr,data);
    *(Address *) reg_addr = data;
    return 0;
}

int isVpuInitialized(void){
    int val;
    // Set Clock true
    val = VpuReadReg(BIT_CUR_PC);
    printf("BIT_CUR_PC in isVpuInitialized: %x\n",val);
    // Set Clock False

    return val != 0;
}

/*!
 * Assigns Memory for the region, which is already allocated by the driver
 *
 * @return Success - if it assigns the Memory successfully
 */
static Error getSharedMemory( const char *regionName, Value *Start, Value *Size )
{
    MemoryRegion mr;
    MemoryRegion vmr;
    Error err;
    ExtendedAddress extFirst;
    ExtendedAddress extLast;

    err = RequestResource((Object *)&mr, regionName, "!systempassword");
    if ( err != Success) {
        err_msg( "Resource '%s' not found (%d)\n", regionName, err );
        return err;
    }

    err = GetMemoryRegionExtendedAddresses(mr, &extFirst, &extLast );
    if ( err != Success ) {
        err_msg( "Failed to get '%s' region (%d)\n", regionName, err );
        return err;
    }

    err = MapMemory(__ghs_VirtualMemoryRegionPool, mr, &vmr, Start, Size);
    if ( err != Success ) {
        err_msg( "Failed to map memory (%d)\n", err );
        return err;
    }
    return Success;
}

/*!  
 * @brief IO system initialization.
 *  When user wants to start up the codec system,
 *  this function call is needed, to open the codec device,
 *  map the register into user space,
 *  get the working buffer/code buffer/parameter buffer,
 *  download the firmware, and then set up the interrupt signal path.
 *   
 * @return
 * @li  0             System initialization success.
 * @li -1       System initialization failure.
 */
int IOSystemInit(void){
    Address Start;
    Value Size;
    IODevice vpuDev;
    Value vpu_physical_base;
    //vpu_regs_t *vpuregs; :TODO CHECK

    // :TODO System Rev Info Skipped
    // :TODO Setting Semaphore Memory for API and REG, Initializing Semaphore for 32 instances Skipped

    ETE(RequestResource((Object *)&vpuDev, VPU_IODEV_NAME, "!systempassword"));

    /* Map the 240KB VPU Register area */
    ETE(getSharedMemory( VPU_REGS_MR_NAME, &Start, &Size ));
    // :TODO Check if vpu_regs_t struct is needed
    // vpuregs = (vpu_regs_t *)Start;
    vpu_reg_base = (Address) Start;
    info_msg("************ Register Virtual Memory: ****************\n");
    info_msg("Mappped Virtual Address of Register Area: 0x%08x\n\n",(Value)vpu_reg_base);

    bit_work_addr.size = TEMP_BUF_SIZE + PARA_BUF_SIZE + CODE_BUF_SIZE + PARA_BUF2_SIZE;
    /* Map the VPU DMA memory area */
    ETE(getSharedMemory( VPU_MEMORY_MR_NAME, &Start, &Size ));

    info_msg("Mappped Virtual Address of Memory addr: 0x%08x size: 0x%x \n\n", (Value)Start, Size);
    bit_work_addr.virt_uaddr = Start;

    /* Get the physical address of this allocated Memory block */
    ETE(ReadIODeviceRegister(vpuDev, IODEV_VPU_PHYSICAL_BASE, &vpu_physical_base));
    info_msg( "VPU memory physical base: 0x%08x\n", vpu_physical_base );
    bit_work_addr.phy_addr = (Address) vpu_physical_base;

    return 0;
}

/*!
 * Wait for Interrupt or TimeOut
 *
 * @param -> Time out in Milli Sec
 * @return -> -1 - for Timeout
 * @return -> 0 - for Interrupt
 */
int IOWaitForInt(int timeout_in_ms){
    IODevice vpuDev;
    Error err;
    int ret = 0;

    err = RequestResource((Object *)&vpuDev, VPU_IODEV_NAME, "!systempassword");
    if(err != Success){
        printf("RequestResource Failure!\n");
        ret = -1;
    }

    err = SynchronousReceive((Connection) vpuDev, NULL);
    if(err != Success){
        printf("SynchronousReceive Returned Error\n");
        ret = -1;
    }
    
    return 0;
}
