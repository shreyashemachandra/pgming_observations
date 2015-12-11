/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description         Author
 *  --------  -------   -----------         ------ 
 *  17/11/15    01      SwapArray,
 *                      LoadBitCodeTable    Shreyas
 *                      Added for
 *                      vpu_Init
 *  24/11/15    02      GetCodecInstance,
 *                      BitIssueCommand,    Shreyas
 *                      CheckDecOpenParam
 *                      Added for vpu_DecOpen
 ******************************************************************************/ 

#include <INTEGRITY.h>
#include <string.h>
#include "vpu_util.h"
#include "vpu_io.h"
#include "vpu_debug.h"
#include "bitCode.h"

#define BIT_CODE_FILE_PATH  "/sda/firmware"

/* Extern Variables */
extern Uint16 bitCode[];
extern Uint16 bitCodeInfo[];

extern Address *virt_paraBuf;

typedef struct {
    Uint8 platform[12];
    Uint32 size;
} headerInfo;

/*!
 * Changing the Endianness of the input
 */
void SwapArray(Uint16 *input, Uint32 size)
{
    int i;

    //    ENTER_FUNC("SwapArray");
    for (i = 0; i < size; i++)
    {
        input[i] = ((((Uint16)input[i] & 0x00ff)  << 8) | (((Uint16)input[i] & 0xff00) >> 8));
    }
    //    EXIT_FUNC("SwapArray");
}

/*
 * GetCodecInstance() obtains an instance.
 * It stores a pointer to the allocated instance in *ppInst
 * and returns RETCODE_SUCCESS on success.
 * Failure results in 0(null pointer) in *ppInst and RETCODE_FAILURE.
 */
RetCode GetCodecInstance(CodecInst ** ppInst)
{
    CodecInst *pCodecInst;
#ifdef LINUX
    /*int i=0;
    // Getting Codec Instance in Linux
    for (i = 0; i < MAX_NUM_INSTANCE; ++i) {
    pCodecInst = (CodecInst *) (&vpu_shared_mem->codecInstPool[i]);
    if (!pCodecInst->inUse)
    break;
    }*/
#else
    // :TODO Remove this once we figure out using vpu_shared_mem
    pCodecInst = (CodecInst *) calloc(1,sizeof(CodecInst));
#endif
    if(pCodecInst == NULL){
        printf("Memory for pCodedIsnt not Alloted, calloc failed\n");
        return RETCODE_FAILURE;
    }
#ifdef LINUX
    if (i == MAX_NUM_INSTANCE) {
        *ppInst = 0;
        return RETCODE_FAILURE;
    }
#endif
    memset(pCodecInst, 0, sizeof(CodecInst));
    pCodecInst->instIndex = 0;
    pCodecInst->inUse = 1;
    *ppInst = pCodecInst;
    return RETCODE_SUCCESS;
}

void SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para)
{      
    CodecInst *pCodecInst;
    int i;
    Uint32 *src;
    int byteSize;

    pCodecInst = handle;

    src = para->paraSet;
    byteSize = para->size / 4;

    for (i = 0; i < byteSize; i += 1) {
        virt_paraBuf[i] = *src++;
    }      

    VpuWriteReg(CMD_DEC_PARA_SET_TYPE, paraSetType);
    VpuWriteReg(CMD_DEC_PARA_SET_SIZE, para->size);

    BitIssueCommand(pCodecInst, DEC_PARA_SET);
    while (VpuReadReg(BIT_BUSY_FLAG)) ;
}

int DecBitstreamBufEmpty(DecHandle handle)
{
    CodecInst *pCodecInst;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    int instIndex;

    ENTER_FUNC("DecBitstreamBufEmpty");

    pCodecInst = handle;

    instIndex = VpuReadReg(BIT_RUN_INDEX);

    rdPtr = (pCodecInst->instIndex == instIndex) ? VpuReadReg(BIT_RD_PTR) : pCodecInst->ctxRegs[CTX_BIT_RD_PTR]; 
    wrPtr = (pCodecInst->instIndex == instIndex) ? VpuReadReg(BIT_WR_PTR) : pCodecInst->ctxRegs[CTX_BIT_WR_PTR];

    EXIT_FUNC("DecBitstreamBufEmpty");
    return rdPtr == wrPtr;
}

void SetMaverickCache(MaverickCacheConfig *pCacheConf, int mapType, int chromInterleave)
{
    if (mapType == LINEAR_FRAME_MAP) {
        /* Set luma */
        pCacheConf->luma.cfg.PageSizeX = 2;
        pCacheConf->luma.cfg.PageSizeY = 0;
        pCacheConf->luma.cfg.CacheSizeX = 2;
        pCacheConf->luma.cfg.CacheSizeY = 6;
        /* Set chroma */
        pCacheConf->chroma.cfg.PageSizeX = 2;
        pCacheConf->chroma.cfg.PageSizeY = 0;
        pCacheConf->chroma.cfg.CacheSizeX = 2;
        pCacheConf->chroma.cfg.CacheSizeY = 4;
        pCacheConf->PageMerge = 2;
    } else {
        /* Set luma */
        pCacheConf->luma.cfg.PageSizeX = 0;
        pCacheConf->luma.cfg.PageSizeY = 2;
        pCacheConf->luma.cfg.CacheSizeX = 4;
        pCacheConf->luma.cfg.CacheSizeY = 4;
        /* Set chroma */
        pCacheConf->chroma.cfg.PageSizeX = 0;
        pCacheConf->chroma.cfg.PageSizeY = 2;
        pCacheConf->chroma.cfg.CacheSizeX = 4;
        pCacheConf->chroma.cfg.CacheSizeY = 3;
        pCacheConf->PageMerge = 1;
    }

    pCacheConf->Bypass = 0; /* cache enable */
    pCacheConf->DualConf = 0;
    pCacheConf->LumaBufferSize = 32;
    if (chromInterleave) {
        pCacheConf->CbBufferSize = 0;
        pCacheConf->CrBufferSize = 0x10;
    } else {
        pCacheConf->CbBufferSize = 8;
        pCacheConf->CrBufferSize = 8;
    }
}

