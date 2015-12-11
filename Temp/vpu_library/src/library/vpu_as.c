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
 *  03/12/15    04      Separated from Lib files    Shreyas
 *                      Only main and App functions 
 *                      are present
 *                      decode_start funtion added
 *  08/12/15    05      Removed ReadFileWithTiming
 *                      Added freadn() funtion
 *                      Added src_input and output
 *                      in the cmdl, along with 
 *                      corresponding fds           Shreyas
 ******************************************************************************/ 

#include <INTEGRITY.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "vpu_lib/vpu_lib.h"
#include "vpu_lib/vpu_reg.h"
#include "vpu_lib/vpu_io.h"
#include "vpu_lib/vpu_debug.h"
#include "vpu_lib/vpu_gdi.h"
#include "vpu_lib/vpu_util.h"
// :TODO Application API - Remove it once Segragation of API and Lib is done
#include "application/vpu_test.h"

void print_codec_inst(CodecInst *inst){
   printf("instIndex = %d\n",inst->instIndex);
   printf("inUse = %d\n",inst->inUse);
   printf("codecMode = %d\n",inst->codecMode);
   printf("codecModeAux = %d\n",inst->codecModeAux);
   printf("contextBufMem:\n");
   printf("\t size = %d\n",inst->contextBufMem.size);
   printf("\t phy_addr = 0x%x\n",inst->contextBufMem.phy_addr);
   printf("\t cpu_addr = 0x%x\n",inst->contextBufMem.cpu_addr);
   printf("\t virt_uaddr = 0x%x\n",inst->contextBufMem.virt_uaddr);
   printf("ctxRegs:\n");
   printf("\t ctxRegs[CTX_BIT_STREAM_PARAM] = %l\n",inst>ctxRegs[CTX_BIT_STREAM_PARAM]);
   printf("\t ctxRegs[CTX_BIT_FRM_DIS_FLG] = %l\n",inst>ctxRegs[CTX_BIT_FRM_DIS_FLG]);
   printf("\t ctxRegs[CTX_BIT_WR_PTR] = %l\n",inst>ctxRegs[CTX_BIT_WR_PTR]);
   printf("\t ctxRegs[CTX_BIT_RD_PTR] = %l\n",inst>ctxRegs[CTX_BIT_RD_PTR]);
   printf("\t ctxRegs[CTX_BIT_FRAME_MEM_CTRL] = %l\n",inst>ctxRegs[CTX_BIT_FRAME_MEM_CTRL]);
   printf("\t ctxRegs[CTX_MAX_REGS] = %l\n",inst>ctxRegs[CTX_MAX_REGS]);

}

void print_val_in_dec(){
    
}


/*!
 * Read n bytes from a file descriptor 
 */
int freadn(int fd, void *vptr, size_t n)
{
    int nleft = 0;
    int nread = 0;
    char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) <= 0) {
            if (nread == 0)
                return (n - nleft);

            perror("read");
            return (-1);            /* error EINTR XXX */
        }   

        nleft -= nread;
        ptr   += nread;
    }   
    return (n - nleft);
}

/*
 * Fill the bitstream ring buffer
 */
