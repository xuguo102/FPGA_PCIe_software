// (c) Copyright 2009  2023 AMD, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of AMD, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// AMD, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) AMD shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or AMD had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// AMD products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of AMD products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.


//--------------------------------------------------------------------------------
//-- Filename: amd_pcie_exerciser.c
//--
//-- Description: amd pcie exerciser device driver.
//--
//-- amd pcie exerciser is an example driver which exercises xilinx BMD design
//--------------------------------------------------------------------------------

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
//#include <linux/pci-aspm.h>
//#include <linux/pci_regs.h>
#include <linux/aer.h>
#include <linux/irq.h>
#include <linux/proc_ns.h>
#include <linux/uaccess.h>   /* copy_to_user */
#include "internal.h"

#include "amd_pcie_exerciser.h"

static struct fasync_struct *fasync;

static struct class* class;
static int major;

int irq_num = 0;
struct proc_dir_entry *de = NULL;
char *aerdrv_name = "aerdrv";
char *exerciser_name = "AMD_PCIe_Exerciser";

struct irq_chip *irq_chip_info;
struct irq_desc *irq_desc_info;
struct proc_dir_entry *proc_dir_entry_tmp;

// semaphores
enum  {
    SEM_READ,
    SEM_WRITE,
    SEM_WRITEREG,
    SEM_READREG,
    SEM_WAITFOR,
    SEM_DMA,
    NUM_SEMS
};

//semaphores
struct semaphore gSem[NUM_SEMS];

// Defines the Vendor ID.  Must be changed if core generated did not set the Vendor ID to the same value
#define PCI_EXERCISER_VENDOR_ID_AMD      0x10ee

// Defines the Device ID.  Must be changed if core generated did not set the Device ID to the same value
#define PCI_EXERCISER_DEVICE_ID_AMD_PCIE 0xb03f

// Defining
#define PBE_REGISTER_SIZE        (4*8)    // There are eight registers, and each is 4 bytes wide.
#define HAVE_REGION               0x01     // I/O Memory region
#define HAVE_IRQ                  0x02     // Interupt

//Status Flags:
//       1 = Resouce successfully acquired
//       0 = Resource not acquired.

#define HAVE_REGION 0x01                    // I/O Memory region
#define HAVE_IRQ    0x02                    // Interupt
#define HAVE_KREG   0x04                    // Kernel registration

typedef struct pbe_device
{
    const char *name;

    unsigned long baseAddr;
    unsigned long baseLength;
    void __iomem *baseVirtual;

    struct pci_dev *pciDev;
    int             irq;

    dma_addr_t    readAddr;
    u8           *readBuffer;
    dma_addr_t    writeAddr;
    u8           *writeBuffer;

    struct cdev   chrDev;
} pbe_device;

typedef union RegWrite {
    uint64_t raw;
    struct {
        int reg;
        int value;
    };
} RegWrite;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

irqreturn_t XPCIe_IRQHandler(int irq, void *dev_id);
irqreturn_t XPCIe_IRQMSIHandler(int irq, void *dev_id);
uint32_t   XPCIe_ReadReg (pbe_device *dev, uint32_t dw_offset);
void  XPCIe_WriteReg (pbe_device *dev, uint32_t dw_offset, uint32_t val);
void  XPCIe_InitCard (pbe_device *dev);
void  XPCIe_InitiatorReset (pbe_device *dev);
uint32_t XPCIe_ReadCfgReg (pbe_device *dev, uint32_t byte);
uint32_t XPCIe_WriteCfgReg (pbe_device *dev, uint32_t byte, uint32_t value);

static int exerciser_fasync(int fd, struct file *filp, int on)
{
	return fasync_helper(fd, filp, on, &fasync);
}

static int proc_match(const char *name, struct proc_dir_entry *de, unsigned int len)
{
	if (len < de->namelen)
		return -1;
	if (len > de->namelen)
		return 1;

	return memcmp(name, de->name, len);
}

static struct proc_dir_entry *pde_subdir_find(struct proc_dir_entry *dir,
					      const char *name,
					      unsigned int len)
{
	struct rb_node *node = dir->subdir.rb_node;

	while (node) {
		struct proc_dir_entry *de = rb_entry(node,
						     struct proc_dir_entry,
						     subdir_node);
		int result = proc_match(name, de, len);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return de;
	}
	return NULL;
}

//--- XPCIe_InitiatorReset(): Resets the PBE reference design
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: Writes a 1 to the DCSR register which resets the PBE design
void XPCIe_InitiatorReset(pbe_device *dev)
{
    XPCIe_WriteReg(dev, Reg_DeviceCS, 1);
    // Write: DCSR (offset 0) with value of 1 (Reset Device)
    XPCIe_WriteReg(dev, Reg_DeviceCS, 0);
    // Write: DCSR (offset 0) with value of 0 (Make Active)
}