RetCode CheckDecInstanceValidity(DecHandle handle)
{   
    CodecInst *pCodecInst;

    pCodecInst = handle;

    if (!pCodecInst->inUse) {
        printf("Instance->inUse=0 error !\n");
        return RETCODE_INVALID_HANDLE;
    }

    if(pCodecInst->codecMode != AVC_DEC){
        printf("CodecMode is not AVC_DEC - Error\n");
        return RETCODE_INVALID_PARAM;
    }
    return RETCODE_SUCCESS;
}


RetCode CheckDecOpenParam(DecOpenParam * pop)
{
    if (pop == 0) {
        printf("pop Null\n");
        return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamBuffer % 512) { /* not 512-byte aligned */
        printf("Not 512-Byte Aligned\n");
        return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamBufferSize % 1024 || pop->bitstreamBufferSize < 1024 || pop->bitstreamBufferSize > 16383 * 1024) {
        printf("Check the Condition\n");// TODO check whats it doing
        return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamFormat != STD_AVC) {
        err_msg("Invalid Param: Only AVC decoding is supported.\n");
        return RETCODE_INVALID_PARAM;
    }
   
    return RETCODE_SUCCESS;
}


void BitIssueCommand(CodecInst *pCodecInst, int cmd)
{       
    int instIdx = MAX_NUM_INSTANCE, cdcMode = 0, auxMode = 0;

    printf("BitIssueCommand: %d\n", cmd);

    if (pCodecInst != NULL) {
        /* Save context related registers to vpu */
        VpuWriteReg(BIT_BIT_STREAM_PARAM,pCodecInst->ctxRegs[CTX_BIT_STREAM_PARAM]);
        VpuWriteReg(BIT_FRM_DIS_FLG,pCodecInst->ctxRegs[CTX_BIT_FRM_DIS_FLG]);
        VpuWriteReg(BIT_WR_PTR,pCodecInst->ctxRegs[CTX_BIT_WR_PTR]);
        VpuWriteReg(BIT_RD_PTR,pCodecInst->ctxRegs[CTX_BIT_RD_PTR]);
        VpuWriteReg(BIT_FRAME_MEM_CTRL,pCodecInst->ctxRegs[CTX_BIT_FRAME_MEM_CTRL]);
        VpuWriteReg(BIT_WORK_BUF_ADDR, pCodecInst->contextBufMem.phy_addr);
        instIdx = pCodecInst->instIndex;
        cdcMode = pCodecInst->codecMode;
        auxMode = pCodecInst->codecModeAux;
        VpuWriteReg(GDI_WPROT_ERR_CLR, 1);
        VpuWriteReg(GDI_WPROT_RGN_EN, 0);
    }
    VpuWriteReg(BIT_BUSY_FLAG, 0x1);
    VpuWriteReg(BIT_RUN_INDEX, instIdx);
    VpuWriteReg(BIT_RUN_COD_STD, cdcMode);
    VpuWriteReg(BIT_RUN_AUX_STD, auxMode);
    VpuWriteReg(BIT_RUN_COMMAND, cmd);
}


RetCode LoadBitCodeTable(Uint16 **pBitCode, int *size){

    FILE *fp;
    headerInfo info;
    int ret;

    ENTER_FUNC("LoadBitCodeTable");
    /*
       SwapArray(&bitCodeInfo[0], 8);
       memcpy(&info,bitCodeInfo,sizeof(headerInfo));

       SwapArray(&bitCode[0], sizeOfBitCode());
       printf("Platform Name: %s \n",info.platform);
       printf("Bite_Code Size : 0x%x | %d | MAX BitCode Len: %d \n",info.size, info.size, MAX_FW_BINARY_LEN);

       if(info.size > MAX_FW_BINARY_LEN){
       printf("Size in VPU header is too large.Size: %d\n", info.size);
       return RETCODE_FAILURE;
       }

    // :TODO Delete this after attaching the actual Bitcode
     *pBitCode = bitCode;

     *size = (int) info.size;*/

    fp = fopen(BIT_CODE_FILE_PATH, "rb");
    if(fp == NULL){
        err_msg("Error on opening Firmware file");
        return RETCODE_FAILURE;
    }

    ret = fread(&info, sizeof(headerInfo),1,fp);
    if(ret == 0){
        err_msg("Error on reading HeaderInfo of Firmware\n");
        return RETCODE_FAILURE;
    }

    if(info.size > MAX_FW_BINARY_LEN){
        err_msg("Size in VPU header is too large.Size: %d\n",(Uint16) info.size);
        fclose(fp);
        return RETCODE_FAILURE;
    }

    ret = fread(pBitCode, sizeof(Uint16), info.size, fp); 
    if (ret < (int)info.size) { 
        err_msg("VPU firmware binary file is wrong or corrupted.\n");
        fclose(fp);
        return RETCODE_FAILURE;
    }
    fclose(fp);

    *size = (int)info.size;

    EXIT_FUNC("LoadBitCodeTable");

    return RETCODE_SUCCESS;
}
