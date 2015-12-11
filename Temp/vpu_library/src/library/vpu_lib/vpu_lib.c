/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description                 Author
 *  --------  -------   -----------                 ------ 
 *  17/11/15    01      vpu_Init, main
 *                      Added. Tested 
 *                      with GetVersion             Shreyas
 *                      function
 *  24/11/15    02      vpu_DecOpen added           Shreyas
 *  27/11/15    03      decode_parse,
 *                      dec_fill_buffer,
 *                      ReadFileWithTimings,
 *                      vpu_DecSetEscSeqInit,       Shreyas
 *                      vpu_DecGetInitialInfo,
 *                      vpu_DecGiveCommand,
 *                      vpu_DecUpdateBitstreamBuffer,
 *                      vpu_DecGetBitstreamBuffer 
 *                      added.
 *  03/12/15    04      vpu_DecStartOneFrame,
 *                      vpu_IsBusy function added   Shreyas
 *  08/12/15    05      vpu_DecGetOutputInfo
 ******************************************************************************/ 

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

#define MAX_PIC_WIDTH           1920
#define MAX_PIC_HEIGHT          1088

/* Extern Variables */
extern vpu_mem_desc bit_work_addr;
extern vpu_mem_desc bit_dec_work_addr;

/* Global Variables */
int vpu_lib_dbg_level;
/* :TODO Check where to place these Buffer Starting address */
Address virt_codeBuf;
Address *virt_paraBuf;
Address *virt_paraBuf2;
Address codeBuffer;
Address tempBuffer;
Address paraBuffer;

/* If a frame is started, pendingInst is set to the proper instance. */
static CodecInst *ppendingInst;
/* The single instance for Decoding */
static CodecInst *pCodecInst;

/*! :TODO Remove It
 * Prints all the Datatype size in Integrity
 */
void dumpSizeInIntegrity(){
    ENTER_FUNC("dumpSizeInIntegrity");
    printf( "\n\nShort size: %d\n",sizeof(short));
    printf( "Int size: %d\n",sizeof(int));
    printf( "long size: %d\n",sizeof(long));
    printf( "Unsigned long size: %d\n",sizeof(unsigned long));
    printf( "Unsigned short size: %d\n\n",sizeof(unsigned short));
    printf( "Char size: %d\n\n",sizeof(char));
    EXIT_FUNC("dumpSizeInIntegrity");
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

    vpu_lib_dbg_level = 9; // :TODO Remove the Hardcoded value, once you figure out how to read env variables

    ENTER_FUNC("vpu_Init");
    dumpSizeInIntegrity();

    // :TODO Check if "lock" has to be incorporated
    err = IOSystemInit();
    // :TODO Unlock "lock"

    if(err){
        err_msg("Error in IOSystem Init\n");
        return RETCODE_FAILURE;
    }

    codeBuffer = bit_work_addr.phy_addr;
    tempBuffer = codeBuffer + CODE_BUF_SIZE;
    paraBuffer = tempBuffer + TEMP_BUF_SIZE + PARA_BUF2_SIZE;

    printf("************ Buffer Physical Memory: ****************\n");
    printf("Code Buffer Physical Start Address: 0x%08x\n",(Value)codeBuffer);
    printf("Temp Buffer Physical Start Address: 0x%08x\n",(Value)tempBuffer);
    printf("Para Buffer Physical Start Address: 0x%08x\n\n",(Value)paraBuffer);

    virt_codeBuf = bit_work_addr.virt_uaddr;
    virt_paraBuf2 = (unsigned long *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE);
    virt_paraBuf = (unsigned long *)(virt_codeBuf + CODE_BUF_SIZE + TEMP_BUF_SIZE + PARA_BUF2_SIZE);

    printf("************ Buffer Virtual Memory: ****************\n");
    printf("Code Buffer Virtual Start Address: 0x%08x\n",(Value)virt_codeBuf);
    printf("Para Buffer Virtual Start Address: 0x%08x\n",(Value) (virt_paraBuf));
    printf("Para Buffer2 Virtual Start Address: 0x%08x\n\n",(Value)(virt_paraBuf2));

    // Updating the Decode work Address
    bit_dec_work_addr.phy_addr = bit_work_addr.phy_addr + 0x1000000; // Skiping 16MB
    bit_dec_work_addr.virt_uaddr = bit_work_addr.virt_uaddr + 0x1000000;
    bit_dec_work_addr.size = 0;

    // :TODO Check CodecInst Struct ppendingInst - is needed or not
    // :TODO Check (CodecInst **) (&vpu_semap->pendingInst)
    ppendingInst = NULL;

    if (!isVpuInitialized()) {
        int size,i;
        volatile Uint32 data;

        printf("VPU Not Initialized, Loading BitCode\n");
        printf("MAX_FW_BINARY_LEN = %d \n",MAX_FW_BINARY_LEN);


        bit_code = calloc(1,MAX_FW_BINARY_LEN * sizeof(Uint16));
        if(bit_code == NULL){
            printf("failed to Allocate Memory for Bit Code\n");
            return RETCODE_FAILURE;
        }
        //        memset(bit_code,0,MAX_FW_BINARY_LEN * sizeof(Uint16));

        LoadBitCodeTable(bit_code,&size);
        printf("Bit Code Length: %d\n",size);
        if(bit_code == NULL){
            printf("Bit Code Not Allocated!\n");
            return RETCODE_FAILURE;
        }
        for (i = 0; i < size; i += 4) {
            data = (bit_code[i + 0] << 16) | bit_code[i + 1];
            ((unsigned int *)virt_codeBuf)[i / 2 + 1] =  data;
            data = (bit_code[i + 2] << 16) | bit_code[i + 3];
            ((unsigned int *)virt_codeBuf)[i / 2] = data;
        }

        for (i = 0; i < 64; i++)
        {
            VpuWriteReg(BIT_CODE_BUF_ADDR + (i * 4), 0);
        }

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
        while (VpuReadReg(BIT_BUSY_FLAG))
        {
            dump_reg(BIT_BUSY_FLAG,"BIT_BUSY_FLAG");
        }
        EXIT_FUNC("vpu_Init");
        // :TODO Free Bit Code Area
    }
    return RETCODE_SUCCESS;
}


