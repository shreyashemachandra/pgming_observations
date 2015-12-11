/*
 *              Copyright 2013 Green Hills Software                      
 *
 *    This program is the property of Green Hills Software, its contents
 *    are proprietary information and no part of it is to be disclosed to
 *    anyone except employees of Green Hills Software, or as agreed in
 *    writing signed by the President of Green Hills Software.
 */

/*! \file BSP support for Video Processing Unit (VPU) */

#include <bsp.h>
#include <INTEGRITY.h>
#include <bsp_export.h>
#include "support/buildmemtable.h"
#include "driver/soc/imx61/ccm.h"
#include "interruptcontroller.h"
#include "driver/video/imx6_vpu_iodev.h"
#include "support/memoryspace.h"

#pragma weak __VPUMemorySize 
extern char __VPUMemorySize[];

/* VPU register area, 0x0204_0000 - 0x0207_0bfff  240KB
*/
static const MemoryReservation vpuregs = {
    MEMORY_RW | MEMORY_VOLATILE | MEMORY_ARM_STRONGLY_ORDERED, 0, IMX6_VPU_BASE, IMX6_VPU_LAST, Other_MemoryType,
    true, 0xfffff000, 0xfffff000, VPU_REGS_MR_NAME
};

/*!
 * struct VPUDevInfo_t
 *    VPU IODevice info
 */
typedef struct {
    const char *name;
    vpu_regs_t *regs;
    VECT        vect;
    MemorySpaceAddress physical_vpumemory;
    struct IODeviceVectorStruct	iodev; /*! INTEGRITY IODevice */
} VPUDevInfo_t;

static VPUDevInfo_t VPUDev = {
    .name = VPU_IODEV_NAME,
    .regs = (vpu_regs_t *)IMX6_VPU_BASE,
    .vect = VECT_VPU
};

/*!
 * \brief VPU exception handler.
 * \param[in] xf
 *    The exception frame (XFRAME) of the current exception. The context of 
 *    the CPU (ie the registers) at the moment of the exception is stored
 *    in the exception frame.
 * \param[in] id
 *    Exception identification value set to the IODevice
 * \retval EVENT_HANDLED
 *    The VPU exception has been handled.
 */
static EVENT vpu_ExceptionHandler(XFRAME *xf, Address id)
{
    VPUDevInfo_t *dev = (VPUDevInfo_t *)id;

    if ( dev->regs->vpu_bitintsts) {
        dev->regs->vpu_bitintclear = 1;
        INTERRUPT_IODeviceNotify(&dev->iodev);
        return EVENT_HANDLED;
    } else {
        return EVENT_UNKNOWN;
    }
}

/*!
 * \brief create the VPU IODevice
 * \param iodev reference to the VPU IODevice
 * \returns Success
 */
static Error vpu_Create( IODeviceVector iodev )
{
    return Success;
}

/*!
 * \brief Read register from the VPU IO device.
 * \param iodev reference to the VPU IO device
 * \param RegisterNumber register to read from
 * \param Value reference to receive value from the given register
 * \return Error
 *     Success only if the RegisterNumber signifies a valid register and the
 *     corresponding read succeeded.
 */
static Error vpu_ReadRegister(IODeviceVector iodev,
        Value RegisterNumber, Value *value)
{
    VPUDevInfo_t *dev = iodev->DeviceSpecificInfo;

    Error err = Success;
    switch ( RegisterNumber ) {
        case IODEV_VPU_PHYSICAL_BASE:
            *value = dev->physical_vpumemory;
            break;
        default:
            err = IllegalRegisterNumber;
    }
    return err;
}

/*!
 * \brief Initialize the VPU IODevice and Memory mappings
 */
static void vpu_init( void )
{
    ExtendedAddress addr;
    CacheCoherencyMode CoherencyMode;
    IODeviceVector iodev = &VPUDev.iodev;

    if ( !__VPUMemorySize ) {
        /* No memory defined for the VPU, so not being used */
        return;
    }

    iodev->DeviceSpecificInfo = &VPUDev;
    iodev->Create        = vpu_Create;
    iodev->ReadBlock     = NULL;
    iodev->WriteBlock    = NULL;
    iodev->ReadBuffers   = NULL;
    iodev->WriteBuffers  = NULL;
    iodev->ReadRegister  = vpu_ReadRegister;
    iodev->WriteRegister = NULL;
    iodev->ReadStatus    = NULL;
    iodev->WriteStatus   = NULL;
    iodev->Reset         = NULL;
    iodev->IOCoherentNotRequired        = 1;
    iodev->IOSynchronizationNotRequired = 1;
    CheckSuccess(RegisterIODeviceVector(&VPUDev.iodev, VPUDev.name));

    /* Allocate VPU memory */
    CheckSuccess(MemorySpace_AllocateDmaMemory(&PhysicalMemorySpace,
                (ExtendedAddress)__VPUMemorySize, ASP_PAGESIZE,
                MEMORY_READ | MEMORY_WRITE, HardwareCacheCoherency,
                VPU_MEMORY_MR_NAME, &VPUDev.physical_vpumemory, NULL,
                &CoherencyMode));

    /* Allocate VPU register area */
    CheckSuccess(BMT_AllocateFromAnonymousMemoryReservation(&vpuregs, &addr));

    /* Install the VPU Exception handler */
    SetExceptionHandler(VPUDev.vect, vpu_ExceptionHandler, (Address)&VPUDev,
            "VPU Exception Handler" );
    EnableVector(VPUDev.vect);
}

void (*__ghsentry_bspuserinit_vpu)(void) = &vpu_init;
