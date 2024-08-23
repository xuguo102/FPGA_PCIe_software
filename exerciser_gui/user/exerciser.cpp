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

// --------------------------------------------------------------------------------------------------------------
//  exerciser.cpp description:
//  This module is sub-class of the amdexerciser_ep_t class type.  It is responsible for the following:
//    1) Exercising the AMDEXERCISER demo by performing DMA transfers based on user settings within the GUI (run_amdexerciser)
//    2) Error checking and performance calculations after DMA transfer has completed
//    3) Returning performance results from DMA transfers
//    4) Returning DMA text details and whether the DMA transaction was a success or failure
//    5) Reading and returning AMDEXERCISER descriptor registers so contents can be view in GUI
//---------------------------------------------------------------------------------------------------------------

//member functions
exerciser_t::exerciser_t(void){
  this->wr_result_text = " ";      // Contains text stating success of WR DMA or error condition if one exists
  this->rd_result_text = " ";      // Contains text stating success of RD DMA or error condition if one exists
  this->exerciser_fatal_text = " ";      // Contains text stating error condition if EXERCISER fails during setup of transfer
  this->wr_success = false;        // Bool declaring if Write DMA was successful
  this->rd_success = false;        // Bool declaring if Read DMA was successful
}

exerciser_t::~exerciser_t(void){
}

//--- WriteData(): Writes data to kernel buffer
//--- Arguments: Takes in the device file, user mode buffer containing data, and size to be transferred
//--- Return Value: Returns INT stating SUCCESS or failure
//--- Detailed Description: Using a standard IOCTL command, writes data to the kernel buffer allocated by driver
int WriteData(int g_devFile, char* buff, size_t size)
{
  int ret = write(g_devFile, buff, size);
  return (ret);
}

//--- ReadData(): Reads data from a kernel buffer
//--- Arguments: Takes in the device file, user mode buffer containing data, and size to be transferred
//--- Detailed Description: Using a standard IOCTL command, writes data to the kernel buffer allocated by driver
int ReadData(int g_devFile, char* buff, size_t size)
{
  int ret = read(g_devFile, buff, size);
  return (ret);
}

struct TransferData  {
  unsigned int data[DMA_BUF_SIZE/4];

} *gReadData, *gWriteData;