/*!
 * @brief Get VPU Firmware Version.
 */
RetCode vpu_GetVersionInfo(vpu_versioninfo * verinfo)
{
    Uint32 ver, fw_code = 0;
    Uint16 pn, version;
    RetCode ret = RETCODE_SUCCESS;
    char productstr[18] = { 0 };

    ENTER_FUNC("vpu_GetVersionInfo");

    if (!isVpuInitialized()) {
        return RETCODE_NOT_INITIALIZED;
    }

    VpuWriteReg(RET_VER_NUM, 0);

    BitIssueCommand(NULL, FIRMWARE_GET);

    while (VpuReadReg(BIT_BUSY_FLAG)) ;

    ver = VpuReadReg(RET_VER_NUM);
    fw_code = VpuReadReg(RET_FW_CODE_REV);

    if (ver == 0)
        return RETCODE_FAILURE;

    pn = (Uint16) (ver >> 16);
    version = (Uint16) ver;

    switch (pn) {
        case PRJ_TRISTAN:
        case PRJ_TRISTAN_REV:
            strcpy(productstr, "i.MX27");
            break;
        case PRJ_CODAHX_14:
            strcpy(productstr, "i.MX51");
            break;
        case PRJ_CODA7541:
            strcpy(productstr, "i.MX53");
            break;
        case PRJ_CODA_960:
            strcpy(productstr, "i.MX6Q/D/S");
            break;
        default:
            printf("Unknown VPU\n");
            ret = RETCODE_FAILURE;
            break;
    }

    if(verinfo != NULL) {
        verinfo->fw_major = (version >> 12) & 0x0f;
        verinfo->fw_minor = (version >> 8) & 0x0f;
        verinfo->fw_release = version & 0xff;
        verinfo->fw_code = fw_code;
        verinfo->lib_major = (VPU_LIB_VERSION_CODE >> (12)) & 0x0f;
        verinfo->lib_minor = (VPU_LIB_VERSION_CODE >> (8)) & 0x0f;
        verinfo->lib_release = (VPU_LIB_VERSION_CODE) & 0xff;
        printf("Product Info: %s\n", productstr);
    } 
    return ret;
}

RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape)
{
    CodecInst *inst;
    RetCode ret;

    ENTER_FUNC("vpu_DecSetEscSeqInit");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    inst = handle;

    if (escape == 0)
        inst->ctxRegs[CTX_BIT_STREAM_PARAM] &= ~0x01;
    else
        inst->ctxRegs[CTX_BIT_STREAM_PARAM] |= 0x01;

    EXIT_FUNC("vpu_DecSetEscSeqInit");

    return RETCODE_SUCCESS;
}

/*!
 * @brief Get header information of bitstream.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param info [Output] Pointer to DecInitialInfo data structure.
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_FAILURE There was an error in getting initial information.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_INVALID_PARAM info is an invalid pointer.
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 */
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    Uint32 val, val2;
    RetCode ret;

    ENTER_FUNC("vpu_DecGetInitialInfo");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;

    if (pDecInfo->initialInfoObtained) {
        printf("initialInfoObtained - %d returned\n",pDecInfo->initialInfoObtained);
        return RETCODE_CALLED_BEFORE;
    }

    if (DecBitstreamBufEmpty(handle)) {
        err_msg("rd 0x%lx, rd reg 0x%lx, wr 0x%lx, wr reg 0x%lx, idx %d, idx reg %ld\n",
                inst->ctxRegs[CTX_BIT_RD_PTR], VpuReadReg(BIT_RD_PTR),
                inst->ctxRegs[CTX_BIT_WR_PTR], VpuReadReg(BIT_WR_PTR),
                inst->instIndex, VpuReadReg(BIT_RUN_INDEX));

        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    VpuWriteReg(CMD_DEC_SEQ_BB_START, pDecInfo->streamBufStartAddr);
    VpuWriteReg(CMD_DEC_SEQ_BB_SIZE, pDecInfo->streamBufSize / 1024);

    val = 0;

    val |= ((pDecInfo->openParam.reorderEnable << 1) & 0x2);


    val |= (pDecInfo->openParam.mp4DeblkEnable & 0x1);

    VpuWriteReg(CMD_DEC_SEQ_OPTION, val);

    VpuWriteReg(CMD_DEC_SEQ_X264_MV_EN, 0);

    VpuWriteReg(CMD_DEC_SEQ_SPP_CHUNK_SIZE, 512);

    BitIssueCommand(inst, SEQ_INIT);

    while (VpuReadReg(BIT_BUSY_FLAG)) ;
    if (pDecInfo->openParam.bitstreamMode) {
        /* check once more in roll back mode, in case
         * BIT_BUSY_FLAG=0 is caused by reset */
        while (VpuReadReg(BIT_BUSY_FLAG)) ;
    }

    /* Backup rd pointer to ctx */
    inst->ctxRegs[CTX_BIT_RD_PTR] = VpuReadReg(BIT_RD_PTR);
    inst->ctxRegs[CTX_BIT_STREAM_PARAM] = VpuReadReg(BIT_BIT_STREAM_PARAM);

    val = VpuReadReg(RET_DEC_SEQ_SUCCESS);

    if (pDecInfo->openParam.bitstreamMode && (val & (1 << 4))) {
        VpuWriteReg(BIT_RUN_INDEX, inst->instIndex);
        return RETCODE_FAILURE;
    }

    if (val == 0) {
        val = VpuReadReg(RET_DEC_SEQ_ERR_REASON);
        info->errorcode = val;
        return RETCODE_FAILURE;
    }

    val = VpuReadReg(RET_DEC_SEQ_SRC_SIZE);

    info->picWidth = ((val >> 16) & 0xffff);
    info->picHeight = (val & 0xffff);

    if (info->picWidth < 64 || info->picHeight < 64) {
        printf("info->picWidth < 64 || info->picHeight < 6 ---> Error\n");
        return RETCODE_NOT_SUPPORTED;
    }

    info->frameRateRes = VpuReadReg(RET_DEC_SEQ_FRATE_NR);
    info->frameRateDiv = VpuReadReg(RET_DEC_SEQ_FRATE_DR);
    info->bitRate = VpuReadReg(RET_DEC_SEQ_BIT_RATE);

    info->minFrameBufferCount = VpuReadReg(RET_DEC_SEQ_FRAME_NEED);
    info->frameBufDelay = VpuReadReg(RET_DEC_SEQ_FRAME_DELAY);

    if (inst->codecMode == AVC_DEC) {
        val = VpuReadReg(RET_DEC_SEQ_CROP_LEFT_RIGHT);
        val2 = VpuReadReg(RET_DEC_SEQ_CROP_TOP_BOTTOM);
        if (val == 0 && val2 == 0) {
            info->picCropRect.left = 0;
            info->picCropRect.right = 0;
            info->picCropRect.top = 0;
            info->picCropRect.bottom = 0;
        }else {
            info->picCropRect.left = ((val >> 16) & 0xFFFF);
            info->picCropRect.right = info->picWidth - ((val & 0xFFFF));
            info->picCropRect.top = ((val2 >> 16) & 0xFFFF);
            info->picCropRect.bottom = info->picHeight - ((val2 & 0xFFFF));
        }
        val = info->picWidth * info->picHeight;
        info->normalSliceSize = (val * 3 / 2) / 1024 / 4;
        info->worstSliceSize = ((val / 256) * 3200 / 8  + 512)/ 1024;
    } else {
        info->picCropRect.left = 0;
        info->picCropRect.right = 0;
        info->picCropRect.top = 0;
        info->picCropRect.bottom = 0;
    }

    val = VpuReadReg(RET_DEC_SEQ_HEADER_REPORT);
    info->profile = (val >> 0) & 0xFF;
    info->level = (val >> 8) & 0xFF;
    info->interlace  = (val >> 16) & 0x01;
    info->direct8x8Flag = (val >> 17) & 0x01;
    info->vc1_psf = (val >> 18) & 0x01;
    info->constraint_set_flag[0] = (val >> 19) & 0x01;
    info->constraint_set_flag[1] = (val >> 20) & 0x01;
    info->constraint_set_flag[2] = (val >> 21) & 0x01;
    info->constraint_set_flag[3] = (val >> 22) & 0x01;

    val = VpuReadReg(RET_DEC_SEQ_ASPECT);
    info->aspectRateInfo = val;

    info->streamInfoObtained = 1;

    pDecInfo->initialInfo = *info;
    pDecInfo->initialInfoObtained = 1;

    SetTiledMapTypeInfo(pDecInfo->mapType, &pDecInfo->sTiledInfo);
    /* Enable 2-D cache */
    SetMaverickCache(&pDecInfo->cacheConfig, pDecInfo->mapType,pDecInfo->openParam.chromaInterleave);

    EXIT_FUNC("vpu_DecGetInitialInfo");
    return RETCODE_SUCCESS;
}