//--- XPCIe_InitCard(): Initializes PBE descriptor registers to default values
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: 1) Resets device
//---                       2) Writes specific values into the PBE registers inside the EP
void XPCIe_InitCard(pbe_device *dev)
{
    uint32_t RegValue;
    XPCIe_InitiatorReset(dev);

    XPCIe_WriteReg(dev, Reg_WriteTlpAddress, dev->writeAddr);        // Write: Write DMA TLP Address register with starting address
    RegValue = XPCIe_ReadReg(dev, Reg_WriteTlpAddress);
    printk("*************** Reg_WriteTlpAddress : 0x%08x ****************\n", RegValue);
    XPCIe_WriteReg(dev, Reg_WriteTlpSize, 0x20);                // Write: Write DMA TLP Size register with default value (32dwords)
    RegValue = XPCIe_ReadReg(dev, Reg_WriteTlpSize);
    printk("*************** Reg_WriteTlpSize : 0x%08x ****************\n", RegValue);
    XPCIe_WriteReg(dev, Reg_WriteTlpCount, 0x2000);              // Write: Write DMA TLP Count register with default value (2000)
    XPCIe_WriteReg(dev, Reg_WriteTlpPattern, 0x00000000);          // Write: Write DMA TLP Pattern register with default value (0x0)

    XPCIe_WriteReg(dev, Reg_ReadTlpPattern, 0xfeedbeef);          // Write: Read DMA Expected Data Pattern with default value (feedbeef)
    XPCIe_WriteReg(dev, Reg_ReadTlpAddress, dev->readAddr);         // Write: Read DMA TLP Address register with starting address.
    XPCIe_WriteReg(dev, Reg_ReadTlpSize, 0x20);                // Write: Read DMA TLP Size register with default value (32dwords)
    XPCIe_WriteReg(dev, Reg_ReadTlpCount, 0x2000);              // Write: Read DMA TLP Count register with default value (2000)
}

static const struct pci_device_id XPCIe_ids[] =
{
    {
        PCI_EXERCISER_VENDOR_ID_AMD, PCI_EXERCISER_DEVICE_ID_AMD_PCIE,
        PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
    },
    { 0 }
};

irqreturn_t XPCIe_IRQHandler(int irq, void *dev_id)
{
    /*uint32_t i, regx, msiReg;*/
    pbe_device *dev = dev_id;

    printk(KERN_WARNING"%s: AER Interrupt Coming ...",dev->name);

#if 0
    msiReg = XPCIe_ReadReg(dev, Reg_DeviceMSIControl);
    if (msiReg & 0x80000000) {
        // NOTE(michiel): Our interrupt
        printk(KERN_WARNING"%s: Interrupt Handler Start ..",dev->name);

        for (i = 0; i < 32; i++) {
            regx = XPCIe_ReadReg(dev, i);
            printk(KERN_WARNING"%s: REG<%d> : 0x%X\n", dev->name, i, regx);
        }

        XPCIe_WriteReg(dev, Reg_DeviceMSIControl, 0x100 | (msiReg & 0xFF));
        XPCIe_ReadReg(dev, Reg_DeviceMSIControl);
        XPCIe_WriteReg(dev, Reg_DeviceMSIControl, (msiReg & 0xFF));

        printk(KERN_WARNING"%s: Interrupt Handler End ..\n", dev->name);

        // send SIGIO signal to the device in async_queue
        kill_fasync(&fasync, SIGIO, POLL_IN);
        return IRQ_HANDLED;
    } else {
        // send SIGIO signal to the device in async_queue
        kill_fasync(&fasync, SIGIO, POLL_IN);
        return IRQ_NONE;
    }
#endif
    // send SIGIO signal to the device in async_queue
    kill_fasync(&fasync, SIGIO, POLL_IN);
    printk(KERN_WARNING"%s: AER Interrupt End ...",dev->name);
    return IRQ_HANDLED;
}

irqreturn_t XPCIe_IRQMSIHandler(int irq, void *dev_id)
{
    uint32_t i, regx;
    pbe_device *dev = dev_id;

    printk("%s: Interrupt Handler Start ..",dev->name);

    for (i = 0; i < 32; i++) {
        regx = XPCIe_ReadReg(dev, i);
        printk("%s : REG<%d> : 0x%X\n", dev->name, i, regx);
    }

    printk("%s: Interrupt Handler End ..\n", dev->name);

    return IRQ_HANDLED;
}

uint32_t XPCIe_ReadReg(pbe_device *dev, uint32_t dw_offset)
{
    uint32_t ret = 0;
    ret = readl(dev->baseVirtual + (4 * dw_offset));

    return ret;
}

void XPCIe_WriteReg(pbe_device *dev, uint32_t dw_offset, uint32_t val)
{
    writel(val, (dev->baseVirtual + (4 * dw_offset)));
}

#if 0
int XPCIe_ReadMem(char *buf, size_t count)
{
    int ret = 0;
    dma_addr_t dma_addr;

    //make sure passed in buffer is large enough
    if ( count < DMA_BUF_SIZE ) {
        printk("%s: XPCIe_Read: passed in buffer too small.\n", dev->name);
        ret = -1;
        goto exit;
    }

    down(&gSem[SEM_DMA]);

    // pci_map_single return the physical address corresponding to
    // the virtual address passed to it as the 2nd parameter

    dma_addr = pci_map_single(gDev, gReadBuffer, DMA_BUF_SIZE, PCI_DMA_FROMDEVICE);
    if ( 0 == dma_addr )  {
        printk("%s: XPCIe_Read: Map error.\n",dev->name);
        ret = -1;
        goto exit;
    }

    // Now pass the physical address to the device hardware. This is now
    // the destination physical address for the DMA and hence the to be
    // put on Memory Transactions

    // Do DMA transfer here....

    printk("%s: XPCIe_Read: ReadBuf Virt Addr = %lX Phy Addr = %X.\n",
            dev->name, (unsigned long)gReadBuffer, (unsigned int)dma_addr);

    // Unmap the DMA buffer so it is safe for normal access again.
    pci_unmap_single(gDev, dma_addr, DMA_BUF_SIZE, PCI_DMA_FROMDEVICE);

    up(&gSem[SEM_DMA]);

    // Now it is safe to copy the data to user space.
    if ( copy_to_user(buf, gReadBuffer, DMA_BUF_SIZE) )  {
        ret = -1;
        printk("%s: XPCIe_Read: Failed copy to user.\n",dev->name);
        goto exit;
    }
exit:
    return ret;
}