//--- run_amdexerciser(): Runs a single DMA iteration
//--- Arguments: Takes in a global struct containing AMDEXERCISER descriptor register values used for setting up transfer
//--- Detailed Description: This module does the following:
//---                       1) Sets up and initializes any variables needed for transfer
//---                       2) Writes the required AMDEXERCISER descriptor registers and lastly, writes to AMDEXERCISER control
//---                          register to start the transfer
//---                       3) After DMA iteration, verifies data written from card into kernel buffer matches the
//---                          pattern established for the transfer
//---                       4) After DMA iteration, verifies that the control register reports that the Write DMA
//---                          engine has completed successfully
//---                       5) After DMA iteration, verifies that the control register reports that the Read DMA
//---                          engine has completed successfully
//---                       6) Updates the current read and write performance numbers.
int exerciser_t::run_amdexerciser(amdexerciser_descriptors_t amdexerciser_descriptors, int ii) {

  // Variable Declaration

  //loop variables
  unsigned int jj;                          // Used for loops requiring buffer access
  unsigned int ll;                          // Used to build arrays containing valid packet sizes/link widths

  int modulo4096 = 1;                       // used to help form modulo 4096 array
  unsigned int mode;                        // Random mode: variable which decides if transfer is >4096 or <4096 bytes

  // EXERCISER Register Variables
  unsigned int dmacr_reg_wr = 0x00000000;   // contains lower 16 bits of DMACR reg used to verify success WR DMA trans
  unsigned int dmacr_reg_rd = 0x00000000;   // contains lower 16 bits of DMACR reg used to verify success RD DMA trans
  unsigned int dmacr_reg = 0x00000000;      // contains all 32 bits of DMACR reg.

  // Device CFG space variables
  unsigned int dcontrol_reg = 0x00000000;   // contains device control register contents used to verify MPS is valid

  // Count, size, and pattern variables to
  int tlpsize[11];                          // Random mode: contains all valid TLP sizes for >4096 transfers
  int tlpsizemax = 0;                       // Random mode: index in tlpsize array to make sure payload isn't > MPS
  unsigned int mps = 32;                    // contains MPS for EP. Default to 32dwords (128bytes)
  unsigned int reg_value = 0;               // contains result of IOCTL reads to AMDEXERCISER descriptor regs
  unsigned int wrwdmatlpc=0;                // contains WR DMA TLP count
  unsigned int wrwdmatlps=0;                // contains WR DMA TLP payload size
  unsigned int wrwdmatlpp=0;                // contains WR DMA TLP pattern
  unsigned int wrrdmatlpc=0;                // contains RD DMA TLP count
  unsigned int wrrdmatlps=0;                // contains RD DMA TLP payload size
  unsigned int wrrdmatlpp=0;                // contains RD DMA TLP pattern

  unsigned int dmisccont = 0;               // contains DMISCCONT register value used to enable trans. streaming

  unsigned int rd_phantom_en = 0;           // used to enable read phantom functions

  // FILE IO - all status displayed to log file amdexerciser_log.txt which is then displayed to GUI upon completion of DMA
  ofstream file;

  // Random Number Variables.  Random numbers are used to generate DMA transfers with Random packet size, TLP count,
  // and TLP pattern.  Also randomized is whether the transfer is > 4096 bytes or < 4096 bytes.  It also randomizes
  // whether read phantom functions and transmit streaming are enabled.
  static MTRand *mtrandPtrRdsize = NULL;
  static unsigned int seedRdsize = 1;

  static MTRand *mtrandPtrWrsize = NULL;
  static unsigned int seedWrsize = 10;

  static MTRand *mtrandPtrRdcount = NULL;
  static unsigned int seedRdcount = 1;

  static MTRand *mtrandPtrWrcount = NULL;
  static unsigned int seedWrcount = 10;

  static MTRand *mtrandMode = NULL;
  static unsigned int seedMode = 1;

  static MTRand *mtrandPattern = NULL;
  static unsigned int seedPattern = 1;

  static MTRand *mtrandWrPhantom = NULL;
  static unsigned int seedWrPhantom = 1;

  static MTRand *mtrandRdPhantom = NULL;
  static unsigned int seedRdPhantom = 1;

  static MTRand *mtrandTransStreaming = NULL;
  static unsigned int seedTransStreaming = 1;

  // Setting seeds for all the random number generation variables
  seedPattern = 1;
  seedMode=2;
  seedRdsize=3;
  seedWrsize=4;
  seedRdcount=5;
  seedWrcount=6;
  seedRdPhantom=7;
  seedWrPhantom=8;
  seedTransStreaming = 9;

  // Test variables
  unsigned long testseed=time(NULL);
  file << "SEED VALUE : "<<testseed<<endl;

  // Initialize MTRand pointers
  mtrandPtrWrsize = new MTRand();
  mtrandPtrRdsize = new MTRand();
  mtrandPtrWrcount = new MTRand();
  mtrandPtrRdcount = new MTRand();
  mtrandMode  = new MTRand();
  mtrandPattern = new MTRand();
  mtrandWrPhantom = new MTRand();
  mtrandRdPhantom = new MTRand();
  mtrandTransStreaming = new MTRand();

  // Open the log file which is eventually displayed in GUI.  If failure occurs, return and set the fatal text which
  // is displayed to the GUI main status bar
  file.open ("amdexerciser_log.txt",ios::app);
  if (!file.is_open()) {
    printf("Error opening log file");
    this->exerciser_fatal_text = "Error Opening Log File: EXERCISER.cpp";
    return CRIT_ERR;
  }

  // Allocate the user mode read data buffer
  gReadData = (TransferData  *) malloc(sizeof(struct TransferData));
  // Allocate the user mode write data buffer
  gWriteData = (TransferData  *) malloc(sizeof(struct TransferData));


  // We read the EP device control register to identify the MPS set for this system.  The MPS is then used to get
  // and index into the Modulo4096 array.  This is needed to guarantee the AMDEXERCISER does not generate packets that exceed
  // the max payload defined by the host upon configuration.  This is used in Random Mode.  The GUI currently restricts
  // packet payloads to 128bytes which is what most chipsets on the market are limited to.
  reg_value = amdexerciser_descriptors.device_stat_cont_offset;
  // To read EP configuration space or AMDEXERCISER descriptor registers, we used pre-defined IOCTL calls (/sys/ioctl.h).
  // ioctl calls take in the device file number, register to be read, and returns the contents of that register.
  // If ioctl call fails, we set the fatal text variable and return immediately.  GUI will show text in main status
  // bar
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_RD_CFG_REG, &reg_value) < 0) {
    this->exerciser_fatal_text = "Error Reading Device Control Reg: EXERCISER.cpp";
    file << "Device Control Register 0x40 Read failed" << endl;
    return CRIT_ERR;
  } else {
    dcontrol_reg = (reg_value & 0x000000E0) >> 5;   // mask off device MPS bits
    mps = dcontrol_reg;
  }
  // Sets up index into Modulo 4096 array
  switch (dcontrol_reg) {

    case 0: // 128 MPS
      tlpsizemax = 5;
      break;
    case 1:  // 256 MPS
      tlpsizemax = 6;
      break;
    case 2:  // 512 MPS
      tlpsizemax = 7;
      break;
    case 3:  // 1024 MPS
      tlpsizemax = 8;
      break;
    case 4: // 2048 MPS
      tlpsizemax = 9;
      break;
    case 5: // 4096 MPS
      tlpsizemax = 10;
      break;
    default: {  //others - non valid MPS setting
      file << "MPS value failure";
      this->exerciser_fatal_text = "Device Control Reg MPS invalid: EXERCISER.cpp 200";
      return CRIT_ERR;
    }
  }

  // Loop builds the tlpsize array which will holds all possible TLP SIZE values which are Modulo 4096 -
  // We use payload sizes that are modulo4096 only when transferring > 4096 bytes.  This gaurantees we do not cross
  // a page boundary in the middle of a TLP.  We use the tlpsizemax index to verify we are always within the MPS
  // defined by the system
  // TLPSIZE = [1,2,4,8,16,32,64,128,256,512,1024,2048,4096]
  modulo4096 = 1;
  for (ll=0; ll < 11; ll++) {
    tlpsize[ll] = modulo4096;
    modulo4096 = modulo4096*2;
  }


  // Use the current iteration count to identify if we exceed 10,000 cycles.  Change out logs at 10,000 or else contents
  // become very large and risk issues loading into GUI.  Place the previous 10,000 iterations into log called
  // "prev_amdexerciser_log.txt."  At all times, not more than the previous 20,000 iterations can be viewed.
  // If a FILEIO error is seen, return and state issue in GUI main status bar.
  if (amdexerciser_descriptors.num_iter == 10000 ) {
    file.close();
    remove("prev_amdexerciser_log.txt");
    rename ("amdexerciser_log.txt", "prev_amdexerciser_log.txt");
    file.open ("amdexerciser_log.txt");
    if (!file.is_open()) {
      this->exerciser_fatal_text = "Error Opening Log File: EXERCISER.cpp";
      return CRIT_ERR;
    }
  }

  file << "--------------------------------- AMDEXERCISER App Iter Count = " <<dec<< ii <<" ---------------------------------\n";



  // The code below diverges into to main sections: Random Mode and Static Mode.
  // Random Mode has two subsections: Transfers > 4096 bytes and Transfer < 4096 bytes

  // RANDOM MODE:
  // All GUI inputs are disregarded and random DMA transfer settings are generated
  // When running with transfers > 4096 bytes, all TLP sizes will be modulo4096 to ensure a page boundary isn't
  // crossed in the middle of a TLP; a requirement by the spec.

  // When running with transfer < 4096 bytes, all TLP sizes are fair game since we guarantee a total transfer will not
  // exceed a page boundary.

  // STATIC MODE:
  // When running in static mode, we take in the the GUI settings (TLP size, count, etc..).
  // All iterations in static mode are < 4096 bytes and is restricted by the GUI.


  if (amdexerciser_descriptors.random_enable == 1) {
    // Generate mode variable randomly which determines >4096 bytes or <4096 byte transfers
    mode = mtrandMode->randInt(1);

    //If Transmit Streaming is enabled, randomly decide whether this iteration will have it enabled
    if (amdexerciser_descriptors.trans_streaming == true) {
      dmisccont = (mtrandTransStreaming->randInt(1)) << 9;
      if (dmisccont == 0x00000020) {
        file << "Transmit Streaming Enabled\n"<<endl;
      }
      dmisccont = dmisccont | 0x01010000;
    } else {
      dmisccont = 0x01010000;
    }

    // If Phantom Functions are enabled, randomly decide whether this iteration will have it enabled. Write Phantom
    // functions not supported
    if (amdexerciser_descriptors.phantom_enable == true) {
      rd_phantom_en = mtrandRdPhantom->randInt(1);
      if (rd_phantom_en == 1) {
        file << "Read Phantom Functions Enabled\n"<<endl;
      }
      rd_phantom_en = rd_phantom_en << 20;
      wrrdmatlps = (wrrdmatlps | rd_phantom_en);
    }

    // MODE = 0 implies that > 4096 transfer was selected.
    if (mode == 0) {

      // Generate a randomly selected WR TLP size that is modulo4096 from the tlpsize array. Indexed by the tlpsizemax
      // variable
      wrwdmatlps = tlpsize[mtrandPtrWrsize->randInt(tlpsizemax)];

      // Generate a random WR TLP count but verify that the total transfer does not exceed the total kernel buffer
      // allocated
      wrwdmatlpc = (65536/wrwdmatlps);

      // Generate a randomly selected RD TLP size that is modulo4096 from the tlpsize array. Indexed by the tlpsizemax
      // variable
      wrrdmatlps = tlpsize[mtrandPtrRdsize->randInt(tlpsizemax)];

      // Generate a random RD TLP count but verify that the total transfer does not exceed the total kernel buffer
      // allocated
      wrrdmatlpc = (65536/wrrdmatlps);

    // MODE = 1 implies that < 4096 transfer was selected
    } else {

      // Generate a randomly selected WR TLP size.  All TLP sizes are valid since we will not cross 4096 boundary
      wrwdmatlps = (1 + mtrandPtrWrsize->randInt(tlpsize[tlpsizemax]-1));
      wrwdmatlpc = (1024/wrwdmatlps);

      // Generate a randomly selected RD TLP size.  All TLP sizes are valid since we will not cross 4096 boundary
      wrrdmatlps = 1 + mtrandPtrRdsize->randInt(tlpsize[tlpsizemax]-1);
      wrrdmatlpc = (1024/wrrdmatlps);
    }

    // We now get a random TLP pattern which has no dependency on the random mode selected
    wrwdmatlpp = mtrandPattern->randInt(0xFFFFFFFF);
    wrrdmatlpp = mtrandPattern->randInt(0xFFFFFFFF);

  // STATIC mode selected
  } else {

    // Take in GUI settings
    wrwdmatlps = amdexerciser_descriptors.wr_tlp_size;                          // Write TLP size
    wrwdmatlpc = amdexerciser_descriptors.num_wr_tlps;                          // Write TLP count
    wrrdmatlps = amdexerciser_descriptors.rd_tlp_size;                          // Read TLP size
    wrrdmatlpc = amdexerciser_descriptors.num_rd_tlps;                          // Read TLP count
    wrrdmatlpp = strtoul(amdexerciser_descriptors.rd_pattern_new, NULL, 16);    // Write TLP pattern - convert to int
    wrwdmatlpp = strtoul(amdexerciser_descriptors.wr_pattern_new, NULL, 16);    // Read TLP pattern  - convert to int
  }

  // Initalize the DMA target user mode buffer - Pattern data is data returned on read completions and must match
  // WRRDMATLPP EXERCISER descriptor register
  for (int jj = 0; jj < DMA_BUF_SIZE/4; jj++) {
    gWriteData->data[jj]= wrrdmatlpp;
  }

  // Write buffer initialized above to kernel mode buffer so that pattern read from Kernel buffer matches the expected
  // pattern checked by AMDEXERCISER
  WriteData(amdexerciser_descriptors.g_devFile, (char*) gWriteData, DMA_BUF_SIZE);

  // -------------------------------------------------------------------------------------
  //  SETTING UP AMDEXERCISER DESCRIPTOR REGISTERS
  // -------------------------------------------------------------------------------------

  // The following IOCTL calls read and write the AMDEXERCISER descriptor registers which set up the transfer.  When all
  // registers are set up, we do one final write to the AMDEXERCISER control register to kick of the Write and Read DMA engines

  // Reset AMDEXERCISER
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_RESET, 0x00000000) < 0) {
    file << "INITRST Failed\n";
    this->exerciser_fatal_text = "INITRST Failed: EXERCISER.cpp";
    return CRIT_ERR;
  }

  // Read AMDEXERCISER Control Register
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_DMA_CTRL, &dmacr_reg) < 0) {
    file << "DDMACR Read Failed\n";
    this->exerciser_fatal_text = "DDMACR Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
    file << "EXERCISER Descriptor Registers: \nDMACR Value = " <<hex<< dmacr_reg<<"\n";
    #endif
  }

  // Writes AMDEXERCISER TLP count register indicating # of WRITE TLP's to send from card
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_WR_COUNT, wrwdmatlpc) < 0) {
    file << "WDMATLPC Write Failed\n";
    this->exerciser_fatal_text = "WDMATLPC Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // Writes TLP size register indicating payload of each WRITE TLP sent from card
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_WR_LEN, wrwdmatlps) < 0) {
    file << "WDMATLPS Write Failed\n";
    this->exerciser_fatal_text = "WDMATLPS Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // Writes the TLP count indicating number of READ TLP's sent from card
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_RD_COUNT, wrrdmatlpc) < 0) {
    file << "WDMATLPC Write Failed\n";
    this->exerciser_fatal_text = "WDMATLPC Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // Writes the TLP size register indicating requested payload seen in each READ TLP sent from card.
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_RD_LEN, wrrdmatlps) < 0) {
    file << "RDMATLPS Write Failed\n";
    this->exerciser_fatal_text = "RDMATLPS Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // this IOCTL call sets the pattern for writes from EXERCISER to system memory
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_WR_PTRN, wrwdmatlpp) < 0) {
    file << "RDMATLPP Write Failed\n";
    this->exerciser_fatal_text = "RDMATLPP Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // this IOCTL call sets the pattern for writes from EXERCISER to system memory
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_RD_PTRN, wrrdmatlpp) < 0) {
    file << "RDMATLPP Write Failed\n";
    this->exerciser_fatal_text = "RDMATLPP Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // Reads WR_DMA TLP COUNT REGISTER
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_WR_COUNT, &reg_value) < 0) {
    file << "WDMATLPC Read Failed\n";
    this->exerciser_fatal_text = "WDMATLPC Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file << "WDMATLPC Value = "<<hex<<reg_value<<"\n";
    #endif
  }

  // Reads WR_DMA TLP SIZE REGISTER
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_WR_LEN, &reg_value) < 0) {
    file << "WDMATLPS Read Failed\n";
    this->exerciser_fatal_text = "WDMATLPS Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file <<"WDMATLPS Value = "<<hex<<reg_value<<"\n";
    #endif
  }

  // Reads RD_DMA TLP Count Register
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_RD_COUNT, &reg_value) < 0) {
    file << "RDMATLPC Read Failed\n";
    this->exerciser_fatal_text = "RDMATLPC Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file << "RDMATLPC Value = "<<hex<<reg_value<<"\n";
    #endif
  }

  // Reads RD_DMA TLP SIZE REGISTER
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_RD_LEN, &reg_value) < 0) {
    file << "RDMATLPS Read Failed\n";
    this->exerciser_fatal_text = "RDMATLPS Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file << "RDMATLPS Value = "<<hex<<reg_value<<"\n";
    #endif
  }

  // Reads WR_DMA TLP PATTERN REGISTER
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_WR_PTRN, &reg_value) < 0) {
    file << "WDMATLPP Read Failed\n";
    this->exerciser_fatal_text = "WDMATLPP Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file <<"WDMATLPP Value = "<<hex<<reg_value<<"\n";
    #endif
  }

  // Reads RD_DMA TLP PATTERN REGISTER
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_RD_PTRN, &reg_value) < 0) {
    file << "RDMATLPP Read Failed\n";
    this->exerciser_fatal_text = "RDMATLPP Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    #ifdef Verbose
      file <<"RDMATLPP Value = "<<hex<<reg_value<<"\n\n";
    #endif
  }

  // WRR Control For Soft Core (no cpl streaming on) WRR 1:1
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_MISC_CTL, dmisccont) < 0) {
    file << "DMISCCONT Write Failed\n";
    this->exerciser_fatal_text = "DMISCCONT Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // OR together the current dmacr_reg with WR_ENABLE and READ_ENABLE to kick off transfer.
  // WRITE_ENABLE AND READ_ENABLE are controlled by amdexerciser_main.cpp and will only be activated if user enables them
  dmacr_reg = dmacr_reg_rd | amdexerciser_descriptors.rd_enable | amdexerciser_descriptors.wr_enable;
  // Initiator Start - Writes the Control Status Descriptor Register (DDMACR)
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_WRITE_DMA_CTRL, dmacr_reg) < 0 ) {
    file << "DMACR Write Failed\n";
    this->exerciser_fatal_text = "DMACR Write Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  }

  // -------------------------------------------------------------------------------------
  //  WAIT FOR DMA ITERATION TO COMPLETE
  // -------------------------------------------------------------------------------------
  usleep (30000);	//This wait is required to make sure all data is transfered before checking integrity.


  // -------------------------------------------------------------------------------------
  //  CHECK KERNEL BUFFER AND AMDEXERCISER STATUS REGISTERS FOR ERRORS
  // -------------------------------------------------------------------------------------

  // This for loop module zeroes out the data that was previously recorded in user mode buffer.  This is required
  // in case the user keeps running with the same TLP pattern.  An error condition may be not be seen as the data
  // read will match
  for (jj = 0; jj < DMA_BUF_SIZE/4; jj++) {
   gReadData->data[jj]= 0x00000000;
  }

  // Read the AMDEXERCISER control register which will state whether AMDEXERCISER encountered an error condition during transfer.
  // If error occurs on read, update fatal_text and return to GUI which will show error in ma in status bar
  if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_DMA_CTRL, &reg_value) < 0) {
    file << "DMACR Read Failed\n";
    this->exerciser_fatal_text = "DMACR Read Failed: EXERCISER.cpp";
    file.close();
    return CRIT_ERR;
  } else {
    dmacr_reg = reg_value;
    // mask off the write bits (lower 16) to check write DMA seperately
    dmacr_reg_wr = dmacr_reg & 0x00001111;
    // mask off the read bits (upper 16) to check read DMA seperately
    dmacr_reg_rd = dmacr_reg & 0x11110000;
  }

  // Read data from Kernel Buffer to User Buffer...
  ReadData(amdexerciser_descriptors.g_devFile, (char *) gReadData, (wrwdmatlps * wrwdmatlpc * 4));

  // ----- WRITE ERROR CHECK -----//
  if (amdexerciser_descriptors.wr_enable == 0x00000001)  {

    // Now check to see if the data seen in user buffer matches the expected pattern.
    for (jj = 0; jj < (wrwdmatlps * wrwdmatlpc); jj++) {
      if (gReadData->data[jj] != wrwdmatlpp) {
        // If a mismatch does occur, output the details to the log.
        file << "*** Write DMA Error (EP -> Chipset) ***\n Details: Data Error Found when comparing data written to kernel buffer with expected pattern: Write DMA \n TLP Size = " << wrwdmatlps << "\n TLP Count = "<< wrwdmatlpc << "\n";

        // State the mismatch found
        file << hex<< gReadData->data[jj] << "(Data Buffer)" << " != "<< hex<< wrwdmatlpp << "(Expected)\n\n";

        // close the log file
        file.close();

        // set the result text stating a mismatch found
        this->wr_result_text = "ERROR: WR Mismatch with expected value";

        // set the WR success variable to false.  Do not return because we need to check whether reads were successful.
        // amdexerciser_main.cpp checks for success after each iteration and will not proceed
        this->wr_success=false;
        break;
      } else {
        // If no errors are found, set success = true.
        this->wr_success=true;
      }
    }

    // if no mismatch is found, lets do a final check of DMACR_REG to make sure WR_DMA completed successfully
    if (this->wr_success != false) {
      if (dmacr_reg_wr!= 0x00000101) {
        // if an error is found, output the control register contents to the log
        file << "DMACR register contents = 32'h" << hex << dmacr_reg<< "\n";

        // Update the WR result text
        this->wr_result_text = "ERROR: WR DMA did not complete successfully";

        // set the WR success variable to false
        this->wr_success=false;
      } else {
        //if both checks fall through- consider a success
        file << "WRITE DMA SUCCESS!"<<endl;
        this->wr_result_text = "WRITE DMA SUCCESS!";
        this->wr_success=true;
      }
    }
  }

  // ----- READ ERROR CHECK -----//
  if (amdexerciser_descriptors.rd_enable == 0x00010000)  {
    // EXERCISER responsible for error checking on read completions and recording to AMDEXERCISER control register
    if (dmacr_reg_rd != 0x01010000) {
      // If error is found, record AMDEXERCISER control register to log
      file << "*** Read DMA Error ***\n Details: Data Error Found by EXERCISER backend and recorded in DDMACR register \n TLP Size = " << wrwdmatlps << "\n TLP Count = "<< wrwdmatlpc <<"\n";
      file << "DMACR register contents =32'h" << hex << dmacr_reg << "\n";

      // close out the log file
      file.close();

      // update the RD result text variable
      this->rd_result_text = "ERROR: DMACR !=0101xxxx";

      // set the RD success variable to false
      this->rd_success=false;;
    } else {
      // Transfer is successful.  State in log file
      file << "READ DMA SUCCESS!"<<endl;

      // update RD result text variable
      this->rd_result_text = "READ DMA SUCCESS!";

      // set the RD success variable to true
      this->rd_success=true;
    }
  }

     // Checking Device Status for FATAL error and returning if one is seen.  Only checking for Fatal condition (Bit 2)
     // Change value of 4 to check for other errors (Bit 0 = Correctable, Bit 1 = Non Fatal, Bit 3 = UR)
    reg_value = amdexerciser_descriptors.device_stat_cont_offset;
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_RD_CFG_REG, &reg_value) < 0) {
      file <<"Device Status Read failed"<<endl;
      this->exerciser_fatal_text = "Device Status Read failed: EXERCISER.cpp";
      file.close();
      return CRIT_ERR;
    } else {
      // Mask off Fatal Error bit
      if (((reg_value & 0x00040000)>>16) == 0x00000004) {
        //printf("CRITICAL ERROR REPORTED BY EP DEVICE");
        this->exerciser_fatal_text = "FATAL ERROR REPORTED BY EP DEVICE: EXERCISER.cpp";
        file.close();
        return CRIT_ERR;
      }
    }

  // -------------------------------------------------------------------------------------
  //  Check performance and update performance member variables
  // -------------------------------------------------------------------------------------   ///

  // Performance is calculated by counting TRN clocks.
  // The TRN clock frequency, which is the clock driving the AMDEXERCISER, varies based on the link width and link speed.  To
  // correctly calculate performance, we must identify the link width and speed.

  int link_width_multiplier = 0;                 // Multiplier value calculated based on link width and speed
  int trn_clks = 0;                              // variable containing the number of TRN clks counted
  int temp_wr_mbps = 0;                          // Temp WR performance variable needed
  int temp_rd_mbps = 0;                          // Temp RD performance variable needed

  // Seperate performance output in log
  file << "\nPerformance (Mbps):\n";

  switch (amdexerciser_descriptors.neg_link_speed) {

    //GEN1
    case 1: {
      switch (amdexerciser_descriptors.neg_link_width) {
        case 1: // x1
          link_width_multiplier = 31;
          break;
        case 2:  // x2
          link_width_multiplier = 62;
          break;
        case 4:  // x4
          link_width_multiplier = 125;
          break;
        case 8:  // x8
          link_width_multiplier = 250;
          break;
        default: {  //others - non-valid link width
          this->exerciser_fatal_text = "Link Width is not valid: EXERCISER.cpp";
	  file.close();
          return CRIT_ERR;
        }

      }
      break;
    }

    // GEN2
    case 2: {
      switch (amdexerciser_descriptors.neg_link_width) {
        case 1: // x1
          link_width_multiplier = 62;
          break;
        case 2:  // x2
          link_width_multiplier = 125;
          break;
        case 4:  // x4
          link_width_multiplier = 250;
          break;
        case 8:  // x8
          link_width_multiplier = 500;
          break;
        default: {  //others - non-valid link width
          this->exerciser_fatal_text = "Link Width is not valid: EXERCISER.cpp";
	  file.close();
          return CRIT_ERR;
        }
      }
      break;
    }
    default: {  //others - non-valid link width
      this->exerciser_fatal_text = "Link Speed is not valid: EXERCISER.cpp";
      file.close();
      return CRIT_ERR;
    }
  }

  // Only update performance if Write DMA is enabled.  Won't be calculated on error condition as we will have already
  // returned by this point
  if (amdexerciser_descriptors.wr_enable == 0x00000001)  {
    if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_WR_PERF, &trn_clks) < 0) {

      // If error is found when reading RD performance register, update the fatal text and return with CRIT_ERR
      file << "RDMATLPP Read Failed\n";
      this->exerciser_fatal_text = "RDMATLPP Read Failed: EXERCISER.cpp";
      file.close();
      return CRIT_ERR;
    }

    // Performance is calculated the following way:
    // tlp_size*4*tlp_count = total bits transferred
    // Multiply by the link width multiplier and divide by the total number of trn_clks to get mbps
    temp_wr_mbps = (amdexerciser_descriptors.wr_tlp_size*4*amdexerciser_descriptors.num_wr_tlps*8*link_width_multiplier)/(trn_clks);
    // add the calculated WR performance to the current value.  Average found later by dividing by total iterations
    this->wr_mbps=temp_wr_mbps+this->wr_mbps;
    file << "Write TRN clocks = " << trn_clks <<"\n";
    file << "Write DMA Performance = "<<temp_wr_mbps<<"\n";
  }

  // Only update performance if Read DMA is enabled.  Won't be calculated on error condition
  if (amdexerciser_descriptors.rd_enable == 0x00010000)  {
    // Reads WR_DMA PERFORMANCE REGISTER
        if (ioctl(amdexerciser_descriptors.g_devFile, XBMD_IOC_READ_RD_PERF, &trn_clks) < 0) {

      // If error is found when reading RD performance register, update the fatal text and return with CRIT_ERR
      file << "RDMATLPP Read Failed\n";
      this->exerciser_fatal_text = "RDMATLPP Read Failed: EXERCISER.cpp";
      file.close();
      return CRIT_ERR;
    }

    // Performance is calculated the following way:
    // tlp_size*4*tlp_count = total bits transferred
    // Multiply by the link width multiplier and divide by the total number of trn_clks to get mbps
    temp_rd_mbps = (amdexerciser_descriptors.rd_tlp_size*4*amdexerciser_descriptors.num_rd_tlps*8*link_width_multiplier)/(trn_clks);
    // add the calculated RD performance to the current value.  Average found later by dividing by total iterations
    this->rd_mbps=temp_rd_mbps+this->rd_mbps;
    file << "Read TRN clocks = " << trn_clks <<"\n";
    file << "Read DMA Performance =" <<temp_rd_mbps<<"\n";
  }

  //close out pointers and open files
  file.close();
  delete gReadData;
  delete gWriteData;
  delete mtrandPtrWrsize;
  delete mtrandPtrRdsize;
  delete mtrandPtrWrcount;
  delete mtrandPtrRdcount;
  delete mtrandMode;
  delete mtrandPattern;
  delete mtrandRdPhantom;
  delete mtrandWrPhantom;
  delete mtrandTransStreaming;
  return SUCCESS;

} // END run_amdexerciser