/*
 * This function resets the VPU instance specified by handle that
 * exists in the current thread. If handle is not NULL, the instance 
 * of handle will be reset;
 */
RetCode vpu_SWReset(DecHandle handle, int index)
{
    CodecInst *inst;
    RetCode ret;

    ENTER_FUNC("vpu_SWReset");

    info_msg("vpu_SWReset\n");
    if (handle == NULL) {
        if (index < 0 || index >= MAX_NUM_INSTANCE)
            return RETCODE_FAILURE;

        /* Free instance info per index */
        inst = pCodecInst;
        if (inst == NULL)
            warn_msg("The instance is freed\n");
        else {
            // FreeCodecInstance(inst); :TODO just making inst->inuse = 0
            inst->inUse = 0;// :TODO Remove this if FreeCodecInstance is implemented
        }
        return RETCODE_SUCCESS;
    }

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity returned Error\n");
        return ret;
    }

    inst = handle;

    if (ppendingInst && (pCodecInst != ppendingInst))
        return RETCODE_FAILURE;
    else if (ppendingInst) {
        ppendingInst = 0;
    } 

    // TODO vpu_mx6_hwreset() -> Yet to implement
    // vpu_mx6_hwreset();
    printf("Hardware Reset yet To implement\n");

    return RETCODE_SUCCESS;
}

/*!
 * @brief Start decoding one frame.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 */
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam * param)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    DecParam *pDecParam;
    Uint32 reg = 0;
    RetCode ret;

    ENTER_FUNC("vpu_DecStartOneFrame");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity returned Error\n");
        return ret;
    }

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;
    pDecParam = &inst->CodecParam.decParam;
    memcpy(pDecParam, param, sizeof(*pDecParam));

    /* This means frame buffers have not been registered. */
    if (pDecInfo->frameBufPool == 0) {
        printf("FrameBuffer Not initailized, Wrong call sequence\n");
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    // Ignoring Rotaion Enable TODO
    // Ignoring Mirror Enable  TODO
    // Ignoring logtime TODO

    SetGDIRegs(&pDecInfo->sTiledInfo);

    // ignoring mx6x_mjpg

    // ignoring tiled enable
    // ignoring dering enable

    reg |= (1 << 10); /* hardcode to use interrupt disable mode  */

    /* if iframeSearch is Enable, other bit is ignored. */
    if (param->iframeSearchEnable == 1) {
        reg |= ((param->iframeSearchEnable & 0x1) << 2);
        pDecInfo->vc1BframeDisplayValid = 0;
    } else {
        if (param->skipframeMode)
            reg |= (param->skipframeMode << 3);
    }

    VpuWriteReg(CMD_DEC_PIC_OPTION, reg);
    VpuWriteReg(CMD_DEC_PIC_SKIP_NUM, param->skipframeNum);

    reg = (pDecInfo->secAxiUse.useBitEnable |
            pDecInfo->secAxiUse.useIpEnable << 1 |
            pDecInfo->secAxiUse.useDbkEnable << 2 |
            pDecInfo->secAxiUse.useDbkEnable << 3 |
            pDecInfo->secAxiUse.useOvlEnable << 4 |
            pDecInfo->secAxiUse.useBtpEnable << 5 |
            pDecInfo->secAxiUse.useHostBitEnable << 8 |
            pDecInfo->secAxiUse.useHostIpEnable << 9 |
            pDecInfo->secAxiUse.useHostDbkEnable << 10 |
            pDecInfo->secAxiUse.useHostDbkEnable << 11 |
            pDecInfo->secAxiUse.useHostOvlEnable << 12 |
            pDecInfo->secAxiUse.useHostBtpEnable << 13 );

    VpuWriteReg(BIT_AXI_SRAM_USE, reg);
    BitIssueCommand(inst, PIC_RUN);
    ppendingInst = inst;

    EXIT_FUNC("vpu_DecStartOneFrame");
    return RETCODE_SUCCESS;
}

