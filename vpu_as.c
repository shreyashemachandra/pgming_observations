#include <INTEGRITY.h>
#include <stdlib.h>
#include <stdio.h>
#include <bsp.h>
#include <driver/video/imx6_vpu_iodev.h>
#include "vpu_lib.h"
#include "vpu_reg.h"
#include "vpu_io.h"

/* Error Throw */
#define ETE( n ) { Error __et_e = ( n ); if ( __et_e != Success ) { printf("%s:%d\n\t\"%s\"\n\tfailed with error (%d)\n", __FILE__, __LINE__, #n, __et_e ); exit(__et_e); }}

/* Extern Variables */
extern Uint16 bitCode[];

/* Global Variables */
static Address vpu_reg_base;


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

void dumpSizeInIntegrity(){
        printf( "\n\nShort size: %d\n",sizeof(short));
	printf( "Int size: %d\n",sizeof(int));
	printf( "long size: %d\n",sizeof(long));
	printf( "Unsigned long size: %d\n",sizeof(unsigned long));
	printf( "Unsigned short size: %d\n\n",sizeof(unsigned short));
}

/*!
 * @brief VPU Initialization
 * This function initializes VPU Hardware and proper data structures/resources.
 * The user must call this function before using any vpu decode operations.
 *
 * @return RETCODE_SUCCESS if success.
 */
RetCode vpu_Init( void )
{
    Address Start;
    Value Size;
    IODevice vpuDev;
    Value vpu_physical_base;
    vpu_regs_t *vpuregs;

    /* :TODO Check where to place these Buffer Starting address */
    Address codeBuffer;
    Address tempBuffer;
    Address paraBuffer;
    Address virt_codeBuf;
    Address *virt_paraBuf;
    Address *virt_paraBuf2;

    // :TODO Remove printf
    printf( "VPU Init Started\n" );
    dumpSizeInIntegrity(); 
    

    // :TODO System Rev Info Skipped
    // :TODO Setting Semaphore Memory for API and REG, Initializing Semaphore for 32 instances Skipped

    ETE(RequestResource((Object *)&vpuDev, VPU_IODEV_NAME, "!systempassword"));

    /* Map the 240KB VPU Register area */
    ETE(getSharedMemory( VPU_REGS_MR_NAME, &Start, &Size ));
    // :TODO Check if vpu_regs_t struct is needed
    vpuregs = (vpu_regs_t *)Start;
    vpu_reg_base = (Address) Start;
    printf("************ Register Virtual Memory: ****************\n");
    printf("Mappped Virtual Address of Register Area: 0x%08x\n\n",(Value)vpu_reg_base);

    /* Map the VPU DMA memory area */
    ETE(getSharedMemory( VPU_MEMORY_MR_NAME, &Start, &Size ));
    virt_codeBuf = Start;
    virt_paraBuf = (Address *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE + PARA_BUF2_SIZE);
    virt_paraBuf2 = (Address *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE);
    
    printf("************ Buffer Virtual Memory: ****************\n");
    printf("Code Buffer Virtual Start Address: 0x%08x\n",(Value)virt_codeBuf);
    printf("Para Buffer Virtual Start Address: 0x%08x\n",(Value) (* virt_paraBuf));
    printf("Para Buffer2 Virtual Start Address: 0x%08x\n\n",(Value)(* virt_paraBuf2));
        
    /* Get the physical address of this allocated Memory block */
    ETE(ReadIODeviceRegister(vpuDev, IODEV_VPU_PHYSICAL_BASE, &vpu_physical_base));
    printf( "VPU memory physical base: 0x%08x\n", vpu_physical_base );

    codeBuffer = (Address) vpu_physical_base;
    tempBuffer = codeBuffer + CODE_BUF_SIZE;
    paraBuffer = tempBuffer + TEMP_BUF_SIZE + PARA_BUF2_SIZE;
    
    printf("************ Buffer Physical Memory: ****************\n");
    printf("Code Buffer Physical Start Address: 0x%08x\n",(Value)codeBuffer);
    printf("Code Buffer Physical Start Address: 0x%08x\n",(Value)tempBuffer);
    printf("Code Buffer Physical Start Address: 0x%08x\n\n",(Value)paraBuffer);

    printf("BitCode %x \n",bitCode[0]);
    // :TODO Enable Clock
    // :TODO Download Bit Code

    printf( "VPU example end\n" );
    return RETCODE_SUCCESS;
}

int main( void ){
    RetCode status;
    status = vpu_Init();
    if(status == RETCODE_SUCCESS){
        printf("Vpu_Init Returns Success\n");
	VpuWriteReg(BIT_INT_REQ,0x1);
	printf("BIT Current PC: 0x%x \n", VpuReadReg(BIT_CUR_PC));
	VpuWriteReg(BIT_CODE_RUN,0x1);
	printf("BIT Current PC: 0x%x \n", VpuReadReg(BIT_CUR_PC));
    }else{
	printf("Something Went! Error Code: %d \n",status);
    }
    
    printf("Main Exitting!\n");
    return 0;
}