//--- get_rd_mbps(): Returns the read performance as a char*
//--- Arguments: Takes in iteration count
//--- Return Value: Returns char* containing read performance
//--- Detailed Description: This module does the following:
//---                       1) Takes the rd_mbps, which is a tallied performance over all iterations and divides it
//---                          by the iteration count to get total average Read performance
//---                          register to start the transfer
//---                       2) Returns the resulting average performance after converting it to char*.  We convert to
//---                          char* because the argument to update the GUI text box takes a char*
const char* exerciser_t::get_rd_mbps(int iter_count){
    snprintf(this->rd_mbps_c, sizeof(this->rd_mbps_c), "%d",(rd_mbps/iter_count));
  return this->rd_mbps_c;
}

//--- get_wr_mbps(): Returns the read performance as a char*
//--- Arguments: Takes in iteration count
//--- Return Value: Returns char* containing read performance
//--- Detailed Description: This module does the following:
//---                       1) Takes the wr_mbps, which is a tallied performance over all iterations and divides it
//---                          by the iteration count to get total average Read performance
//---                          register to start the transfer
//---                       2) Returns the resulting average performance after converting it to char*.  We convert to
//---                          char* because the argument to update the GUI text box takes a char*
const char* exerciser_t::get_wr_mbps(int iter_count){
    snprintf(this->wr_mbps_c, sizeof(this->wr_mbps_c),"%d",(wr_mbps/iter_count));
  return this->wr_mbps_c;
}