/*!
 * @brief
 * This functure indicate whether processing(encoding/decoding) a frame
 * is completed or not yet.
 *
 * @return
 * @li 0: VPU hardware is idle.
 * @li Non-zero value: VPU hardware is busy processing a frame.
 */
int vpu_IsBusy()
{
    Uint32 vpu_busy = 0;

    ENTER_FUNC("vpu_IsBusy");

    vpu_busy = VpuReadReg(BIT_BUSY_FLAG);

    return (vpu_busy != 0);
}

/*!
 * Waits till the Interrurpt or the Timeout occurs
 *
 * @param -> time out in milli secs
 * @return -> 0 - on success
 * @return -> -1 - on failure
 */
int vpu_WaitForInt(int timeout_in_ms)
{
    int ret = -1;

    ret = IOWaitForInt(timeout_in_ms);
    printf("IOWaitForInt Returned: %d\n",ret);

    if(VpuReadReg(BIT_BUSY_FLAG)) {
        if (ret == 0) 
            printf("VPU is Still BUSY\n");
        ret = -1;
    } else {
        ret = 0;
    }
    return ret;
}

/*!
 * @brief Give command to the decoder.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param cmd [Intput] Command.
 * @param param [Intput/Output] param  for cmd.
 *  
 * @return
 * @li RETCODE_INVALID_COMMANDcmd is not valid.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 */
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *param)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    RetCode ret;

    ENTER_FUNC("vpu_DecGiveCommand");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;

    switch (cmd) {
        case DEC_SET_SPS_RBSP:
            {
                if (inst->codecMode != AVC_DEC) {
                    return RETCODE_INVALID_COMMAND;
                }
                if (param == 0) {
                    return RETCODE_INVALID_PARAM;
                }

                SetParaSet(handle, 0, param);
                break;
            }
        case DEC_SET_PPS_RBSP:
            {
                if (inst->codecMode != AVC_DEC) {
                    return RETCODE_INVALID_COMMAND;
                }
                if (param == 0) {
                    return RETCODE_INVALID_PARAM;
                }
                SetParaSet(handle, 1, param);
                break;
            }

        case ENABLE_DERING:
            {       

                pDecInfo->deringEnable = 1;
                break;
            }

        case DISABLE_DERING:
            {   
                pDecInfo->deringEnable = 0;
                break;
            }

        case SET_DBK_OFFSET :
            {   
                DbkOffset dbkOffset;
                dbkOffset = *(DbkOffset *)param;

                pDecInfo->dbkOffset.DbkOffsetA = dbkOffset.DbkOffsetA;
                pDecInfo->dbkOffset.DbkOffsetB = dbkOffset.DbkOffsetB;

                pDecInfo->dbkOffset.DbkOffsetEnable = ((pDecInfo->dbkOffset.DbkOffsetA !=0 ) && (pDecInfo->dbkOffset.DbkOffsetB != 0));
                break;
            }
        case DEC_SET_FRAME_DELAY:
            {
                pDecInfo->frame_delay = *(int *)param;
                break;
            }
        default:
            printf("Invalid CodecCommand - %d\n",cmd);
            return RETCODE_INVALID_COMMAND;
    }

    return RETCODE_SUCCESS;
}

/*!
 * @brief Update the current bit stream position.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param size [Input] Size of bit stream you put.
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_INVALID_PARAM Invalid input parameters.
 */
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    PhysicalAddress wrPtr;
    PhysicalAddress rdPtr;
    RetCode ret;
    int room = 0, instIndex, wrOffset;
    Uint32 val = 0;

    ENTER_FUNC("vpu_DecUpdateBitstreamBuffer");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity() returned Error!\n");
        return ret;
    }

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;
    wrPtr = pDecInfo->streamWrPtr;

    instIndex = (int)VpuReadReg(BIT_RUN_INDEX);
    printf("Dec Run Index: %d\n",instIndex);

    val = inst->ctxRegs[CTX_BIT_STREAM_PARAM];
    /* Set stream end flag if size == 0; otherwise, clear the flag */
    val = (size == 0) ? (val | 1 << 2) : (val & ~(1 << 2));
    /* Backup to context reg */
    inst->ctxRegs[CTX_BIT_STREAM_PARAM] = val;

    if (inst->instIndex == instIndex)
        VpuWriteReg(BIT_BIT_STREAM_PARAM, val); /* Write to vpu hardware */

    if (size == 0) {
        return RETCODE_SUCCESS;
    }

    rdPtr = (inst->instIndex == instIndex) ? VpuReadReg(BIT_RD_PTR) : inst->ctxRegs[CTX_BIT_RD_PTR];

    if (wrPtr < rdPtr) {
        if (rdPtr <= wrPtr + size) {
            printf("rdPtr <= wrPtr + size --- ERROR!! \n");
            return RETCODE_INVALID_PARAM;
        }
    }

    wrPtr += size;

    if (wrPtr > pDecInfo->streamBufEndAddr) {
        room = wrPtr - pDecInfo->streamBufEndAddr;
        wrPtr = pDecInfo->streamBufStartAddr;
        wrPtr += room;
    }

    if (wrPtr == pDecInfo->streamBufEndAddr) {
        wrPtr = pDecInfo->streamBufStartAddr;
    }

    pDecInfo->streamWrPtr = wrPtr;

    if (inst->instIndex == instIndex)
        VpuWriteReg(BIT_WR_PTR, wrPtr);
    inst->ctxRegs[CTX_BIT_WR_PTR] = wrPtr;

    printf("Updated BitstreamBuffer: \nSize: 0x%x \nrdPtr: 0x%x \nwrPtr: 0x%x\n ",room,rdPtr,wrPtr);

    EXIT_FUNC("vpu_DecUpdateBitstreamBuffer");
    return RETCODE_SUCCESS;
}