ssize_t XPCIe_WriteMem(const char *buf, size_t count) {
    int ret = 0;
    dma_addr_t dma_addr;

    if ( (count % 4) != 0 )  {
        printk("%s: XPCIe_Write: Buffer length not dword aligned.\n",dev->name);
        ret = -1;
        goto exit;
    }

    // Now it is safe to copy the data from user space.
    if ( copy_from_user(gWriteBuffer, buf, count) )  {
        ret = -1;
        printk("%s: XPCIe_Write: Failed copy to user.\n",dev->name);
        goto exit;
    }

    //set DMA semaphore if in loopback
    down(&gSem[SEM_DMA]);

    // pci_map_single return the physical address corresponding to
    // the virtual address passed to it as the 2nd parameter

    dma_addr = pci_map_single(gDev, gWriteBuffer, DMA_BUF_SIZE, PCI_DMA_FROMDEVICE);
    if ( 0 == dma_addr )  {
        printk("%s: XPCIe_Write: Map error.\n",dev->name);
        ret = -1;
        goto exit;
    }

    // Now pass the physical address to the device hardware. This is now
    // the source physical address for the DMA and hence the to be
    // put on Memory Transactions

    // Do DMA transfer here....

    printk("%s: XPCIe_Write: WriteBuf Virt Addr = %lXx Phy Addr = %X.\n",
            dev->name, (unsigned long)gReadBuffer, (unsigned int)dma_addr);

    // Unmap the DMA buffer so it is safe for normal access again.
    pci_unmap_single(gDev, dma_addr, DMA_BUF_SIZE, PCI_DMA_FROMDEVICE);

    up(&gSem[SEM_DMA]);

exit:
    return (ret);
}
#endif

uint32_t XPCIe_ReadCfgReg(pbe_device *dev, uint32_t byte) {
    uint32_t pciReg;
    if (pci_read_config_dword(dev->pciDev, byte, &pciReg) < 0) {
        printk("%s: XPCIe_ReadCfgReg: Reading PCI interface failed.",dev->name);
        return (-1);
    }
    return (pciReg);
}