int dec_fill_bsbuffer(DecHandle handle, struct cmd_line *cmd, Uint32 bs_va_startaddr, Uint32 bs_va_endaddr,
        Uint32 bs_pa_startaddr, int defaultsize,int *eos, int *fill_end_bs)
{
    RetCode ret;
    PhysicalAddress pa_read_ptr, pa_write_ptr;
    Uint32 target_addr, space;
    int size;
    int nread, room;
    *eos = 0;
    ENTER_FUNC("dec_fill_bsbuffer");
    ret = vpu_DecGetBitstreamBuffer(handle, &pa_read_ptr, &pa_write_ptr,&space);

    printf ("The left space is %d  def szie %d, read_ptr = 0x%x , write_ptr = 0x%x \n", space, defaultsize,pa_read_ptr,pa_write_ptr);
    if (ret != RETCODE_SUCCESS) {
        err_msg("vpu_DecGetBitstreamBuffer failed\n");
        return -1;
    }

    /* Decoder bitstream buffer is empty */
    if (space <= 0){
        warn_msg("space %lu <= 0\n",space);
        return 0;
    }

    if (defaultsize > 0) {
        if (space < defaultsize)
            return 0;

        size = defaultsize;
    } else {
        size = ((space >> 9) << 9);
    }

    if (size == 0){
        warn_msg("size == 0, space %lu\n", space);
        return 0;
    }

    /* Fill the bitstream buffer */
    target_addr = bs_va_startaddr + (pa_write_ptr - bs_pa_startaddr);
    printf("Target Address: 0x%x\n",target_addr);
    if ( (target_addr + size) > bs_va_endaddr) {
        room = bs_va_endaddr - target_addr;
        printf("BitStream To read: %d bytes \n",room);
        nread = freadn(cmd->src_fd,(void *)target_addr,room);
        printf("BitStream Read: %d bytes \n",nread);
        if (nread <= 0) {
            /* EOF or error */
            if (nread < 0) {
                if (nread == -EAGAIN)
                    return 0;
                //err_msg("nread %d < 0\n", nread);

                printf("========== EOF ===============\n");
                return -1;
            }
            printf("========== EOS ===============\n");

            *eos = 1;
        } else {
            /* unable to fill the requested size, so back off! */
            if (nread != room)
                goto update;

            /* read the remaining */
            space = nread;
            nread = freadn(cmd->src_fd,(void *)bs_va_startaddr,(size - room));
            if (nread <= 0) {
                /* EOF or error */
                if (nread < 0) {
                    if (nread == -EAGAIN)
                        return 0;
                    printf("========== EOF ===============\n");
                    return -1;
                }
                printf("========== EOS ===============\n");

                *eos = 1;
            }

            nread += space;
        }
    } else {
        printf("BitStream To read: %d bytes\n",size);
        nread = freadn(cmd->src_fd,(void *)target_addr, size);
        printf("BitStream Read: %d bytes \n",nread);
        if (nread <= 0) {
            /* EOF or error */
            if (nread < 0) {
                if (nread == -EAGAIN)
                    return 0;
                printf("========== EOF ===============\n");
                return -1;
            }

            printf("========== EOS ===============\n");
            *eos = 1;
        }
    }

update:
    printf("Update Called!\n");
    if (*eos == 0) {
        ret = vpu_DecUpdateBitstreamBuffer(handle, nread);
        if (ret != RETCODE_SUCCESS) {
            err_msg("vpu_DecUpdateBitstreamBuffer failed\n");
            return -1;
        }
        *fill_end_bs = 0;
    } else {
        if (!*fill_end_bs) {
            ret = vpu_DecUpdateBitstreamBuffer(handle,STREAM_END_SIZE);
            if (ret != RETCODE_SUCCESS) {
                err_msg("vpu_DecUpdateBitstreamBuffer failed"
                        "\n");
                return -1;
            }
            *fill_end_bs = 1;
        }

    }
    return nread;
}

int decoder_parse(struct decode *dec)
{
    DecInitialInfo initinfo = {0};
    DecHandle handle = dec->handle;
    int align, extended_fbcount;
    RetCode ret;

    /* Parse bitstream and get width/height/framerate etc */
    vpu_DecSetEscSeqInit(handle, 1);
    ret = vpu_DecGetInitialInfo(handle, &initinfo);
    vpu_DecSetEscSeqInit(handle, 0);

    if (ret != RETCODE_SUCCESS) {
        printf("vpu_DecGetInitialInfo failed, ret:%d, errorcode:%ld\n",ret, initinfo.errorcode);
        return -1;
    }
    if (initinfo.streamInfoObtained) {
        switch (dec->cmdl->format) {
            case STD_AVC:
                printf("Initial AVC Info:");
                printf("H.264 Profile: %d Level: %d Interlace: %d\n",initinfo.profile, initinfo.level, initinfo.interlace);
                if (initinfo.aspectRateInfo) {
                    int aspect_ratio_idc;
                    int sar_width, sar_height;

                    if ((initinfo.aspectRateInfo >> 16) == 0) {
                        aspect_ratio_idc = (initinfo.aspectRateInfo & 0xFF);
                        printf("aspect_ratio_idc: %d\n", aspect_ratio_idc);
                    } else {
                        sar_width = (initinfo.aspectRateInfo >> 16) & 0xFFFF;
                        sar_height = (initinfo.aspectRateInfo & 0xFFFF);
                        printf("sar_width: %d, sar_height: %d\n",sar_width, sar_height);
                    }   
                } else {
                    printf("Aspect Ratio is not present.\n");
                }
                break;

            default:
                printf("No other Format is allowed other than STD_AVC!\n format used now: %d",dec->cmdl->format);
                return -1;
        }
    }
    printf("Decoder: width = %d, height = %d, frameRateRes = %d, frameRateDiv = %d, count = %u\n",
            initinfo.picWidth, initinfo.picHeight,
            initinfo.frameRateRes, initinfo.frameRateDiv,
            initinfo.minFrameBufferCount);

    dec->minfbcount = initinfo.minFrameBufferCount;
    extended_fbcount = 2;

    if (initinfo.interlace)
        dec->regfbcount = dec->minfbcount + extended_fbcount + 2;
    else
        dec->regfbcount = dec->minfbcount + extended_fbcount;

    dec->picwidth = ((initinfo.picWidth + 15) & ~15);

    align = 16;
    if (initinfo.interlace == 1)
        align = 32;

    dec->picheight = ((initinfo.picHeight + align - 1) & ~(align - 1));
    if ((dec->picwidth == 0) || (dec->picheight == 0))
        return -1;
    // TODO REMOVE The Rot and Crop things
    if ((dec->picwidth > initinfo.picWidth || dec->picheight > initinfo.picHeight) && (!initinfo.picCropRect.left &&
                !initinfo.picCropRect.top && !initinfo.picCropRect.right && !initinfo.picCropRect.bottom)) {
        initinfo.picCropRect.left = 0;
        initinfo.picCropRect.top = 0;
        initinfo.picCropRect.right = initinfo.picWidth;
        initinfo.picCropRect.bottom = initinfo.picHeight;
    }

    printf("CROP left/top/right/bottom %lu %lu %lu %lu\n",initinfo.picCropRect.left, initinfo.picCropRect.top,
            initinfo.picCropRect.right, initinfo.picCropRect.bottom);
    memcpy(&(dec->picCropRect), &(initinfo.picCropRect),sizeof(initinfo.picCropRect));

    dec->phy_slicebuf_size = initinfo.worstSliceSize * 1024;
    dec->stride = dec->picwidth;


    printf("Display fps will be %d\n", dec->cmdl->fps);// What to set the FPS as? :TODO
    return 0;
}