/*!
 * @brief Get bitstream for decoder.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param bufAddr [Output] Bitstream buffer physical address.
 * @param size [Output] Bitstream size.
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_INVALID_PARAM buf or size is invalid.
 */
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle,PhysicalAddress * paRdPtr,PhysicalAddress * paWrPtr, Uint32 * size)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    int instIndex;
    Uint32 room;
    RetCode ret;

    ENTER_FUNC("vpu_DecGetBitstreamBuffer");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }
    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity Returned Error!\n");
        return ret;
    }

    if (paRdPtr == 0 || paWrPtr == 0 || size == 0){
        printf("Read | Write Pointers or size is Zero! - Error\n");
        return RETCODE_INVALID_PARAM;
    }

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;
    wrPtr = pDecInfo->streamWrPtr;

    /* Check current instance is in running or not, if not
       Get the pointer from back context regs */
    instIndex = (int)VpuReadReg(BIT_RUN_INDEX);
    printf("Dec Run Index = %d | VPU returned Index: %d\n", inst->instIndex, instIndex);
    rdPtr = (inst->instIndex == instIndex) ? VpuReadReg(BIT_RD_PTR) : inst->ctxRegs[CTX_BIT_RD_PTR];

    if (wrPtr < rdPtr) {
        room = rdPtr - wrPtr - VPU_GBU_SIZE*2 - 1;
    } else {
        room = (pDecInfo->streamBufEndAddr - wrPtr) +
            (rdPtr - pDecInfo->streamBufStartAddr) - VPU_GBU_SIZE*2 - 1;
    }   

    *paRdPtr = rdPtr;
    *paWrPtr = wrPtr;
    *size = room;

    printf("BitstreamBuffer: \nSize: 0x%x \nrdPtr: 0x%x \nwrPtr: 0x%x\n ",room,rdPtr,wrPtr);
    EXIT_FUNC("vpu_DecGetBitstreamBuffer");

    return RETCODE_SUCCESS;
}