uint32_t XPCIe_WriteCfgReg(pbe_device *dev, uint32_t byte, uint32_t val) {
    if (pci_write_config_dword(dev->pciDev, byte, val) < 0) {
        printk("%s: XPCIe_WriteCfgReg: Writing PCI interface failed.",dev->name);
        return (-1);
    }
    return 1;
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Open
//
// Description: Book keeping routine invoked each time the device is opened.
//
// Arguments: inode :
//            filp  :
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
int XPCIe_Open(struct inode *inode, struct file *filp)
{
    pbe_device *dev = container_of(inode->i_cdev, pbe_device, chrDev);
    filp->private_data = dev;

    if (!dev) {
        printk(KERN_WARNING"PBE: Open: No device associated with this file.\n");
        return -EBUSY;
    }

    if (!dev->baseVirtual || !dev->readBuffer || !dev->writeBuffer) {
        printk("%s: dev->baseVirtual %lX\n", dev->name,(unsigned long)dev->baseVirtual);
        printk("%s: dev->readBuffer  %lX\n", dev->name,(unsigned long)dev->readBuffer );
        printk("%s: dev->writeBuffer %lX\n", dev->name,(unsigned long)dev->writeBuffer);
        return -ENOMEM;
    }

    // TODO(michiel): Check for 0 dev
    printk(KERN_INFO"%s: Open: module opened\n", dev->name);
    return SUCCESS;
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Release
//
// Description: Book keeping routine invoked each time the device is closed.
//
// Arguments: inode :
//            filp  :
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
int XPCIe_Release(struct inode *inode, struct file *filp)
{
    pbe_device *dev = filp->private_data;
    fasync_helper(-1, filp, 0, &fasync);
    printk(KERN_INFO"%s: Release: module released\n", dev->name);
    return(SUCCESS);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Write
//
// Description: This routine is invoked from user space to write data to
//              the PCIe device.
//
// Arguments: filp  : file pointer to opened device.
//            buf   : pointer to location in users space, where data is to
//                    be acquired.
//            count : Amount of data in bytes user wishes to send.
//
// Returns: SUCCESS  = Success
//          CRIT_ERR = Critical failure
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
ssize_t XPCIe_Write(struct file *filp, const char __user *buf, size_t count,
        loff_t *f_pos)
{
    pbe_device *dev = filp->private_data;
    int ret = SUCCESS;

    if (copy_from_user(dev->writeBuffer, buf, count)) {
        printk("%s: XPCIe_Write: Failed copy from user.\n", dev->name);
        return -EFAULT;
    }
    printk(KERN_INFO"%s: XPCIe_Write: %ld bytes have been written...\n", dev->name, count);

    if (copy_from_user(dev->readBuffer, buf, count)) {
        printk("%s: XPCIe_Write: Failed copy from user.\n", dev->name);
        return -EFAULT;
    }
    printk(KERN_INFO"%s: XPCIe_Write: %ld bytes have been read...\n", dev->name, count);

    return (ret);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Read
//
// Description: This routine is invoked from user space to read data from
//              the PCIe device. ***NOTE: This routine returns the entire
//              buffer, (DMA_BUF_SIZE), count is ignored!. The user App must
//              do any needed processing on the buffer.
//
// Arguments: filp  : file pointer to opened device.
//            buf   : pointer to location in users space, where data is to
//                    be placed.
//            count : Amount of data in bytes user wishes to read.
//
// Returns: SUCCESS  = Success
//          CRIT_ERR = Critical failure
//
//  Modification log:
//  Date      Who  Description
//----------------------------------------------------------------------------

ssize_t XPCIe_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    pbe_device *dev = filp->private_data;

    if (copy_to_user(buf, dev->writeBuffer, count)) {
        printk("%s: XPCIe_Read: Failed copy to user.\n", dev->name);
        return -EFAULT;
    }
    printk(KERN_INFO"%s: XPCIe_Read: %ld bytes have been read...\n", dev->name, count);

    return (0);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Ioctl
//
// Description: This routine is invoked from user space to configure the
//              running driver.
//
// Arguments: filp  : File pointer to opened device.
//            cmd   : Ioctl command to execute.
//            arg   : Argument to Ioctl command.
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
#define IOCTL_READ_REG(x)     { regx = XPCIe_ReadReg(dev, x); ret = put_user(regx, (uint32_t *)arg); }
#define IOCTL_WRITE_REG(x)    { XPCIe_WriteReg(dev, x, arg); }
long XPCIe_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    pbe_device *dev = filp->private_data;
    uint32_t regx;
    int ret = SUCCESS;

    switch (cmd) {
        // Initailizes PBE application
        case PBE_IOC_INITCARD:
            {
                XPCIe_InitCard(dev);
            }
            break;

        // Resets PBE applications
        case PBE_IOC_RESET:
            {
                XPCIe_InitiatorReset(dev);
            }
            break;

        case PBE_IOC_DISP_REGS:
            {
                printk("*************** no Register for it.****************\n");
            }
            break;

        // Read: Device Control Status Register
        case PBE_IOC_READ_CTRL:      IOCTL_READ_REG(Reg_DeviceCS) break;

        // Read: DMA Control Status Register
        case PBE_IOC_READ_DMA_CTRL:  IOCTL_READ_REG(Reg_DeviceDMACS) break;

        // Read: Write DMA TLP Address Register
        case PBE_IOC_READ_WR_ADDR:   IOCTL_READ_REG(Reg_WriteTlpAddress) break;

        // Read: Write DMA TLP Size Register
        case PBE_IOC_READ_WR_LEN:    IOCTL_READ_REG(Reg_WriteTlpSize) break;

        // Read: Write DMA TLP Count Register
        case PBE_IOC_READ_WR_COUNT:  IOCTL_READ_REG(Reg_WriteTlpCount) break;

        // Read: Write DMA TLP Pattern Register
        case PBE_IOC_READ_WR_PTRN:   IOCTL_READ_REG(Reg_WriteTlpPattern) break;

        // Read: Read DMA TLP Pattern Register
        case PBE_IOC_READ_RD_PTRN:   IOCTL_READ_REG(Reg_ReadTlpPattern) break;

        // Read: Read DMA TLP Address Register
        case PBE_IOC_READ_RD_ADDR:   IOCTL_READ_REG(Reg_ReadTlpAddress) break;

        // Read: Read DMA TLP Size Register
        case PBE_IOC_READ_RD_LEN:    IOCTL_READ_REG(Reg_ReadTlpSize) break;

        // Read: Read DMA TLP Count Register
        case PBE_IOC_READ_RD_COUNT:  IOCTL_READ_REG(Reg_ReadTlpCount) break;

        // Read: Write DMA Performance Register
        case PBE_IOC_READ_WR_PERF:   IOCTL_READ_REG(Reg_WriteDMAPerf) break;

        // Read: Read DMA Performance Register
        case PBE_IOC_READ_RD_PERF:   IOCTL_READ_REG(Reg_ReadDMAPerf) break;

        // Read: Read DMA Status Register
        case PBE_IOC_READ_CMPL:      IOCTL_READ_REG(Reg_ReadComplStatus) break;

        // Read: Number of Read Completion w/ Data Register
        case PBE_IOC_READ_CWDATA:    IOCTL_READ_REG(Reg_ComplWithData) break;

        // Read: Read Completion Size Register
        case PBE_IOC_READ_CSIZE:     IOCTL_READ_REG(Reg_ComplSize) break;

        // Read: Device Link Width Status Register
        case PBE_IOC_READ_LINKWDTH:  IOCTL_READ_REG(Reg_DeviceLinkWidth) break;

        // Read: Device Link Transaction Size Status Register
        case PBE_IOC_READ_LINKLEN:   IOCTL_READ_REG(Reg_DeviceLinkTlpSize) break;

        // Read: Device Miscellaneous Control Register
        case PBE_IOC_READ_MISC_CTL:  IOCTL_READ_REG(Reg_DeviceMiscControl) break;

        // Read: Device MSI Control
        case PBE_IOC_READ_INTRPT:    IOCTL_READ_REG(Reg_DeviceMSIControl) break;

        // Read: Device Directed Link Change Register
        case PBE_IOC_READ_DIR_LINK:  IOCTL_READ_REG(Reg_DeviceDirectedLinkChange) break;

        // Read: Device FC Control Register
        case PBE_IOC_READ_FC_CTRL:   IOCTL_READ_REG(Reg_DeviceFCControl) break;

        // Read: Device FC Posted Information
        case PBE_IOC_READ_FC_POST:   IOCTL_READ_REG(Reg_DeviceFCPostedInfo) break;

        // Read: Device FC Non Posted Information
        case PBE_IOC_READ_FC_NPOST:  IOCTL_READ_REG(Reg_DeviceFCNonPostedInfo) break;

        // Read: Device FC Completion Information
        case PBE_IOC_READ_FC_CMPL:   IOCTL_READ_REG(Reg_DeviceFCCompletionInfo) break;

        // Write: DMA Control Status Register
        case PBE_IOC_WRITE_DMA_CTRL: IOCTL_WRITE_REG(Reg_DeviceDMACS) break;

        // Write: Write DMA TLP Size Register
        case PBE_IOC_WRITE_WR_LEN:   IOCTL_WRITE_REG(Reg_WriteTlpSize) break;

        // Write: Write DMA TLP Count Register
        case PBE_IOC_WRITE_WR_COUNT: IOCTL_WRITE_REG(Reg_WriteTlpCount) break;

        // Write: Write DMA TLP Pattern Register
        case PBE_IOC_WRITE_WR_PTRN:  IOCTL_WRITE_REG(Reg_WriteTlpPattern) break;

        // Write: Read DMA TLP Size Register
        case PBE_IOC_WRITE_RD_LEN:   IOCTL_WRITE_REG(Reg_ReadTlpSize) break;

        // Write: Read DMA TLP Count Register
        case PBE_IOC_WRITE_RD_COUNT: IOCTL_WRITE_REG(Reg_ReadTlpCount) break;

        // Write: Read DMA TLP Pattern Register
        case PBE_IOC_WRITE_RD_PTRN:  IOCTL_WRITE_REG(Reg_ReadTlpPattern) break;

        // Write: Device Miscellaneous Control Register
        case PBE_IOC_WRITE_MISC_CTL: IOCTL_WRITE_REG(Reg_DeviceMiscControl) break;


        // Write: PCIe AER control Register
        case PBE_IOC_WRITE_AER_CNTL: IOCTL_WRITE_REG(Reg_AerControl) break;
        // Read: PCIe AER control Register
        case PBE_IOC_READ_AER_CNTL: IOCTL_READ_REG(Reg_AerControl) break;

        // Write: PCIe error Register
        case PBE_IOC_WRITE_ERRORINJECT: IOCTL_WRITE_REG(Reg_ErrorInjectControl) break;
        // Read: PCIe error Register
        case PBE_IOC_READ_ERRORINJECT: IOCTL_READ_REG(Reg_ErrorInjectControl) break;

        // Write: PCIe AER Cor error Register
        case PBE_IOC_WRITE_COR_ERRORINJECT: IOCTL_WRITE_REG(Reg_AerCorErrorInjectControl) break;
        // Read: PCIe AER Cor error Register
        case PBE_IOC_READ_COR_ERRORINJECT: IOCTL_READ_REG(Reg_AerCorErrorInjectControl) break;

        // Write: PCIe AER UNCor error Register
        case PBE_IOC_WRITE_UNCOR_ERRORINJECT: IOCTL_WRITE_REG(Reg_AerUNCorErrorInjectControl) break;
        // Read: PCIe AER UNCor error Register
        case PBE_IOC_READ_UNCOR_ERRORINJECT: IOCTL_READ_REG(Reg_AerUNCorErrorInjectControl) break;


        // Write: Device Directed Link Change Register
        case PBE_IOC_WRITE_DIR_LINK: IOCTL_WRITE_REG(Reg_DeviceDirectedLinkChange) break;

        // Read: Any PBE Reg.  Added generic functionality so all register can be read
        case PBE_IOC_RD_BMD_REG:
            {
                if (get_user(regx, (uint32_t *)arg) < 0) {
                    ret = -EFAULT;
                } else {
                    regx = XPCIe_ReadReg(dev, regx);
                    ret = put_user(regx, (uint32_t *)arg);
                }
            }
            break;

        // Read: Any CFG Reg.  Added generic functionality so all register can be read
        case PBE_IOC_RD_CFG_REG:
            {
                if (get_user(regx, (uint32_t *)arg) < 0) {
                    ret = -EFAULT;
                } else {
                    regx = XPCIe_ReadCfgReg(dev, regx);
                    ret = put_user(regx, (uint32_t *)arg);
                }
            }
            break;

        // Write: Any BMD Reg.  Added generic functionality so all register can be read
        case PBE_IOC_WR_BMD_REG:
            {
                RegWrite bmdArg = {0};
                if (get_user(bmdArg.raw, (uint64_t *)arg) < 0) {
                    ret = -EFAULT;
                } else {
                    XPCIe_WriteReg(dev, bmdArg.reg, bmdArg.value);
                    printk(KERN_WARNING"%d: Write Register.\n", bmdArg.reg);
                    printk(KERN_WARNING"%d: Write Value.\n", bmdArg.value);
                }
            }
            break;

        // Write: Any CFG Reg.  Added generic functionality so all register can be read
        case PBE_IOC_WR_CFG_REG:
            {
                RegWrite cfgArg = {0};
                if (get_user(cfgArg.raw, (uint64_t *)arg) < 0) {
                    ret = -EFAULT;
                } else {
                    XPCIe_WriteCfgReg(dev, cfgArg.reg, cfgArg.value);
                    printk(KERN_WARNING"%d: Write Register.\n", cfgArg.reg);
                    printk(KERN_WARNING"%d: Write Value.\n", cfgArg.value);
                }
            }
            break;

        default: {} break;
    }

    return ret;
}
#undef IOCTL_WRITE_REG
#undef IOCTL_READ_REG

//--- XPCIe_exit(): Performs any cleanup required before releasing the device
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: Performs all cleanup functions required before releasing device
static void XPCIe_Exit(pbe_device *dev)
{
    if (!dev) {
        return;
    }

    if (dev->writeBuffer) {

        dma_free_coherent(&dev->pciDev->dev, DMA_BUF_SIZE, dev->writeBuffer, dev->writeAddr);
        dev->writeBuffer = 0;
        dev->writeAddr = 0;
    }

    if (dev->readBuffer) {
        dma_free_coherent(&dev->pciDev->dev, DMA_BUF_SIZE, dev->readBuffer, dev->readAddr);
        dev->readBuffer = 0;
        dev->readAddr = 0;
    }

    for (irq_num = 1; irq_num < 888 ; irq_num++) {

        if (!irq_get_irq_data(irq_num))
            continue;

        irq_desc_info = irq_data_to_desc(irq_get_irq_data(irq_num));
        if (!irq_desc_info)
            continue;

        proc_dir_entry_tmp = irq_desc_info->dir;
        if (!proc_dir_entry_tmp)
           continue;

        de = pde_subdir_find(irq_desc_info->dir, aerdrv_name, strlen(aerdrv_name));
        if (de) {

            printk("@@@@@@@@@ Find /proc entry '%s'", de->name);

            free_irq(irq_num, dev);
            printk("%s: @@@@@ release Device IRQ: %s \n",dev->name, proc_dir_entry_tmp->name);

        }
    }

    if (dev->irq >= 0) {
        free_irq(dev->irq, dev);
        dev->irq = -1;
        pci_free_irq_vectors(dev->pciDev);
    }

    if (dev->baseVirtual) {
        pci_iounmap(dev->pciDev, dev->baseVirtual);
        dev->baseVirtual = 0;
    }

    pci_release_regions(dev->pciDev);
    pci_clear_master(dev->pciDev);
    pci_disable_device(dev->pciDev);
    pci_set_drvdata(dev->pciDev, 0);

    dev->baseAddr = 0;
    kfree(dev);

    // Update Kernel log stating driver is unloaded
    printk(KERN_ALERT"%s driver is unloaded\n", dev->name);
}

// Aliasing write, read, ioctl, etc...
struct file_operations XPCIe_fops = {
    .owner = THIS_MODULE,
    .read  = XPCIe_Read,
    .write = XPCIe_Write,
    .unlocked_ioctl = XPCIe_Ioctl,
	.fasync = exerciser_fasync,
    .open  = XPCIe_Open,
    .release = XPCIe_Release,
};

static int XPCIe_Probe(struct pci_dev *pci, const struct pci_device_id *pci_id)
{
    int ret;
    u8 version;
    unsigned Device_ID;

    // Find the AMD EP device.  The device is found by matching device and vendor ID's which is defined
    // at the top of this file.  Be default, the driver will look for 10EE & 0007.  If the core is generated
    // with other settings, the defines at the top must be changed or the driver will not load
    // Update: The pci kernel module does the lookup now and calls this probe function if it
    // matched the ids defined at the top of the file.

    // TODO(michiel): Ref for now: https://elixir.bootlin.com/linux/v4.9.150/source/drivers/gpu/drm/bridge/dw-hdmi-ahb-audio.c#L582
    pbe_device *dev = kzalloc(sizeof(pbe_device), GFP_KERNEL);
    dev->name = exerciser_name;
    dev->pciDev = pci;
    dev->irq = -1;

    pci_set_drvdata(pci, dev);

    /*pci_enable_pcie_error_reporting(pci);*/

    // Bus Master Enable
    if (0 > pci_enable_device(pci)) {
        printk(KERN_WARNING"%s: Init: Device not enabled.\n", dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }
    pci_set_master(pci);

    // Check the memory region to see if it is in use
    // Try to gain exclusive control of memory for demo hardware.
    if (0 > pci_request_regions(pci, "3GIO_Demo_Drv")) {
        printk(KERN_WARNING"%s: Init: Memory in use.\n", dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }

    // Get Base Address of registers from pci structure. Should come from pci_dev
    // structure, but that element seems to be missing on the development system.
    dev->baseAddr = pci_resource_start(pci, 0);

    if (dev->baseAddr < 0) {
        printk(KERN_WARNING"%s: Init: Base Address not set.\n", dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }

    // Print Base Address to kernel log
    printk(KERN_INFO"%s: Init: Base hw val %X\n", dev->name, (unsigned int)dev->baseAddr);

    // Get the Base Address Length
    dev->baseLength = pci_resource_len(pci, 0);

    // Print the Base Address Length to Kernel Log
    printk(KERN_INFO"%s: Init: Base hw len %d\n", dev->name, (unsigned int)dev->baseLength);

    // Remap the I/O register block so that it can be safely accessed.
    // I/O register block starts at gBaseHdwr and is 32 bytes long.
    dev->baseVirtual = pci_iomap(pci, 0, dev->baseLength);
    if (!dev->baseVirtual) {
        printk(KERN_WARNING"%s: Init: Could not remap memory.\n", dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }

    // Print out the aquired virtual base addresss
    printk(KERN_INFO"%s: Init: Virt HW address %lX\n", dev->name,
            (unsigned long)dev->baseVirtual);

    //---START: Initialize Hardware

    printk(KERN_INFO"%s: Init: Initialize Hardware Done..\n",dev->name);

    // Request IRQ from OS.
    // In past architectures, the SHARED and SAMPLE_RANDOM flags were called: SA_SHIRQ and SA_SAMPLE_RANDOM
    // respectively.  In older Fedora core installations, the request arguments may need to be reverted back.
    printk(KERN_INFO"%s: Init: Device IRQ: %d\n",dev->name, pci->irq);

    if (pci_alloc_irq_vectors(dev->pciDev, 1, 32, PCI_IRQ_ALL_TYPES) < 0) {
        printk(KERN_WARNING"%s: Init: Unable to allocate all MSI IRQs", dev->name);
        XPCIe_Exit(dev);
        return CRIT_ERR;
    }

    dev->irq = pci_irq_vector(pci, 0);
    printk(KERN_INFO"%s: Allocate Device IRQ: %d\n",dev->name, pci->irq);

    if (dev->irq < 0) {
        printk(KERN_WARNING"%s: Init: Unable to find IRQ vector", dev->name);
        XPCIe_Exit(dev);
        return CRIT_ERR;
    }
    if (request_irq(dev->irq, XPCIe_IRQMSIHandler, 0, dev->name, dev) < 0) {
        printk(KERN_WARNING"%s: Init: Unable to allocate IRQ", dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }

#if 0
    irq_chip_info = irq_get_chip(dev->irq);
    printk(KERN_INFO"%s: @@@@@ Allocate Device IRQ: %s\n",dev->name, irq_chip_info->name);
    if (!irq_chip_info->parent_device) {
        printk(KERN_INFO"%s: @@@@@ Allocate Device IRQ: %s\n",dev->name, irq_chip_info->parent_device->init_name);
    }

    /*
     *irq_desc_info = irq_data_to_desc(irq_get_irq_data(dev->irq));
     *printk(KERN_INFO"%s: ##### Allocate Device IRQ: %s\n",dev->name, irq_desc_info->name);
     */
#endif

    for (irq_num = 1; irq_num < 888 ; irq_num++) {

        if (!irq_get_irq_data(irq_num))
            continue;

        irq_desc_info = irq_data_to_desc(irq_get_irq_data(irq_num));
        if (!irq_desc_info)
            continue;

        proc_dir_entry_tmp = irq_desc_info->dir;
        if (!proc_dir_entry_tmp)
           continue;

        de = pde_subdir_find(irq_desc_info->dir, aerdrv_name, strlen(aerdrv_name));
        if (de) {

            printk("######### Find /proc entry '%s'", de->name);

            if (request_irq(irq_num, XPCIe_IRQHandler, IRQF_SHARED, dev->name, dev) < 0) {
                printk(KERN_WARNING"Init: Unable to allocate IRQ : %d", irq_num);
            }
            printk("%s: @@@@@ Allocate Device IRQ: %s \n",dev->name, proc_dir_entry_tmp->name);

        }
    }

    //--- END: Initialize Hardware

    //--- START: Allocate Buffers

    // Allocate the read buffer with size DMA_BUF_SIZE and return the starting address
    dev->readBuffer = dma_alloc_coherent(&pci->dev, DMA_BUF_SIZE, &dev->readAddr, GFP_DMA | GFP_KERNEL | GFP_ATOMIC);
    if (!dev->readBuffer) {
        printk(KERN_CRIT"%s: Init: Unable to allocate read buffer.\n",dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }
    // Print Read buffer size and address to kernel log
    printk(KERN_INFO"%s: Read Buffer Allocation: %lX->%X\n", dev->name,
            (unsigned long)dev->readBuffer, (unsigned int)dev->readAddr);

    // Allocate the write buffer with size DMA_BUF_SIZE and return the starting address
    dev->writeBuffer = dma_alloc_coherent(&pci->dev, DMA_BUF_SIZE, &dev->writeAddr, GFP_DMA | GFP_KERNEL | GFP_ATOMIC);
    if (!dev->writeBuffer) {
        printk(KERN_CRIT"%s: Init: Unable to allocate gBuffer.\n",dev->name);
        XPCIe_Exit(dev);
        return (CRIT_ERR);
    }
    // Print Write buffer size and address to kernel log
    printk(KERN_INFO"%s: Write Buffer Allocation: %lX->%X\n", dev->name,
            (unsigned long)dev->writeBuffer, (unsigned int)dev->writeAddr);

    //--- END: Allocate Buffers

    //--- START: Register Driver

    // Register with the kernel as a character device.

	cdev_init(&dev->chrDev, &XPCIe_fops);
	dev->chrDev.owner = THIS_MODULE;
	ret = cdev_add(&dev->chrDev, MKDEV(AMDEXERCISER_MAJOR_NUMBER, 0), 1);
	if (ret < 0)
    {
        printk("Failed to add char device \n");
        XPCIe_Exit(dev);
    }

#if 0
    if ((major = register_chrdev(AMDEXERCISER_MAJOR_NUMBER, AMDEXERCISER_NAME, &XPCIe_fops)) < 0) {
        printk("Failed to register AMD kernel driver: %d\n", major);
        XPCIe_Exit(dev);
        goto err_unreg_chrdev;
    }
#endif

    /*if (IS_ERR(class = class_create(THIS_MODULE, AMDEXERCISER_NAME))) {*/
    if (IS_ERR(class = class_create(AMDEXERCISER_NAME))) {
        printk("Failed to creating %s device class.\n", AMDEXERCISER_NAME);
        XPCIe_Exit(dev);
        goto err_unreg_class;
    }

    if (IS_ERR(device_create(class, NULL, MKDEV(AMDEXERCISER_MAJOR_NUMBER, 0), NULL, AMDEXERCISER_NAME))) {
        printk("Failed to creating %s device .\n", AMDEXERCISER_NAME);
        XPCIe_Exit(dev);
        goto err_unreg_class;
    }

    printk(KERN_INFO"%s: Init: module registered\n", dev->name);

    //--- END: Register Driver

    // The driver is now successfully loaded.  All HW is initialized, IRQ's assigned, and buffers allocated
    printk("%s driver is loaded\n", dev->name);

    pci_read_config_dword(pci, 0, &Device_ID);
    printk(KERN_INFO "%s: Device ID : 0x%x\n", dev->name, Device_ID);

    pci_read_config_byte(pci, 8, &version);
    printk(KERN_INFO "%s: Version: %d\n", dev->name, version);

    // Initializing card registers
    XPCIe_InitCard(dev);

    return 0;

err_unreg_class:
    class_destroy(class);
    unregister_chrdev(AMDEXERCISER_MAJOR_NUMBER, AMDEXERCISER_NAME);

    return 0;
}

void amdexerciser_exit(void)
{
    printk("AMD Exerciser module exit Start ...\n");

    if (class) {
        device_destroy(class, MKDEV(AMDEXERCISER_MAJOR_NUMBER, 0));
        /*class_unregister(class);*/
        class_destroy(class);
    }
    unregister_chrdev(major, AMDEXERCISER_NAME);
    printk("AMD Exerciser module exit done\n");
}
EXPORT_SYMBOL_GPL(amdexerciser_exit);

void XPCIe_Remove(struct pci_dev *pci)
{
    amdexerciser_exit();
    XPCIe_Exit(pci_get_drvdata(pci));
}

/**
 * amdexerciser_remove() - Called when device is removed (hot-plugable)
 * @pci_dev:	PCI device information struct
 *
 * Or when driver is unloaded respecitively when unbind is done.
 */
static void amdexerciser_remove(struct pci_dev *pci_dev)
{
    printk("**TODO AER : Do amdexerciser_remove ****\n");
	/*struct amdexerciser_dev *cd = dev_get_drvdata(&pci_dev->dev);*/

	/*amdexerciser_health_check_stop(cd);*/

	/*
	 * amdexerciser_stop() must survive if it is called twice
	 * sequentially. This happens when the health thread calls it
	 * and fails on amdexerciser_bus_reset().
	 */
	/*amdexerciser_stop(cd);*/
	/*amdexerciser_pci_remove(cd);*/
	/*amdexerciser_dev_free(cd);*/
    return;
}

/**
 * amdexerciser_err_error_detected() - Error detection callback
 * @pci_dev:	PCI device information struct
 * @state:	PCI channel state
 *
 * This callback is called by the PCI subsystem whenever a PCI bus
 * error is detected.
 */
static pci_ers_result_t amdexerciser_err_error_detected(struct pci_dev *pci_dev,
						 pci_channel_state_t state)
{
	/*struct amdexerciser_dev *cd;*/

    dev_err(&pci_dev->dev, "[%s] state=%d\n", __func__, state);
    printk("**TODO AER : amdexerciser_err_error_detected ****\n");

	/*cd = dev_get_drvdata(&pci_dev->dev);*/
	/*if (cd == NULL)*/
		/*return PCI_ERS_RESULT_DISCONNECT;*/

	/* Stop the card */
	/*amdexerciser_health_check_stop(cd);*/
	/*amdexerciser_stop(cd);*/

	/*
	 * On permanent failure, the PCI code will call device remove
	 * after the return of this function.
	 * amdexerciser_stop() can be called twice.
	 */
	/*if (state == pci_channel_io_perm_failure) {*/
		/*return PCI_ERS_RESULT_DISCONNECT;*/
	/*} else {*/
		/*amdexerciser_pci_remove(cd);*/
		/*return PCI_ERS_RESULT_NEED_RESET;*/
	/*}*/
    return PCI_ERS_RESULT_NEED_RESET;
}

static pci_ers_result_t amdexerciser_err_slot_reset(struct pci_dev *pci_dev)
{
   /* int rc;*/
    printk("**TODO AER : Do slot_reset ****\n");
	/*struct amdexerciser_dev *cd = dev_get_drvdata(&pci_dev->dev);*/

	/*rc = amdexerciser_pci_setup(cd);*/
	/*if (!rc) {*/
		/*return PCI_ERS_RESULT_RECOVERED;*/
	/*} else {*/
		/*dev_err(&pci_dev->dev,*/
			/*"err: problems with PCI setup (err=%d)\n", rc);*/
		/*return PCI_ERS_RESULT_DISCONNECT;*/
   /* }*/
    return PCI_ERS_RESULT_RECOVERED;
}

static pci_ers_result_t amdexerciser_err_result_none(struct pci_dev *dev)
{
    printk("**TODO AER : return PCI_ERS_RESULT_NONE ****\n");
	return PCI_ERS_RESULT_NONE;
}

static void amdexerciser_err_resume(struct pci_dev *pci_dev)
{
    printk("**TODO AER : do resume ****\n");
	/*int rc;*/
	/*struct amdexerciser_dev *cd = dev_get_drvdata(&pci_dev->dev);*/

	/*rc = amdexerciser_start(cd);*/
	/*if (!rc) {*/
		/*rc = amdexerciser_health_check_start(cd);*/
		/*if (rc)*/
			/*dev_err(&pci_dev->dev,*/
				/*"err: cannot start health checking! (err=%d)\n",*/
				/*rc);*/
	/*} else {*/
		/*dev_err(&pci_dev->dev,*/
			/*"err: cannot start card services! (err=%d)\n", rc);*/
	/*}*/
    return;
}

static const struct pci_error_handlers amdexerciser_err_handler = {
	.error_detected = amdexerciser_err_error_detected,
	.mmio_enabled	= amdexerciser_err_result_none,
	.slot_reset	= amdexerciser_err_slot_reset,
	.resume		= amdexerciser_err_resume,
};

static struct pci_driver XPCIe_driver = {
    .name = KBUILD_MODNAME,
    .id_table = XPCIe_ids,
    .probe = XPCIe_Probe,
    .remove = XPCIe_Remove,
	.err_handler = &amdexerciser_err_handler,
};

module_pci_driver(XPCIe_driver);
MODULE_LICENSE("Dual BSD/GPL");