int decoder_allocate_framebuffer(struct decode *dec){
    DecBufInfo bufinfo;
    int i, regfbcount = dec->regfbcount, totalfb, mvCol;
    int rot_en = dec->cmdl->rot_en;// :TODO Check
    int deblock_en = dec->cmdl->deblock_en;// :TODO Check
    int dering_en = dec->cmdl->dering_en;
    int tiled2LinearEnable =  dec->tiled2LinearEnable;
    RetCode ret;
    DecHandle handle = dec->handle;
    FrameBuffer *fb;
    struct frame_buf **pfbpool;
    int stride;

    totalfb = regfbcount + dec->extrafb;// TODO Check
    printf("Total FrameBuffers Used: %d\n",totalfb);

    fb = dec->fb = calloc(totalfb, sizeof(FrameBuffer));
    if (fb == NULL) {
        err_msg("Failed to allocate fb\n");
        return -1;
    }
    printf("dec->fb calloced\n");

    pfbpool = dec->pfbpool = calloc(totalfb, sizeof(struct frame_buf *));
    if (pfbpool == NULL) {
        err_msg("Failed to allocate pfbpool\n");
        free(dec->fb);
        dec->fb = NULL;
        return -1;
    }
    printf("dec->pfbpool calloced\n");

    mvCol = 1;

    if (dec->cmdl->mapType == LINEAR_FRAME_MAP) {
        /* All buffers are linear */
        for (i = 0; i < totalfb; i++) {
            pfbpool[i] = framebuf_alloc(dec->cmdl->format, dec->mjpg_fmt,dec->stride, dec->picheight, mvCol);// :TODO
            if (pfbpool[i] == NULL){
                printf("pfbpool[%d] == NULL\n",i);
                // :TODO Free pfbpool
                goto err;
            }
            printf("fb[%d]->fb's Phy Start Addr: 0x%x\n",i,pfbpool[i]->desc.phy_addr);
            printf("fb[%d]->fb's Virt Start Addr: 0x%x\n",i,pfbpool[i]->desc.virt_uaddr);
        }
    } else {// Ignring else part tiled_framebuf_alloc
        printf("Tiled_frame_Buf Not codded -- ERROR\n");
        return -1;
    }

    for (i = 0; i < totalfb; i++) {
        fb[i].myIndex = i;
        fb[i].bufY = pfbpool[i]->addrY;
        fb[i].bufCb = pfbpool[i]->addrCb;
        fb[i].bufCr = pfbpool[i]->addrCr;
        fb[i].bufMvCol = pfbpool[i]->mvColBuf;

        printf("\nfb[%d]:\n ",i);
        printf("bufY = 0x%x\n",fb[i].bufY);
        printf("bufCb = 0x%x\n",fb[i].bufCb);
        printf("bufCr = 0x%x\n",fb[i].bufCr);
        printf("bufMvCol = 0x%x\n",fb[i].bufMvCol);
    }

    stride = ((dec->stride + 15) & ~15);

    bufinfo.avcSliceBufInfo.bufferBase = dec->phy_slice_buf;
    bufinfo.avcSliceBufInfo.bufferSize = dec->phy_slicebuf_size;

    /* User needs to fill max suported macro block value of frame as following*/
    bufinfo.maxDecFrmInfo.maxMbX = dec->stride / 16;
    bufinfo.maxDecFrmInfo.maxMbY = dec->picheight / 16;
    bufinfo.maxDecFrmInfo.maxMbNum = dec->stride * dec->picheight / 256;
    ret = vpu_DecRegisterFrameBuffer(handle, fb, dec->regfbcount, stride, &bufinfo);
    if (ret != RETCODE_SUCCESS) {
        err_msg("Register frame buffer failed, ret=%d\n", ret);
        goto err;
    }
    return 0;

err:
    for (i = 0; i < totalfb; i++) {
        framebuf_free(pfbpool[i]);// :TODO Check Freee is not done properly
    }
    free(dec->pfbpool);
    dec->fb = NULL;
    dec->pfbpool = NULL;
    return -1;
}

