#include <INTEGRITY.h>
#include <stdlib.h>
#include <stdio.h>
#include <bsp.h>
#include <driver/video/imx6_vpu_iodev.h>
#include <string.h>
#include "vpu_lib.h"
#include "vpu_reg.h"
#include "vpu_io.h"
#include "vpu_debug.h"
#include "vpu_gdi.h"
#include "vpu_util.h"

#define IMAGE_ENDIAN            0
#define STREAM_ENDIAN           0

/* Extern Variables */
extern vpu_mem_desc bit_work_addr;

/* Global Variables */
Address virt_codeBuf;
Address *virt_paraBuf;
Address *virt_paraBuf2;

void dumpSizeInIntegrity(){
        printf( "\n\nShort size: %d\n",sizeof(short));
	printf( "Int size: %d\n",sizeof(int));
	printf( "long size: %d\n",sizeof(long));
	printf( "Unsigned long size: %d\n",sizeof(unsigned long));
	printf( "Unsigned short size: %d\n\n",sizeof(unsigned short));
	printf( "Char size: %d\n\n",sizeof(char));
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
    int err;
    Uint16 *bit_code = NULL;

    /* :TODO Check where to place these Buffer Starting address */
    Address codeBuffer;
    Address tempBuffer;
    Address paraBuffer;

    // :TODO Remove printf
    printf( "VPU Init Started\n" );
    dumpSizeInIntegrity();     

    // :TODO Wait for bInit to be 1
    err = IOSystemInit();
   // :TODO set bInit to 0
    if(err){
	printf("Error in IOSystem Init\n");
	return RETCODE_FAILURE;
    }

    codeBuffer = bit_work_addr.phy_addr;
    tempBuffer = codeBuffer + CODE_BUF_SIZE;
    paraBuffer = tempBuffer + TEMP_BUF_SIZE + PARA_BUF2_SIZE;
    
    printf("************ Buffer Physical Memory: ****************\n");
    printf("Code Buffer Physical Start Address: 0x%08x\n",(Value)codeBuffer);
    printf("Code Buffer Physical Start Address: 0x%08x\n",(Value)tempBuffer);
    printf("Code Buffer Physical Start Address: 0x%08x\n\n",(Value)paraBuffer);

    virt_codeBuf = bit_work_addr.virt_uaddr;
    virt_paraBuf = (Address *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE + PARA_BUF2_SIZE);
    virt_paraBuf2 = (Address *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE);
    
    printf("************ Buffer Virtual Memory: ****************\n");
    printf("Code Buffer Virtual Start Address: 0x%08x\n",(Value)virt_codeBuf);
    printf("Para Buffer Virtual Start Address: 0x%08x\n",(Value) (* virt_paraBuf));
    printf("Para Buffer2 Virtual Start Address: 0x%08x\n\n",(Value)(* virt_paraBuf2));
    
    // :TODO Check CodecInst Struct ppendingInst - is needed or not
    if (!isVpuInitialized()) {
        int size,i;
	volatile Uint32 data;
	
	printf("VPU Not Initialized, Loading BitCode\n");
        // :TODO Download Bit Code from the file system
	printf("MAX_FW_BINARY_LEN = %d \n",MAX_FW_BINARY_LEN);

	/* bit_code = malloc(MAX_FW_BINARY_LEN * sizeof(Uint16));
	if(bit_code == NULL){
	    printf("failed to Allocate Memory for Bit Code\n");
	    return RETCODE_FAILURE;
	}
        memset(bit_code,0,MAX_FW_BINARY_LEN * sizeof(Uint16));*/
	
        LoadBitCodeTable(&bit_code,&size);
	printf("Bit Code Length: %d\n",size);
	if(bit_code == NULL){
	    printf("Bit Code Not Allocated!\n");
	    return RETCODE_FAILURE;
	}
	for (i = 0; i < size; i += 4) {
              data = (bit_code[i + 0] << 16) | bit_code[i + 1];
              ((Address *)virt_codeBuf)[i / 2 + 1] =  data;     
              data = (bit_code[i + 2] << 16) | bit_code[i + 3];
              ((Address *)virt_codeBuf)[i / 2] = data;
         }

	for (i = 0; i < 64; i++)
              VpuWriteReg(BIT_CODE_BUF_ADDR + (i * 4), 0);
        VpuWriteReg(BIT_PARA_BUF_ADDR, paraBuffer);
        VpuWriteReg(BIT_CODE_BUF_ADDR, codeBuffer);
        VpuWriteReg(BIT_TEMP_BUF_ADDR, tempBuffer);

	VpuWriteReg(BIT_BIT_STREAM_PARAM, 0);

	if (VpuReadReg(BIT_CUR_PC) != 0) {
                /* IRQ is disabled during shutdown */
                VpuWriteReg(BIT_INT_ENABLE, 1 << INT_BIT_PIC_RUN);
                return RETCODE_SUCCESS;
        }

	VpuWriteReg(BIT_CODE_RUN, 0);
              
        /* Download BIT Microcode to Program Memory */
        for (i = 0; i < 2048; ++i) {
            data = bit_code[i];
            VpuWriteReg(BIT_CODE_DOWN, (i << 16) | data);
        }
          
        data = STREAM_ENDIAN | STREAM_FULL_EMPTY_CHECK_DISABLE << BIT_BUF_CHECK_DIS;
        data |= BUF_PIC_FLUSH << BIT_BUF_PIC_FLUSH | BUF_PIC_RESET << BIT_BUF_PIC_RESET;
        VpuWriteReg(BIT_BIT_STREAM_CTRL, data);

	VpuWriteReg(BIT_FRAME_MEM_CTRL, IMAGE_ENDIAN | 1 << 12);
	VpuWriteReg(BIT_INT_ENABLE, 1 << INT_BIT_PIC_RUN);
        VpuWriteReg(BIT_AXI_SRAM_USE, 0);
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	dump_regs(0, 128);
        VpuWriteReg(BIT_CODE_RUN, 1);
        while (VpuReadReg(BIT_BUSY_FLAG));

	// :TODO Free Bit Code Area
    }

    printf( "VPU example end\n" );
    return RETCODE_SUCCESS;
}

int main( void ){
    RetCode status;
    status = vpu_Init();
    if(status == RETCODE_SUCCESS){
        printf("Vpu_Init Returns Success\n");
	/*VpuWriteReg(BIT_INT_REQ,0x1);
	printf("BIT Current PC: 0x%x \n", VpuReadReg(BIT_CUR_PC));
	VpuWriteReg(BIT_CODE_RUN,0x1);
	printf("BIT Current PC: 0x%x \n", VpuReadReg(BIT_CUR_PC));*/
    }else{
	printf("Something Went Wrong! Error Code: %d \n",status);
    }
    
    printf("Main Exitting!\n");
    return 0;
}
