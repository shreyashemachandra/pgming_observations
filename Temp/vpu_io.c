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
// vpu_mem_desc bit_work_addr; // :TODO Check if it is needed
static Address vpu_reg_base;

vpu_mem_desc bit_work_addr;

inline Address *reg_map(Address offset){
    return (Address *)(offset + vpu_reg_base);
}

/*!
 * Reads the Register Index passed as an argument
 */
Value VpuReadReg(Address addr){
    Address *reg_addr = reg_map(addr);
    return *(volatile Value *) reg_addr;
}

/*!
 * Writes data to the Register Index passed as an argument
 */
Value VpuWriteReg(Address addr, Value data){
    Address *reg_addr = reg_map(addr);
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
	printf( "Resource '%s' not found (%d)\n", regionName, err );
	return err;
    }
    err = GetMemoryRegionExtendedAddresses(mr, &extFirst, &extLast );
    if ( err != Success ) {
	printf( "Failed to get '%s' region (%d)\n", regionName, err );
	return err;
    }
    /*printf( "Found MemoryRegion '%s':\n", regionName );
      printf( "  Logical area: 0x%08llx-0x%08llx\n", extFirst, extLast);*/
    err = MapMemory(__ghs_VirtualMemoryRegionPool, mr, &vmr, Start, Size);
    if ( err != Success ) {
	printf( "Failed to map memory (%d)\n", err );
	return err;
    }
    //printf( "  Virtual area:  0x%08x-0x%08x\n", *Start, *Start+*Size-1);
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
    printf("************ Register Virtual Memory: ****************\n");
    printf("Mappped Virtual Address of Register Area: 0x%08x\n\n",(Value)vpu_reg_base);

    bit_work_addr.size = TEMP_BUF_SIZE + PARA_BUF_SIZE + CODE_BUF_SIZE + PARA_BUF2_SIZE;
    /* Map the VPU DMA memory area */
    ETE(getSharedMemory( VPU_MEMORY_MR_NAME, &Start, &Size ));
    bit_work_addr.virt_uaddr = Start;
        
    /* Get the physical address of this allocated Memory block */
    ETE(ReadIODeviceRegister(vpuDev, IODEV_VPU_PHYSICAL_BASE, &vpu_physical_base));
    printf( "VPU memory physical base: 0x%08x\n", vpu_physical_base );
    bit_work_addr.phy_addr = (Address) vpu_physical_base;
    
    return 0;
}