/*
 *YUV image copy from on-board memory of tiled YUV to host buffer
 *
 int SaveTiledYuvImageHelper(struct decode *dec, int yuvFp,int picWidth, int picHeight, int index)
 {
 int frameSize, pix_addr, offset;
 int y, x, nY, nCb, j;
 Uint8 *puc, *pYuv = 0, *dstAddrCb, *dstAddrCr;
 Uint32 addrY, addrCb, addrCr;
 Uint8 temp_buf[8];
 struct frame_buf *pfb = NULL;

 frameSize = picWidth * picHeight * 3 / 2;
 pfb = dec->pfbpool[index];

 pYuv = malloc(frameSize);
 if (!pYuv) {
 err_msg("Fail to allocate memory\n");
 return -1;
 }
 }
 */

/*!
 * write n bytes to a file descriptor 
 */
int fwriten(int fd, void *vptr, size_t n)
{
    int nleft;
    int nwrite;
    char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwrite = write(fd, ptr, nleft)) <= 0) {
            perror("fwrite: ");
            return (-1);
        }

        nleft -= nwrite;
        ptr   += nwrite;
    }
    return (n);
}

/*
 * This function is to convert framebuffer from interleaved Cb/Cr mode
 * to non-interleaved Cb/Cr mode.
 *
 * Note: This function does _NOT_ really store this framebuffer into file.
 */
static void saveNV12ImageHelper(Uint8 *pYuv, struct decode *dec, Uint8 *buf)
{
    int Y, Cb;
    Uint8 *Y1, *Cb1, *Cr1;
    int img_size;
    int y, x;
    Uint8 *tmp;
    int height = dec->picheight;
    int stride = dec->stride;

    if (!pYuv || !buf) {
        err_msg("pYuv or buf should not be NULL.\n");
        return;
    }

    img_size = stride * height;

    Y = (int)buf;
    Cb = Y + img_size;

    Y1 = pYuv;
    Cb1 = Y1 + img_size;
    Cr1 = Cb1 + (img_size >> 2);

    memcpy(Y1, (Uint8 *)Y, img_size);

    for (y = 0; y < (dec->picheight / 2); y++) {
        tmp = (Uint8*)(Cb + dec->picwidth * y);
        for (x = 0; x < dec->picwidth; x += 2) {
            *Cb1++ = tmp[x];
            *Cr1++ = tmp[x + 1];
        }
    }
}

/*
 * This function is to store the framebuffer into file.
 */
