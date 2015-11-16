#include <INTEGRITY.h>
#include <string.h>
#include "vpu_util.h"
#include "vpu_io.h"
#include "vpu_debug.h"

/* Extern Variables */
extern Uint16 bitCode[];
extern Uint16 bitCodeInfo[];

typedef struct {
    Uint8 platform[12];
    Uint32 size;
} headerInfo;

/*!
 * Changing Endianness
 */
void SwapArray(Uint16 *input, Uint32 size)
{
    int i;
    
    for (i = 0; i < size; i++)
    {
	input[i] = ((((Uint16)input[i] & 0x00ff)  << 8) | (((Uint16)input[i] & 0xff00) >> 8));
	printf("0x%x\n",input[i]);
    }
}

RetCode LoadBitCodeTable(Uint16 **pBitCode, int *size){
    headerInfo info;
    
    SwapArray(&bitCodeInfo[0], 8);
    memcpy(&info,bitCodeInfo,sizeof(headerInfo));
    
    printf("Platform Name: %s \n",info.platform);
    printf("Bite_Code Size : 0x%x | %d | MAX BitCode Len: %d \n",info.size, info.size, MAX_FW_BINARY_LEN);
    
    if(info.size > MAX_FW_BINARY_LEN){
	printf("Size in VPU header is too large.Size: %d\n", info.size);
	return RETCODE_FAILURE;
    }

    // :TODO Delete this after attaching the actual Bitcode
    *pBitCode = bitCode;
    
    *size = (int) info.size;

    return RETCODE_SUCCESS;
}