//--- get_rd_success(): Returns bool stating whether the rd DMA was successful
//--- Arguments: None
//--- Return Value: Returns boolean
//--- Detailed Description: This module does the following:
//---                       1) Returns boolean stating RD success or failure
bool exerciser_t::get_rd_success(void){
  return this->rd_success;
}

//--- get_wr_success(): Returns bool stating whether the rd DMA was successful
//--- Arguments: None
//--- Return Value: Returns boolean
//--- Detailed Description: This module does the following:
//---                       1) Returns boolean stating WR success or failure
bool exerciser_t::get_wr_success(void){
  return this->wr_success;
}

//--- get_wr_result_text(): Returns text stating details or WR result transfer
//--- Arguments: None
//--- Return Value: char*
//--- Detailed Description: This module does the following:
//---                       1) Returns char* providing WR details
const char* exerciser_t::get_wr_result_text(void){
  return this->wr_result_text;
}

//--- get_wr_result_text(): Returns text stating details or RD result transferful
//--- Arguments: None
//--- Return Value: Returns char*
//--- Detailed Description: This module does the following:
//---                       1) Returns char* providing RD details
const char* exerciser_t::get_rd_result_text(void){
  return this->rd_result_text;
}

struct RegValueRead
{
    XbmDmaControlReg reg;
    const char *name;
};
static RegValueRead gRegReads[] = {
    {Reg_DeviceCS, "Device Control Status"},
    {Reg_DeviceDMACS, "DMA Control Status"},
    {Reg_WriteTlpAddress, "Write Tlp Address"},
    {Reg_WriteTlpSize, "Write Tlp Size"},
    {Reg_WriteTlpCount, "Write Tlp Count"},
    {Reg_WriteTlpPattern, "Write Tlp Pattern"},
    {Reg_ReadTlpPattern, "Read Tlp Pattern"},
    {Reg_ReadTlpAddress, "Read Tlp Address"},
    {Reg_ReadTlpSize, "Read Tlp Size"},
    {Reg_ReadTlpCount, "Read Tlp Count"},
    {Reg_WriteDMAPerf, "Write DMA Perf"},
    {Reg_ReadDMAPerf, "Read DMA Perf"},
    {Reg_ReadComplStatus, "Read Completion Status"},
    {Reg_ComplWithData, "Completion With Data"},
    {Reg_ComplSize, "Completion Size"},
    {Reg_DeviceLinkWidth, "Device Link Width"},
    {Reg_DeviceLinkTlpSize, "Device Link Tlp Size"},
    {Reg_DeviceMiscControl, "Device Misc Control"},
    {Reg_DeviceMSIControl, "Device MSI Control"},
    {Reg_DeviceDirectedLinkChange, "Device Directed Link Change"},
    {Reg_DeviceFCControl, "Device FC Control"},
    {Reg_DeviceFCPostedInfo, "Device FC Posted Info"},
    {Reg_DeviceFCNonPostedInfo, "Device FC Non-Posted Info"},
    {Reg_DeviceFCCompletionInfo, "Device FC Completion Info"},
};