static void write_to_file(struct decode *dec, int index)
{
    int height = (dec->picheight + 15) & ~15 ;
    int stride = dec->stride;
    int chromaInterleave = dec->cmdl->chromaInterleave;
    int img_size;
    Uint8 *pYuv = NULL, *pYuv0 = NULL, *buf;
    struct frame_buf *pfb = NULL;

    // TODO Check if this is needed? line 847
    /*if ((dec->cmdl->mapType != LINEAR_FRAME_MAP) && !dec->tiled2LinearEnable) {
      SaveTiledYuvImageHelper(dec, dec->cmdl->dst_fd, stride, height, index);
      goto out;
      }*/

    pfb = dec->pfbpool[index];
    buf = (Uint8 *)(pfb->addrY + pfb->desc.virt_uaddr - pfb->desc.phy_addr);

    img_size = stride * height * 3 / 2;

    if (chromaInterleave == 0 ) {
        fwriten(dec->cmdl->dst_fd, buf, img_size);
    } else {
        printf("ChromaInterleave Enabled!\n");
        /*
         * There could be these three cases:
         * interleave == 0, cropping == 1
         * interleave == 1, cropping == 0
         * interleave == 1, cropping == 1
         */
        if (!pYuv) {
            pYuv0 = pYuv = malloc(img_size);
            if (!pYuv)
                err_msg("malloc error\n");
        }

        saveNV12ImageHelper(pYuv, dec, buf);
        fwriten(dec->cmdl->dst_fd, (Uint8 *)pYuv0, img_size);             
    }

    if (pYuv0) {
        free(pYuv0);
        pYuv0 = NULL;
        pYuv = NULL;
    }

    return;    
}