/*!
 * @brief Register decoder frame buffers.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param bufArray [Input] Pointer to the first element of an array of FrameBuffer.
 * @param num [Input] Number of elements of the array.
 * @param stride [Input] Stride value of frame buffers being registered.
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_FRAME_BUFFER Buffer is an invalid pointer.
 * @li RETCODE_INSUFFICIENT_FRAME_BUFFERS num is less than
 * the value requested by vpu_DecGetInitialInfo().
 * @li RETCODE_INVALID_STRIDE stride is less than the picture width.
 */
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,FrameBuffer * bufArray, int num, int stride,DecBufInfo * pBufInfo)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    int i;
    Uint32 val;
    RetCode ret;

    ENTER_FUNC("vpu_DecRegisterFrameBuffer");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }
    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity() Retured Error\n");
        return ret;
    }

    inst = handle;    
    pDecInfo = &inst->CodecInfo.decInfo;

    if (pDecInfo->frameBufPool) {
        printf("Frame Buff Pool = 1, Called Before - Error\n");
        return RETCODE_CALLED_BEFORE;
    }

    if (!pDecInfo->initialInfoObtained) {
        printf("Initial Info Not obtained, Wrong Sequence Call!\n");
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    if (bufArray == 0) {
        printf("bufArray == NULL\n");
        return RETCODE_INVALID_FRAME_BUFFER;
    }

    if (num < pDecInfo->initialInfo.minFrameBufferCount) {
        printf("Insufficient FrameBuffers\n");
        return RETCODE_INSUFFICIENT_FRAME_BUFFERS;
    }

    if (stride < pDecInfo->initialInfo.picWidth || stride % 8 != 0){
        printf("Invalid Stride\n");
        return RETCODE_INVALID_STRIDE;
    }

    pDecInfo->frameBufPool = bufArray;
    pDecInfo->numFrameBuffers = num;
    pDecInfo->stride = stride;


    /* none mx27 platform case need to swap word */
    for (i = 0; i < num; i += 2) {
        /* TODO Check
           if (pDecInfo->mapType == LINEAR_FRAME_MAP) {
           if (!(IOPhyMemCheck(bufArray[i].bufY, "bufY") && IOPhyMemCheck(bufArray[i].bufCb, "bufCb"))) {// TODO How to check if the physical Mem is valid?
           printf("bufArray[%d].bufY / bufArray[%d].bufCb are not a valid Physical Address - ERROR!\n",i,i);
           return RETCODE_INVALID_FRAME_BUFFER;
           }
           if (pDecInfo->openParam.chromaInterleave == 0) {
           if (!IOPhyMemCheck(bufArray[i].bufCr, "bufCr")) {// TODO How to check if the physical address is valid?
           printf("bufArray[%d].bufCr - Not a valid Physical Address\n",i);
           return RETCODE_INVALID_FRAME_BUFFER;
           }
           }
           }
           */
        virt_paraBuf[i * 3] = bufArray[i].bufCb;
        virt_paraBuf[i * 3 + 1] = bufArray[i].bufY;
        virt_paraBuf[i * 3 + 3] = bufArray[i].bufCr;

        virt_paraBuf[96 + i + 1] = bufArray[i].bufMvCol;

        if (i + 1 < num) {
            virt_paraBuf[i * 3 + 2] = bufArray[i + 1].bufY;
            virt_paraBuf[i * 3 + 4] = bufArray[i + 1].bufCr;
            virt_paraBuf[i * 3 + 5] = bufArray[i + 1].bufCb;

            virt_paraBuf[96 + i] = bufArray[i + 1].bufMvCol;
        }
    }

    /* Tell the decoder how much frame buffers were allocated. */
    VpuWriteReg(CMD_SET_FRAME_BUF_NUM, num);
    VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, stride);

    VpuWriteReg(CMD_SET_FRAME_AXI_BIT_ADDR, pDecInfo->secAxiUse.bufBitUse);
    VpuWriteReg(CMD_SET_FRAME_AXI_IPACDC_ADDR, pDecInfo->secAxiUse.bufIpAcDcUse);
    VpuWriteReg(CMD_SET_FRAME_AXI_DBKY_ADDR, pDecInfo->secAxiUse.bufDbkYUse);
    VpuWriteReg(CMD_SET_FRAME_AXI_DBKC_ADDR, pDecInfo->secAxiUse.bufDbkCUse);
    VpuWriteReg(CMD_SET_FRAME_AXI_OVL_ADDR, pDecInfo->secAxiUse.bufOvlUse);

    VpuWriteReg(CMD_SET_FRAME_AXI_BTP_ADDR, pDecInfo->secAxiUse.bufBtpUse);

    VpuWriteReg(CMD_SET_FRAME_DELAY, pDecInfo->frame_delay);

    val = (pDecInfo->cacheConfig.luma.cfg.PageSizeX << 28) | (pDecInfo->cacheConfig.luma.cfg.PageSizeY << 24) |
        (pDecInfo->cacheConfig.luma.cfg.CacheSizeX << 20) | (pDecInfo->cacheConfig.luma.cfg.CacheSizeY << 16) |
        (pDecInfo->cacheConfig.chroma.cfg.PageSizeX << 12) | (pDecInfo->cacheConfig.chroma.cfg.PageSizeY << 8) |
        (pDecInfo->cacheConfig.chroma.cfg.CacheSizeX << 4) | (pDecInfo->cacheConfig.chroma.cfg.CacheSizeY << 0);
    VpuWriteReg(CMD_SET_FRAME_CACHE_SIZE, val);

    val = (pDecInfo->cacheConfig.Bypass << 4) | (pDecInfo->cacheConfig.DualConf << 2) | (pDecInfo->cacheConfig.PageMerge << 0);
    val = val << 24;
    val |= (pDecInfo->cacheConfig.LumaBufferSize << 16) | (pDecInfo->cacheConfig.CbBufferSize << 8) | (pDecInfo->cacheConfig.CrBufferSize);
    VpuWriteReg(CMD_SET_FRAME_CACHE_CONFIG, val);

    if (inst->codecMode == VPX_DEC && inst->codecModeAux == VPX_AUX_VP8) {// TODO Check VPX_DEC
        VpuWriteReg(CMD_SET_FRAME_MB_BUF_BASE, pBufInfo->vp8MbDataBufInfo.bufferBase);
    }

    //if (inst->codecMode == AVC_DEC) {   
    VpuWriteReg( CMD_SET_FRAME_SLICE_BB_START, pBufInfo->avcSliceBufInfo.bufferBase);
    VpuWriteReg(CMD_SET_FRAME_SLICE_BB_SIZE, (pBufInfo->avcSliceBufInfo.bufferSize / 1024));
    //}

    if (pBufInfo->maxDecFrmInfo.maxMbNum == 0) {
        pBufInfo->maxDecFrmInfo.maxMbX = MAX_PIC_WIDTH/16;
        pBufInfo->maxDecFrmInfo.maxMbY = MAX_PIC_HEIGHT/16;
        pBufInfo->maxDecFrmInfo.maxMbNum = pBufInfo->maxDecFrmInfo.maxMbX * pBufInfo->maxDecFrmInfo.maxMbY;
    }

    VpuWriteReg(CMD_SET_FRAME_MAX_DEC_SIZE, (pBufInfo->maxDecFrmInfo.maxMbNum << 16 | pBufInfo->maxDecFrmInfo.maxMbX << 8 |
                pBufInfo->maxDecFrmInfo.maxMbY));

    BitIssueCommand(inst, SET_FRAME_BUF);

    while (VpuReadReg(BIT_BUSY_FLAG)) ;

    EXIT_FUNC("vpu_DecRegisterFrameBuffer");

    return RETCODE_SUCCESS;
}

/*!
 * @brief Decoder initialization
 *
 * @param pHandle [Output] Pointer to DecHandle type
 * @param pop [Input] Pointer to DecOpenParam type.
 *
 * @return
 * @li RETCODE_SUCCESS Success in acquisition of a decoder instance.
 * @li RETCODE_FAILURE Failed in acquisition of a decoder instance.
 * @li RETCODE_INVALID_PARAM pop is a null pointer or invalid.
 */
