// (c) Copyright 2009  2009 AMD, Inc. All rights reserved.
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
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
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
//-- Filename: amd_exerciser.h
//--
//-- Description: Main header file for kernel driver
//--
//-- PBE is an example Red Hat device driver which exercises PBE design
//-- Device driver has been tested on Red Hat Fedora FC9 2.6.15.
//--------------------------------------------------------------------------------
#include <linux/ioctl.h>
#include <linux/types.h>

#define AMDEXERCISER_VERSION "1.3.1"

#define AMDEXERCISER_NAME         "amdpcieexerciser"
#define AMDEXERCISER_DEVICE       "/dev/amdpcieexerciser"
#define AMDEXERCISER_MAJOR_NUMBER 'u'
#define AMDEXERCISER_MINOR_NUMBER 0

#define KILOBYTES(x)  ((x) * 1024L)
#define MEGABYTES(x)  (KILOBYTES(x) * 1024L)

#define SUCCESS    0
#define CRIT_ERR  -1

#define VERBOSE    1

#define DMA_BUF_SIZE   MEGABYTES(2)

typedef enum ControlReg
{
    Reg_DmaReadAddressLow       = 0x0, // NOTE: DMA Control Status
    Reg_DmaReadAddressHight     = 0x4, // NOTE: DMA Control Status
    Reg_DmaReadSize             = 0x8, // NOTE: DMA Control Status
    Reg_DmaReadControlStates    = 0xc, // NOTE: DMA Control Status
    Reg_DmaWriteAddressLow      = 0x10,
    Reg_DmaWriteAddressHight    = 0x14,
    Reg_DmaWriteSize            = 0x18,
    Reg_DmaWriteControlStates   = 0x1c, // NOTE: DMA Control Status
    Reg_ErrorInjectControl      = 0x20,
    Reg_WriteTlpPattern         = 0x24,
    Reg_WriteDMAPerf,
    Reg_ReadDMAPerf,
    Reg_ReadComplStatus,
    Reg_ComplWithData,
    Reg_ComplSize,
    Reg_DeviceLinkWidth,
    Reg_DeviceLinkTlpSize

} ControlReg;

#define PBE_IOC_MAGIC          '!'

#define PBE_IOC_INITCARD                  _IO(PBE_IOC_MAGIC, 0)
#define PBE_IOC_READ_READ_DMA_CTRL        _IOR(PBE_IOC_MAGIC, 1, uint32_t *)
#define PBE_IOC_READ_WRITE_DMA_CTRL       _IOR(PBE_IOC_MAGIC, 2, uint32_t *)
#define PBE_IOC_READ_RD_ADDR_LOW          _IOR(PBE_IOC_MAGIC, 3, uint32_t *)
#define PBE_IOC_READ_RD_ADDR_HIGHT        _IOR(PBE_IOC_MAGIC, 4, uint32_t *)
#define PBE_IOC_READ_WR_ADDR_LOW          _IOR(PBE_IOC_MAGIC, 5, uint32_t *)
#define PBE_IOC_READ_WR_ADDR_HIGHT        _IOR(PBE_IOC_MAGIC, 6, uint32_t *)
#define PBE_IOC_READ_RD_LEN               _IOR(PBE_IOC_MAGIC, 7, uint32_t *)
#define PBE_IOC_READ_WR_LEN               _IOR(PBE_IOC_MAGIC, 8, uint32_t *)
#define PBE_IOC_READ_WR_PERF              _IOR(PBE_IOC_MAGIC, 9, uint32_t *)
#define PBE_IOC_READ_RD_PERF              _IOR(PBE_IOC_MAGIC, 10, uint32_t *)
#define PBE_IOC_READ_CMPL                 _IOR(PBE_IOC_MAGIC, 11, uint32_t *)
#define PBE_IOC_READ_CWDATA               _IOR(PBE_IOC_MAGIC, 12, uint32_t *)
#define PBE_IOC_READ_CSIZE                _IOR(PBE_IOC_MAGIC, 13, uint32_t *)
#define PBE_IOC_READ_LINKWDTH             _IOR(PBE_IOC_MAGIC, 14, uint32_t *)
#define PBE_IOC_READ_LINKLEN              _IOR(PBE_IOC_MAGIC, 15, uint32_t *)
#define PBE_IOC_READ_INTRPT               _IOR(PBE_IOC_MAGIC, 16, uint32_t *)
#define PBE_IOC_READ_DIR_LINK             _IOR(PBE_IOC_MAGIC, 17, uint32_t *)
#define PBE_IOC_WRITE_WR_ADDR_LOW         _IOR(PBE_IOC_MAGIC, 20, uint32_t *)
#define PBE_IOC_WRITE_RD_ADDR_LOW         _IOR(PBE_IOC_MAGIC, 21, uint32_t *)
#define PBE_IOC_WRITE_RD_ADDR_HIGHT       _IOR(PBE_IOC_MAGIC, 22, uint32_t *)
#define PBE_IOC_WRITE_WR_ADDR_HIGHT       _IOR(PBE_IOC_MAGIC, 23, uint32_t *)
#define PBE_IOC_WRITE_READ_DMA_CTRL       _IOR(PBE_IOC_MAGIC, 24, uint32_t *)
#define PBE_IOC_WRITE_WRITE_DMA_CTRL      _IOR(PBE_IOC_MAGIC, 25, uint32_t *)
#define PBE_IOC_WRITE_WR_LEN              _IOW(PBE_IOC_MAGIC, 26, uint32_t)
#define PBE_IOC_WRITE_WR_PTRN             _IOW(PBE_IOC_MAGIC, 27, uint32_t)
#define PBE_IOC_WRITE_RD_LEN              _IOW(PBE_IOC_MAGIC, 28, uint32_t)
#define PBE_IOC_WRITE_RD_PTRN             _IOW(PBE_IOC_MAGIC, 29, uint32_t)
#define PBE_IOC_WRITE_DIR_LINK            _IOW(PBE_IOC_MAGIC, 30, uint32_t)
#define PBE_IOC_RD_ANY_REG                _IOWR(PBE_IOC_MAGIC, 31, uint32_t *)
#define PBE_IOC_RD_CFG_REG                _IOWR(PBE_IOC_MAGIC, 32, uint32_t *)
#define PBE_IOC_WR_ANY_REG                _IOW(PBE_IOC_MAGIC, 33, uint64_t)
#define PBE_IOC_WR_CFG_REG                _IOW(PBE_IOC_MAGIC, 34, uint64_t)

#define PBE_IOC_READ_ERRORINJECT          _IOR(PBE_IOC_MAGIC, 35, uint32_t *)
#define PBE_IOC_WRITE_ERRORINJECT         _IOR(PBE_IOC_MAGIC, 36, uint32_t *)