#define DO_READ_REG(i) { \
    RegValueRead reader = gRegReads[i]; \
      unsigned int reg_value = reader.reg; \
        if (ioctl(g_devFile, XBMD_IOC_RD_BMD_REG, &reg_value) < 0) { \
        exerciser_file << reader.name << " read failed" << endl; \
        return CRIT_ERR; \
    } else { \
            exerciser_file << reader.name << " = 0x" << hex << reg_value << endl; \
    } \
}

//--- read_exerciser_regs(): Reads AMDEXERCISER regs and outputs to exerciser_regs.txt file
//--- Arguments:  int device file number
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads all the AMDEXERCISER descriptor registers and outputs those values to a text file
//---                          so that it can be displayed in the GUI under the Read_EXERCISER tab
  int exerciser_t::read_exerciser_regs(int g_devFile) {
    ofstream exerciser_file;

    // Open exerciser_regs.txt.  If FILEIO occurs, print to log file the error and also update the main status bar.
    //  then return CRIT_ERR
    exerciser_file.open ("exerciser_regs.txt");
    if (!exerciser_file.is_open()) {
      exerciser_file << "Error opening log exerciser_file";
      this->exerciser_fatal_text = "Error opening log exerciser_file: EXERCISER.cpp";
      return CRIT_ERR;
    }

    // Switch statement reads AMDEXERCISER descriptor register values sequentially and outputs to log file.  We use a switch
    // statement so we can give actual descriptor register names rather than solely offsets from base.
    exerciser_file <<endl<<"*** AMDEXERCISER Register Values ***\n"<<endl;
    DO_READ_REG(0);
    DO_READ_REG(1);
    DO_READ_REG(2);
    DO_READ_REG(3);
    DO_READ_REG(4);
    DO_READ_REG(5);
    DO_READ_REG(6);
    DO_READ_REG(7);
    DO_READ_REG(8);
    DO_READ_REG(9);
    DO_READ_REG(10);
    DO_READ_REG(11);
    DO_READ_REG(12);
    DO_READ_REG(13);
    DO_READ_REG(14);
    DO_READ_REG(15);
    DO_READ_REG(16);
    DO_READ_REG(17);
    DO_READ_REG(18);
    DO_READ_REG(19);
    exerciser_file <<"*** End AMDEXERCISER Register Space ***\n"<<endl;
    exerciser_file.close();
    return SUCCESS;
  }

#undef DO_READ_REG

