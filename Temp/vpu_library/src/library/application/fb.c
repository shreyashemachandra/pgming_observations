#include <INTEGRITY.h>
#include <string.h>

#include "vpu_test.h"
#include "../vpu_lib/vpu_lib.h"


#define NUM_FRAME_BUFS  32
#define FB_INDEX_MASK   (NUM_FRAME_BUFS - 1)

static int fb_index;
static struct frame_buf *fbarray[NUM_FRAME_BUFS];
static struct frame_buf fbpool[NUM_FRAME_BUFS];

void framebuf_init(void)
{   
      int i;
      
      for (i = 0; i < NUM_FRAME_BUFS; i++) {
          fbarray[i] = &fbpool[i];
      }
}

struct frame_buf *get_framebuf(void)
{
    struct frame_buf *fb;

    fb = fbarray[fb_index];
    fbarray[fb_index] = 0;

    ++fb_index;
    fb_index &= FB_INDEX_MASK;

    return fb;

}

struct frame_buf *framebuf_alloc(int stdMode, int format, int strideY, int height, int mvCol)
{
    struct frame_buf *fb;
    RetCode ret;
    int divX, divY;

    fb = get_framebuf();
    if (fb == NULL){
        printf("get_framebuf returned NULL\n");
        return NULL;
    }

    divX = (format == MODE420 || format == MODE422) ? 2 : 1;
    divY = (format == MODE420 || format == MODE224) ? 2 : 1; // TODO Check if this is needed

    memset(&(fb->desc), 0, sizeof(vpu_mem_desc));

    fb->desc.size = (strideY * height  + strideY / divX * height / divY * 2);
    if (mvCol)
        fb->desc.size += strideY / divX * height / divY;

    ret = get_dec_work_addr(&fb->desc);
    if (ret != RETCODE_SUCCESS) {
        printf("Frame buffer allocation failure\n");
        memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
        return NULL;
    }
    fb->addrY = fb->desc.phy_addr;
    fb->addrCb = fb->addrY + strideY * height;
    fb->addrCr = fb->addrCb + strideY / divX * height / divY;
    fb->strideY = strideY;
    fb->strideC =  strideY / divX;
    if (mvCol) 
        fb->mvColBuf = fb->addrCr + strideY / divX * height / divY;

    if (fb->desc.virt_uaddr <= 0) {
        printf("No Memory was given by the Lib\n");
        memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
        return NULL;
    }
    
    return fb;
}

void put_framebuf(struct frame_buf *fb)
{
    --fb_index;
    fb_index &= FB_INDEX_MASK;

    fbarray[fb_index] = fb;
}

void framebuf_free(struct frame_buf *fb)
{
    if (fb == NULL)
        return;

    if (fb->desc.virt_uaddr) {
       // IOFreeVirtMem(&fb->desc);// :TODO Check with akshay what to do
    }        

    if (fb->desc.phy_addr) {
       // IOFreePhyMem(&fb->desc);// :TODO Check with akshay, what to do
    }        

    memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
    put_framebuf(fb);
}