int decoder_start(struct decode *dec)
{
    DecHandle handle = dec->handle;// Handle
    DecOutputInfo outinfo = {0}; // DecOutput Info
    DecParam decparam = {0}; 
    int rot_stride, fwidth, fheight;
    int deblock_en = dec->cmdl->deblock_en;
    int dering_en = dec->cmdl->dering_en;
    FrameBuffer *deblock_fb = NULL;
    FrameBuffer *fb = dec->fb;
    struct frame_buf **pfbpool = dec->pfbpool;
    struct frame_buf *pfb = NULL;
    int err = 0, eos = 0, fill_end_bs = 0, decodefinish = 0; 
    RetCode ret; 
    int loop_id;
    Uint32 img_size;
    int frame_id = 0;
    int decIndex = 0; 
    int rotid = 0, dblkid = 0;
    int count = dec->cmdl->count;
    int totalNumofErrMbs = 0; 
    int disp_clr_index = -1, actual_display_index = -1;
    int is_waited_int = 0; 
    int tiled2LinearEnable = dec->tiled2LinearEnable;

    printf("Dering: %d, tiled2Len: %d \n",dering_en, tiled2LinearEnable);
    printf("deblock_en: %d \n",deblock_en);

    if (dering_en || tiled2LinearEnable) {
        rotid = dec->regfbcount;
        if (deblock_en) {
            dblkid = dec->regfbcount + dec->rot_buf_count;
        }
    } else if (deblock_en) {
        dblkid = dec->regfbcount;
    }

    decparam.dispReorderBuf = 0;
    decparam.skipframeMode = 0;
    decparam.skipframeNum = 0;
    /*
     * once iframeSearchEnable is enabled, prescanEnable, prescanMode
     * and skipframeMode options are ignored.
     */
    decparam.iframeSearchEnable = 0;

    fwidth = ((dec->picwidth + 15) & ~15);
    fheight = ((dec->picheight + 15) & ~15);

    if (deblock_en) {
        deblock_fb = &fb[dblkid];
    }

    img_size = dec->picwidth * dec->picheight * 3 / 2;
    if (deblock_en) {
        pfb = pfbpool[dblkid];
        deblock_fb->bufY = pfb->addrY;
        deblock_fb->bufCb = pfb->addrCb;
        deblock_fb->bufCr = pfb->addrCr;
    }

    while (1) {

        if (dering_en || tiled2LinearEnable) { // TODO Check
            vpu_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, (void *)&fb[rotid]);
            if (frame_id == 0) {
                if (dering_en) {
                    vpu_DecGiveCommand(handle, ENABLE_DERING, 0);
                }
            }
        }

        if (deblock_en) {   
            ret = vpu_DecGiveCommand(handle, DEC_SET_DEBLOCK_OUTPUT, (void *)deblock_fb);
            if (ret != RETCODE_SUCCESS) {
                err_msg("Failed to set deblocking output\n");
                return -1;  
            }
        }

        ret = vpu_DecStartOneFrame(handle, &decparam);
        if (ret != RETCODE_SUCCESS) {
            err_msg("DecStartOneFrame failed, ret=%d\n", ret);
            return -1;
        }

        is_waited_int = 0;
        loop_id = 0;

        while (vpu_IsBusy()) {
            err = dec_fill_bsbuffer(handle, dec->cmdl,dec->virt_bsbuf_addr,(dec->virt_bsbuf_addr + STREAM_BUF_SIZE), dec->phy_bsbuf_addr, STREAM_FILL_SIZE, &eos, &fill_end_bs);
            if (err < 0) {
                err_msg("dec_fill_bsbuffer failed\n");
                return -1;
            }

            /*
             * Suppose vpu is hang if one frame cannot be decoded in 5s,
             * then do vpu software reset.
             * Please take care of this for network case since vpu
             * interrupt also cannot be received if no enough data.
             */
            if (loop_id == 50) {
                err = vpu_SWReset(handle, 0);// TODO imp vpu_SWReset
                return -1;
            }

            vpu_WaitForInt(100);
            is_waited_int = 1;
            loop_id ++;
        }

        if (!is_waited_int) 
            vpu_WaitForInt(100);

        ret = vpu_DecGetOutputInfo(handle, &outinfo);

        /* In 8 instances test, we found some instance(s) may not get a chance to be scheduled
         * until timeout, so we yield schedule each frame explicitly.
         * This may be kernel dependant and may be removed on customer platform */
        usleep(0);

        printf("frame_id %d, decidx %d, disidx %d, rotid %d, decodingSuccess 0x%x\n",
                (int)frame_id, outinfo.indexFrameDecoded, outinfo.indexFrameDisplay,
                rotid, outinfo.decodingSuccess);

        if (ret != RETCODE_SUCCESS) {
            err_msg("vpu_DecGetOutputInfo failed Err code is %d\n\tframe_id = %d\n", ret, (int)frame_id);
            return -1;
        }

#ifdef test
        {
            FILE *fp_out = fopen("/sda/frame0.yuv","wb");
            if(fp_out!=NULL)
            {
                struct frame_buf *ptr = *(dec->pfbpool);
                printf("fb out address 0x%x\n",ptr[outinfo.indexFrameDisplay].desc.virt_uaddr);
                fwrite(ptr[outinfo.indexFrameDisplay].desc.virt_uaddr,1,320*240*3/2,fp_out);
                fclose(fp_out);
            }
        }
#endif

        if (outinfo.decodingSuccess == 0) {
            warn_msg("Incomplete finish of decoding process.\n\tframe_id = %d\n", (int)frame_id);
            continue;
        }

        if (outinfo.decodingSuccess & 0x10) {
            warn_msg("vpu needs more bitstream in rollback mode\n\tframe_id = %d\n", (int)frame_id);

            err = dec_fill_bsbuffer(handle,  dec->cmdl, dec->virt_bsbuf_addr,(dec->virt_bsbuf_addr + STREAM_BUF_SIZE),
                    dec->phy_bsbuf_addr, 0, &eos, &fill_end_bs);
            if (err < 0) {
                err_msg("dec_fill_bsbuffer failed\n");
                return -1;
            }
            continue;
        }

        if (outinfo.decodingSuccess & 0x100000)
            warn_msg("sequence parameters have been changed\n");

        if (outinfo.notSufficientPsBuffer) {
            err_msg("PS Buffer overflow\n");
            return -1;
        }           

        if (outinfo.notSufficientSliceBuffer) {
            err_msg("Slice Buffer overflow\n");
            return -1;
        } 

        // TODO Check the field variable in the position in linux dec.c line no. 1318
        printf("Top Field First flag: %d, dec_idx %d\n", outinfo.topFieldFirst, decIndex);

        if (outinfo.indexFrameDisplay == -1)
            decodefinish = 1;

        if (decodefinish && (!(dering_en || tiled2LinearEnable)))
            break;

        if(outinfo.indexFrameDecoded >= 0) {
            if (outinfo.numOfErrMBs) {
                totalNumofErrMbs += outinfo.numOfErrMBs;
                info_msg("Num of Error Mbs : %d, in Frame : %d \n", outinfo.numOfErrMBs, decIndex);
            }
        }

        if(outinfo.indexFrameDecoded >= 0)
            decIndex++;

        /* BIT don't have picture to be displayed */
        if ((outinfo.indexFrameDisplay == -3) || (outinfo.indexFrameDisplay == -2)) {
            printf("VPU doesn't have picture to be displayed.\n\toutinfo.indexFrameDisplay = %d\n", outinfo.indexFrameDisplay);
            continue;
        }

        if ( dering_en || tiled2LinearEnable) {
            /* delay one more frame for PP */
            if (disp_clr_index < 0) {
                disp_clr_index = outinfo.indexFrameDisplay;
                continue;
            }
            actual_display_index = rotid;
        }
        else
            actual_display_index = outinfo.indexFrameDisplay;

        write_to_file(dec, actual_display_index);

        if ( disp_clr_index >= 0 ) {
            err = vpu_DecClrDispFlag(handle,disp_clr_index);
            if (err)
                err_msg("vpu_DecClrDispFlag failed Error code - %d\n", err);
        }
        disp_clr_index = outinfo.indexFrameDisplay;

        frame_id++;
        if ((count != 0) && (frame_id >= count))
            break;

        // In linux - sleep time is read from the env 
        usleep(1000);// TODO Check if the hardcode is ok?

        if (decodefinish)
            break;

        printf("End of While\n");
    }

    if (totalNumofErrMbs) {
        info_msg("Total Num of Error MBs : %d\n", totalNumofErrMbs);
    }

    info_msg("Number of frames Decoded: %d \n", (int)frame_id);

    printf("decoder_start function ended\n");

    return 0;
}