RetCode vpu_DecOpen(DecHandle * pHandle, DecOpenParam * pop)
{
    DecInfo *pDecInfo;
    int instIdx;
    Uint32 val;
    RetCode ret;

    ENTER_FUNC("vpu_DecOpen");

    ret = CheckDecOpenParam(pop);
    if (ret != RETCODE_SUCCESS) {
        err_msg("CheckDecOpenParam Returned Error");
        return ret;
    }

    if (!isVpuInitialized()) {
        err_msg("VPU Not Initialized");
        return RETCODE_NOT_INITIALIZED;
    }

    ret = GetCodecInstance(&pCodecInst);
    if (ret == RETCODE_FAILURE) {
        *pHandle = 0;
        err_msg("GetCodecInstance Returned Error");
        return RETCODE_FAILURE;
    }

    *pHandle = pCodecInst;
    instIdx = pCodecInst->instIndex;
    pDecInfo = &pCodecInst->CodecInfo.decInfo;
    /* Allocate context buffer */
    pCodecInst->contextBufMem.size = SIZE_CONTEXT_BUF;

#if 0
    // ret = IOGetPhyMem(&pCodecInst->contextBufMem);// In Linux, Allocating Physical and virtual Mapping
#endif
    ret = get_dec_work_addr(&pCodecInst->contextBufMem);

    if (ret == RETCODE_FAILURE) { // TODO Check virt memory is checked in this line
        err_msg("Unable to obtain Start Address mem\n");
        return RETCODE_FAILURE;
    }

    printf("Context Buffer Phy Mem Starting Address: %x\n",pCodecInst->contextBufMem.phy_addr);
    printf("Context Buffer Virt Mem Starting Address: %x\n",pCodecInst->contextBufMem.virt_uaddr);

    pDecInfo->openParam = *pop;

    switch (pop->bitstreamFormat) {
        case STD_AVC:
            pCodecInst->codecMode = AVC_DEC;
            pCodecInst->codecModeAux = pop->avcExtension;
            break;
        default:
            err_msg("Unable to obtain physical mem\n");
            return RETCODE_FAILURE;
    }

    pDecInfo->streamWrPtr = pop->bitstreamBuffer;
    pDecInfo->streamBufStartAddr = pop->bitstreamBuffer;
    pDecInfo->streamBufSize = pop->bitstreamBufferSize;
    pDecInfo->streamBufEndAddr = pop->bitstreamBuffer + pop->bitstreamBufferSize;

    pDecInfo->frameBufPool = 0;

    pDecInfo->deringEnable = 0;

    pDecInfo->initialInfoObtained = 0;
    pDecInfo->vc1BframeDisplayValid = 0;

    pDecInfo->frame_delay = -1;

    pDecInfo->mapType = pop->mapType;
    pDecInfo->tiledLinearEnable = pop->tiled2LinearEnable;
    pDecInfo->cacheConfig.Bypass = 1;

    pCodecInst->ctxRegs[CTX_BIT_RD_PTR] = pDecInfo->streamBufStartAddr;// :TODO Check
    pCodecInst->ctxRegs[CTX_BIT_WR_PTR] = pDecInfo->streamWrPtr;// :TODO Check
    pCodecInst->ctxRegs[CTX_BIT_FRM_DIS_FLG] = 0;// :TODO Check
    pCodecInst->ctxRegs[CTX_BIT_STREAM_PARAM] = 0;// :TODO Check

    if (instIdx == (int)VpuReadReg(BIT_RUN_INDEX)) {
        printf("Writing BIT_RD_PTR and BIT_WR_PTR in VPU\n");
        VpuWriteReg(BIT_RD_PTR, pDecInfo->streamBufStartAddr);
        VpuWriteReg(BIT_WR_PTR, pDecInfo->streamWrPtr);
        VpuWriteReg(BIT_FRM_DIS_FLG, 0);
    }

    val = VpuReadReg(BIT_FRAME_MEM_CTRL);
    val &= ~(1 << 2 | 1 << 3); /* clear the bit firstly */
    val &= 0x3f;

    if (pDecInfo->openParam.bitstreamMode)
        pCodecInst->ctxRegs[CTX_BIT_STREAM_PARAM] |= 1 << 3;

    if (pDecInfo->mapType)
        val |= (pDecInfo->tiledLinearEnable << 11 | 0x03 << 9);
    val |= 1 << 12;

    pCodecInst->ctxRegs[CTX_BIT_FRAME_MEM_CTRL] = val | (pDecInfo->openParam.chromaInterleave << 2);

    info_msg("bitstreamMode %d, chromaInterleave %d, mapType %d, tiled2LinearEnable %d\n",
            pop->bitstreamMode, pop->chromaInterleave, pop->mapType, pop->tiled2LinearEnable);

    EXIT_FUNC("vpu_DecOpen");
    return RETCODE_SUCCESS;
}

