// (c) Copyright 2009  2009 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.

//--------------------------------------------------------------------------------
//-- Filename: amdexerciser_main.h
//--
//-- Description: Main header file stating defines
//--
//-- AMDEXERCISER is an example Red Hat device driver which exercises AMDEXERCISER design
//-- Device driver has been tested on Red Hat Fedora FC9 2.6.15.
//--------------------------------------------------------------------------------

// Define EXERCISER Descriptor Registers
#define DCSR 0                  //offset for AMDEXERCISER Device Control Status Register
#define DMACR 1                 //offset for AMDEXERCISER DMA Control Status Register
#define WDMATLPA 2              //offset for AMDEXERCISER Write DMA TLP Address Register
#define WDMATLPS 3              //offset for AMDEXERCISER Write DMA TLP Size Register
#define WDMATLPC 4              //offset for AMDEXERCISER Write DMA TLP Count Register
#define WDMATLPP 5              //offset for AMDEXERCISER Write DMA TLP Pattern Register
#define RDMATLPP 6              //offset for AMDEXERCISER Read DMA TLP Pattern Register
#define RDMATLPA 7              //offset for AMDEXERCISER Read DMA TLP Address Register
#define RDMATLPS 8              //offset for AMDEXERCISER Read DMA TLP Size Register
#define RDMATLPC 9              //offset for AMDEXERCISER Read DMA TLP Count Register
#define WDMAPERF 10             //offset for AMDEXERCISER Write DMA Performance Register
#define RDMAPERF 11             //offset for AMDEXERCISER Read DMA Performance Register
#define RDMASTAT 12             //offset for AMDEXERCISER Read DMA Status Register
#define NRDCOMP 13              //offset for AMDEXERCISER Number of Read Completion w/ Data Register
#define RCOMPDSIZE 14           //offset for AMDEXERCISER Read Completion Data Size Register
#define DLWSTAT 15              //offset for AMDEXERCISER Device Link Width Status Register
#define DLTRSSTAT 16            //offset for AMDEXERCISER Device Link Transaction Size Register
#define DMISCCONT 17            //offset for AMDEXERCISER Device Miscellaneous Control Register
#define DMSICONT 18             //offset for AMDEXERCISER Device MSI Control Register
#define DLNKC 19                //offset for AMDEXERCISER Device Directed Link Change Register
#define DFCCTL 20               //offset for AMDEXERCISER Device FC Control Register
#define DFCPINFO 21             //offset for AMDEXERCISER Device FC Posted Information Register
#define DFCNPINFO 22            //offset for AMDEXERCISER Device FC Non Posted Information Register
#define DFCCINFO 23             //offset for AMDEXERCISER Device FC Completion Information Register