#if 0
/*!
 * Temporary Application Entry 1
 * For vpu_GetVersionInfo
 */
int main( void )
{
    RetCode status;

    status = vpu_Init();
    if(status == RETCODE_SUCCESS){
        vpu_versioninfo vpu_ver;

        printf("Vpu_Init Returns Success\n");

        vpu_GetVersionInfo(&vpu_ver);
        printf("firmware major version: %d \n",vpu_ver.fw_major);
        printf("firmware minor version: %d \n",vpu_ver.fw_minor);
        printf("firmware release version: %d \n",vpu_ver.fw_release);
        printf("firmware checkin code number: %d \n",vpu_ver.fw_code);
        printf("library major version: %d \n",vpu_ver.lib_major);
        printf("library minor version: %d \n",vpu_ver.lib_minor);
        printf("library release version: %d \n",vpu_ver.lib_release);

    }else{
        printf("Something Went Wrong! Error Code: %d \n",status);
    }

    printf("Main Exitting!\n");
    return 0;
}
#endif

#if 1
/*!
 * Temporary Application Entry 1
 * For vpu_GetVersionInfo
 */
int main(void)
{
    struct decode *dec;
    struct cmd_line cmdl = {0}; // :TODO Set the Structure before Decode Open
    vpu_mem_desc mem_desc = {0};
    vpu_mem_desc ps_mem_desc = {0};
    vpu_mem_desc slice_mem_desc = {0};
    RetCode ret;
    int eos = 0, fill_end_bs = 0, fillsize = 0;
    int status;
    int dummy_fd;// TODO: for waiting till the VFS Loads - Temporary Remove it 

    DecHandle handle = {0};
    DecOpenParam oparam = {0};

    dummy_fd = open("/sda/dummy",O_RDWR,0); // TODO: remove it. Just to invoke the VFS to open before other Open calls
    printf("Dummy fileFD: %d\n",dummy_fd);

    framebuf_init();
    printf("framebuf initialised!");
    ret = vpu_Init();
    if(ret != RETCODE_SUCCESS){
        err_msg("VPU INIT Failed! retured %d\n",ret);
        return ret;
    }

    dec = (struct decode *)calloc(1,sizeof(struct decode));
    if(dec == NULL){
        printf("Unable to allocate Memory for dec! \n");
        return -1;
    }
    printf("Cleared - dec structure\n");

    mem_desc.size = STREAM_BUF_SIZE;
    ret = get_dec_work_addr(&mem_desc); // TODO Not at all a good idea.. Check this with akshay
    if (ret == RETCODE_FAILURE) { // TODO Check virt memory is checked in this line
        err_msg("Unable to obtain Start Address mem\n");
        return RETCODE_FAILURE;
    }
    printf("Bitstream Phy Mem Starting Address: %x\n",mem_desc.phy_addr);
    printf("Bitstream Virt Mem Starting Address: %x\n",mem_desc.virt_uaddr);

    dec->phy_bsbuf_addr = mem_desc.phy_addr;
    dec->virt_bsbuf_addr = mem_desc.virt_uaddr;

    dec->reorderEnable = 0;
    dec->tiled2LinearEnable = 0;

    // :TODO Hard Coding Cmd Line Args
    cmdl.format = STD_AVC;
    cmdl.deblock_en = 1; // Enable
    cmdl.chromaInterleave = 0; // INterleaved
    cmdl.mp4_h264Class = 0; // MPEG-4
    cmdl.mapType = 0; // Linear Frame Map
    cmdl.bs_mode = 0;// Check
    cmdl.fps = 30;

    strcpy(cmdl.input,"/sda/dummy.h264");
    strcpy(cmdl.output,"/sda/output.yuv");// TODO check whats the output extention

    cmdl.src_fd = open(cmdl.input,O_RDONLY,0);
    if(cmdl.src_fd < 0){
        printf("Input File (BitStream file) is not able to open! - ERROR\n");
        return -1;
    }
    printf("Input File Opened - %s with FD: %d\n",cmdl.input,cmdl.src_fd);

    cmdl.dst_fd = open(cmdl.output,O_RDWR | O_CREAT, 0777);
    if(cmdl.dst_fd < 0){
        printf("Failed to Create OutPut File! - ERROR\n");
        return -1;
    }
    printf("Output File Created - %s with FD: %d\n",cmdl.output,cmdl.dst_fd);

    dec->cmdl = &cmdl;

    if (cmdl.format == STD_AVC) {
        ps_mem_desc.size = PS_SAVE_SIZE;
        ret = get_dec_work_addr(&ps_mem_desc); // TODO Not at all a good idea.. Check this with akshay
        if (ret) {
            err_msg("Unable to obtain Start Address ps save mem\n");
            return RETCODE_FAILURE;
        }
        printf("Physical Mem of ps_mem 0x%8x\n",ps_mem_desc.phy_addr);
        printf("Virtual Mem of ps_mem 0x%8x\n",ps_mem_desc.virt_uaddr);

        dec->phy_ps_buf = ps_mem_desc.phy_addr; 
    }

    if (dec->cmdl->mapType == LINEAR_FRAME_MAP)
        dec->tiled2LinearEnable = 0;
    else
        /* CbCr interleave must be enabled for tiled map */
        dec->cmdl->chromaInterleave = 1;

    oparam.bitstreamFormat = dec->cmdl->format;
    oparam.bitstreamBuffer = dec->phy_bsbuf_addr;
    oparam.bitstreamBufferSize = STREAM_BUF_SIZE;
    oparam.pBitStream = dec->virt_bsbuf_addr; // TODO CHECK giving waring
    oparam.reorderEnable = dec->reorderEnable;
    oparam.mp4DeblkEnable = dec->cmdl->deblock_en;
    oparam.chromaInterleave = dec->cmdl->chromaInterleave;
    oparam.mp4Class = dec->cmdl->mp4_h264Class;

    oparam.avcExtension = dec->cmdl->mp4_h264Class;
    oparam.mapType = dec->cmdl->mapType;
    oparam.tiled2LinearEnable = dec->tiled2LinearEnable;
    oparam.bitstreamMode = dec->cmdl->bs_mode;

    if (oparam.mp4DeblkEnable == 1) {
        dec->cmdl->deblock_en = 0;
    }

    oparam.psSaveBuffer = dec->phy_ps_buf;
    oparam.psSaveBufferSize = PS_SAVE_SIZE;

    ret = vpu_DecOpen(&handle, &oparam);
    if (ret != RETCODE_SUCCESS) {
        err_msg("vpu_DecOpen failed, ret:%d\n", ret);
        return -1;
    }

    dec->handle = handle;

    cmdl.complete = 1;

    status = dec_fill_bsbuffer(dec->handle, &cmdl,dec->virt_bsbuf_addr,(dec->virt_bsbuf_addr + STREAM_BUF_SIZE),dec->phy_bsbuf_addr, fillsize, &eos, &fill_end_bs);
    cmdl.complete = 0;
    if (fill_end_bs) {
        err_msg("Update 0 before seqinit, fill_end_bs = %d\n",fill_end_bs);
    }
    if (status < 0) {
        err_msg("dec_fill_bsbuffer failed\n");
        return -1;
    }

    /* parse the bitstream */
    status = decoder_parse(dec);
    if (status != 0) {
        err_msg("decoder parse failed\n");
        return -1;
    }

    /* allocate slice buf */
    slice_mem_desc.size = dec->phy_slicebuf_size;
    ret = get_dec_work_addr(&slice_mem_desc);
    if (ret) {
        err_msg("Unable to obtain physical slice save mem\n");
        return -1;
    }
    printf("Physical SliceBuf Mem: 0x%x\n",slice_mem_desc.phy_addr);
    printf("Virtual SliceBuf Mem: 0x%x\n",slice_mem_desc.virt_uaddr);
    dec->phy_slice_buf = slice_mem_desc.phy_addr;

    status = decoder_allocate_framebuffer(dec);
    if(status != 0){
        printf("decoder_allocate_framebuffer returned Error!\n");
        return -1;
    }

    /* start decoding */
    status = decoder_start(dec);
    if(status != 0){
        printf("Decoder_start Returned an Error!\n");
    }

    printf("END!!!\n");

    return 0;
}
#endif
