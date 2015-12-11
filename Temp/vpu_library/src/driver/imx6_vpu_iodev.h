/*
 *              Copyright 2013 Green Hills Software                      
 *
 *    This program is the property of Green Hills Software, its contents
 *    are proprietary information and no part of it is to be disclosed to
 *    anyone except employees of Green Hills Software, or as agreed in
 *    writing signed by the President of Green Hills Software.
 */

/*! \file The public portion of the VPU IODevice.
 */

/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description         Author
 *  --------  -------   -----------         ------ 
 *  07/12/15    1       Copied Header from
 *                      Linux Drivers       Shreyas
 ******************************************************************************/ 

#ifndef IMX6_VPU_IODEV_H__
#define IMX6_VPU_IODEV_H__

#define VPU_IODEV_NAME		"VPUDev"
#define VPU_REGS_MR_NAME	"vpu-regs"
#define VPU_MEMORY_MR_NAME	"vpu-memory"

typedef enum {
    IODEV_VPU_PHYSICAL_BASE = 1
} iodev_vpu_regs;

typedef volatile uint32_t sfr32_t;

typedef struct vpu_regs_t_ {
    sfr32_t vpu_coderun;      /* 0x0204_0000 BIT Processor run start - W 0000_0000 */
    sfr32_t vpu_codedown;     /* 0x0204_0004 BIT Boot Code Download Data register - W 0000_0000 */
    sfr32_t vpu_hostintreq;   /* 0x0204_0008 Host Interrupt Request to BIT - W 0000_0000 */
    sfr32_t vpu_bitintclear;  /* 0x0204_000C BIT Interrupt Clear - W 0000_0000 */
    sfr32_t vpu_bitintsts;    /* 0x0204_0010 BIT Interrupt Status - R 0000_0000 */
    sfr32_t reserved1;        /* 0x0204_0014 */
    sfr32_t vpu_bitcurpc;     /* 0x0204_0018 BIT Current PC - R 0000_0000 */
    sfr32_t reserved2;        /* 0x0204_001C */
    sfr32_t vpu_bitcodecbusy; /* 0x0204_0020 BIT CODEC Busy - R 0000_0000 */
} vpu_regs_t;

// From Linux - mxc_vpu.h
#define BIT_CODE_RUN            0x000
#define BIT_CODE_DOWN           0x004
#define BIT_INT_CLEAR           0x00C
#define BIT_INT_STATUS          0x010
#define BIT_CUR_PC          0x018
#define BIT_INT_REASON          0x174

#define MJPEG_PIC_STATUS_REG        0x3004
#define MBC_SET_SUBBLK_EN       0x4A0

#define BIT_WORK_CTRL_BUF_BASE      0x100
#define BIT_WORK_CTRL_BUF_REG(i)    (BIT_WORK_CTRL_BUF_BASE + i * 4)
#define BIT_CODE_BUF_ADDR       BIT_WORK_CTRL_BUF_REG(0)
#define BIT_WORK_BUF_ADDR       BIT_WORK_CTRL_BUF_REG(1)
#define BIT_PARA_BUF_ADDR       BIT_WORK_CTRL_BUF_REG(2)
#define BIT_BIT_STREAM_CTRL     BIT_WORK_CTRL_BUF_REG(3)
#define BIT_FRAME_MEM_CTRL      BIT_WORK_CTRL_BUF_REG(4)
#define BIT_BIT_STREAM_PARAM        BIT_WORK_CTRL_BUF_REG(5)

#ifndef CONFIG_SOC_IMX6Q
#define BIT_RESET_CTRL          0x11C
#else
#define BIT_RESET_CTRL          0x128
#endif

/* i could be 0, 1, 2, 3 */
#define BIT_RD_PTR_BASE         0x120
#define BIT_RD_PTR_REG(i)       (BIT_RD_PTR_BASE + i * 8)
#define BIT_WR_PTR_REG(i)       (BIT_RD_PTR_BASE + i * 8 + 4)

/* i could be 0, 1, 2, 3 */
#define BIT_FRM_DIS_FLG_BASE        (cpu_is_mx51() ? 0x150 : 0x140)
#define BIT_FRM_DIS_FLG_REG(i)      (BIT_FRM_DIS_FLG_BASE + i * 4)

#define BIT_BUSY_FLAG           0x160
#define BIT_RUN_COMMAND         0x164
#define BIT_INT_ENABLE          0x170

#define BITVAL_PIC_RUN          8

#define VPU_SLEEP_REG_VALUE     10
#define VPU_WAKE_REG_VALUE      11

#endif /* IMX6_VPU_IODEV_H__ */