/*!
 * @brief Get the information of output of decoding.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param info [Output] Pointer to DecOutputInfo data structure.
 *
 * @return
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_PARAM Info is an invalid pointer.
 */
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    RetCode ret; 
    Uint32 val = 0; 
    Uint32 val2 = 0; 

    ENTER_FUNC("vpu_DecGetOutputInfo");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        err_msg("CheckInst, ret=%d\n", ret);
        return ret; 
    }    

    if (info == 0) { 
        return RETCODE_INVALID_PARAM;
    }    

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;

    if (ppendingInst == 0) {
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    if (inst != ppendingInst) {
        err_msg("inst 0x%p, pendingInst 0x%p\n", inst, ppendingInst);
        return RETCODE_INVALID_HANDLE;
    }

    memset(info, 0, sizeof(DecOutputInfo));

    if (VpuReadReg(BIT_BUSY_FLAG))
        err_msg("fatal: VPU is busy in %s\n", "vpu_DecGetOutputInfo");

    val = VpuReadReg(RET_DEC_PIC_SUCCESS);
    info->decodingSuccess = (val & 0x01);

    if (pDecInfo->openParam.bitstreamMode && (val & (1 << 4))) {
        info->decodingSuccess |= 0x10;
        VpuWriteReg(BIT_RUN_INDEX, inst->instIndex);
    }

    info->decodingSuccess |= val & (1 << 20);

    info->notSufficientPsBuffer = (val >> 3) & 0x1;
    info->notSufficientSliceBuffer = (val >> 2) & 0x1;

    val = VpuReadReg(RET_DEC_PIC_SIZE);     /* decoding picture size */
    info->decPicHeight = val & 0xFFFF;
    info->decPicWidth = (val >> 16) & 0xFFFF;

    info->frameStartPos = inst->ctxRegs[CTX_BIT_RD_PTR];
    info->frameEndPos = VpuReadReg(BIT_RD_PTR);

    if (info->frameEndPos < info->frameStartPos) {
        info->consumedByte = pDecInfo->streamBufEndAddr - info->frameStartPos;
        info->consumedByte += info->frameEndPos - pDecInfo->streamBufStartAddr;
    } else
        info->consumedByte = info->frameEndPos - info->frameStartPos;

    val = VpuReadReg(RET_DEC_PIC_CROP_LEFT_RIGHT);
    val2 = VpuReadReg(RET_DEC_PIC_CROP_TOP_BOTTOM);
    if (val == 0xFFFFFFFF && val2 == 0xFFFFFFFF) {
        /* Keep current crop information */
    } else if (val == 0 && val2 == 0) {
        info->decPicCrop.left = 0;
        info->decPicCrop.right = 0;
        info->decPicCrop.top = 0;
        info->decPicCrop.bottom = 0;
    } else {
        info->decPicCrop.left = ((val >> 16) & 0xFFFF);
        info->decPicCrop.right = info->decPicWidth - ((val & 0xFFFF));
        info->decPicCrop.top = ((val2 >> 16) & 0xFFFF);
        info->decPicCrop.bottom = info->decPicHeight - ((val2 & 0xFFFF));
    }

    val = VpuReadReg(RET_DEC_PIC_TYPE);

    info->picType = val & 0x7;

    info->picTypeFirst = (val & 0x38) >> 3;
    info->idrFlg = (val & 0xC0) >> 6;

    info->interlacedFrame = (val >> 16) & 0x1;

    info->h264Npf = (val >> 16) & 0x3;
    info->interlacedFrame = (val >> 18) & 0x1;
    info->pictureStructure = (val >> 19) & 0x0003;  /* MbAffFlag[17], FieldPicFlag[16] */
    info->topFieldFirst = (val >> 21) & 0x0001; /* TopFieldFirst[18] */
    info->repeatFirstField = (val >> 22) & 0x0001;

    info->progressiveFrame = (val >> 23) & 0x0003;
    info->fieldSequence = (val >> 25) & 0x0007;

    info->frameRateRes = VpuReadReg(RET_DEC_PIC_FRATE_NR);
    info->frameRateDiv = VpuReadReg(RET_DEC_PIC_FRATE_DR);
    if (inst->codecMode == AVC_DEC && info->frameRateDiv)
        info->frameRateDiv *= 2;

    info->aspectRateInfo = VpuReadReg(RET_DEC_PIC_ASPECT);

    info->numOfErrMBs = VpuReadReg(RET_DEC_PIC_ERR_MB);

    info->indexFrameDisplay = VpuReadReg(RET_DEC_PIC_FRAME_IDX);
    info->indexFrameDecoded = VpuReadReg(RET_DEC_PIC_CUR_IDX);

    /* save decoded picType to this array */
    if (info->indexFrameDecoded >= 0)
        pDecInfo->decoded_pictype[info->indexFrameDecoded] = info->picType;

    if (inst->codecModeAux == AVC_AUX_MVC) {
        val = VpuReadReg(RET_DEC_PIC_MVC_REPORT);
        info->mvcPicInfo.viewIdxDisplay = val & 1;
        info->mvcPicInfo.viewIdxDecoded = (val >> 1) & 1;
    }

    val = VpuReadReg(RET_DEC_PIC_AVC_FPA_SEI0);

    if ((int)val < 0)
        info->avcFpaSei.exist = 0;
    else {
        info->avcFpaSei.exist = 1;
        info->avcFpaSei.frame_packing_arrangement_id = val & 0x7FFFFFFF;

        val = VpuReadReg(RET_DEC_PIC_AVC_FPA_SEI1);
        info->avcFpaSei.content_interpretation_type = val & 0x3F;
        info->avcFpaSei.frame_packing_arrangement_type = (val >> 6) & 0x7F;
        info->avcFpaSei.frame_packing_arrangement_ext_flag = (val >> 13) & 0x01;
        info->avcFpaSei.frame1_self_contained_flag = (val >> 14) & 0x01;
        info->avcFpaSei.frame0_self_contained_flag = (val >> 15) & 0x01;
        info->avcFpaSei.current_frame_is_frame0_flag = (val >> 16) & 0x01;
        info->avcFpaSei.field_views_flag = (val >> 17) & 0x01;
        info->avcFpaSei.frame0_flipped_flag = (val >> 18) & 0x01;
        info->avcFpaSei.spatial_flipping_flag = (val >> 19) & 0x01;
        info->avcFpaSei.quincunx_sampling_flag = (val >> 20)&0x01;
        info->avcFpaSei.frame_packing_arrangement_cancel_flag = (val >> 21) & 0x01;

        val = VpuReadReg(RET_DEC_PIC_AVC_FPA_SEI2);
        info->avcFpaSei.frame_packing_arrangement_repetition_period = val & 0x7FFF;
        info->avcFpaSei.frame1_grid_position_y = (val >> 16) & 0x0F;
        info->avcFpaSei.frame1_grid_position_x = (val >> 20) & 0x0F;
        info->avcFpaSei.frame0_grid_position_y = (val >> 24) & 0x0F;
        info->avcFpaSei.frame0_grid_position_x = (val >> 28) &0x0F;
    }

    /* Backup context regs, no need to save BIT_WR_PTR
       and BIT_FRAME_MEM_CTRL since f/w doesn't update the registers */
    inst->ctxRegs[CTX_BIT_FRM_DIS_FLG] = VpuReadReg(BIT_FRM_DIS_FLG);
    inst->ctxRegs[CTX_BIT_RD_PTR] = VpuReadReg(BIT_RD_PTR);
    inst->ctxRegs[CTX_BIT_STREAM_PARAM] = VpuReadReg(BIT_BIT_STREAM_PARAM);

    ppendingInst = 0;

    return RETCODE_SUCCESS;
}

/*!
 * Clearing Buffer Display Flag
 */
RetCode vpu_DecClrDispFlag(DecHandle handle, int index)
{
    CodecInst *inst;
    DecInfo *pDecInfo;
    RetCode ret;
    int val;

    ENTER_FUNC("vpu_DecClrDispFlag");

    if (pCodecInst != handle) {
        printf("Invalid Handle Returned Error\n");
        return RETCODE_INVALID_HANDLE;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS){
        printf("CheckDecInstanceValidity returned ERROR\n");
        return ret;
    }

    inst = handle;
    pDecInfo = &inst->CodecInfo.decInfo;

    /* This means frame buffers have not been registered. */
    if (pDecInfo->frameBufPool == 0) {
        printf("Wrong Call Sequence call - ERROR\n");
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    if ((index < 0) || (index > (pDecInfo->numFrameBuffers - 1))){
        printf("Index < 0 or Index > numFrameBuffers -- ERROR \n");
        return RETCODE_INVALID_PARAM;
    }

    val = (~(1 << index) & inst->ctxRegs[CTX_BIT_FRM_DIS_FLG]);
    inst->ctxRegs[CTX_BIT_FRM_DIS_FLG] = val;

    ENTER_FUNC("vpu_DecClrDispFlag");

    return RETCODE_SUCCESS;
}
