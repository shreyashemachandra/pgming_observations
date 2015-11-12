/*!
 * @file vpu_io.h
 *
 * @brief VPU system ioctrl definition
 *
 * @ingroup VPU
 */

/*!
 * @brief  vpu memory description structure
 */
typedef struct vpu_mem_desc {
	int size;		/*!requested memory size */
	unsigned long phy_addr;	/*!physical memory address allocated */
	unsigned long cpu_addr;	/*!cpu addr for system free usage */
	unsigned long virt_uaddr;	/*!virtual user space address */
} vpu_mem_desc;

#ifndef	true
#define true	1
#endif
#ifndef	false
#define false	0
#endif

#define	VPU_IOC_MAGIC		'V'

#define	VPU_IOC_PHYMEM_ALLOC	_IO(VPU_IOC_MAGIC, 0)
#define	VPU_IOC_PHYMEM_FREE	_IO(VPU_IOC_MAGIC, 1)
#define VPU_IOC_WAIT4INT	_IO(VPU_IOC_MAGIC, 2)
#define	VPU_IOC_PHYMEM_DUMP	_IO(VPU_IOC_MAGIC, 3)
#define	VPU_IOC_REG_DUMP	_IO(VPU_IOC_MAGIC, 4)
#define	VPU_IOC_IRAM_BASE	_IO(VPU_IOC_MAGIC, 6)
#define	VPU_IOC_CLKGATE_SETTING	_IO(VPU_IOC_MAGIC, 7)
#define VPU_IOC_GET_WORK_ADDR   _IO(VPU_IOC_MAGIC, 8)
#define VPU_IOC_REQ_VSHARE_MEM  _IO(VPU_IOC_MAGIC, 9)
#define VPU_IOC_SYS_SW_RESET	_IO(VPU_IOC_MAGIC, 11)
#define VPU_IOC_GET_SHARE_MEM   _IO(VPU_IOC_MAGIC, 12)
#define VPU_IOC_QUERY_BITWORK_MEM  _IO(VPU_IOC_MAGIC, 13)
#define VPU_IOC_SET_BITWORK_MEM    _IO(VPU_IOC_MAGIC, 14)
#define VPU_IOC_PHYMEM_CHECK	_IO(VPU_IOC_MAGIC, 15)

Value VpuWriteReg(Address addr, Value data);
Value VpuReadReg(Address addr);
