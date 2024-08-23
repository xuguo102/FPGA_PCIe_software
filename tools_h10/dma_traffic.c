#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "../driver_vpk120/amd_pcie_exerciser.h"

/*#define TANSFER_SIZE 5*1024*1024*/
/*#define TANSFER_SIZE 100*1024*/
#define TANSFER_SIZE 1024

/*ifdef MalformTLP_TEST*/
/*#define AER_TEST 1*/

struct Config
{
	// NOTE(michiel): Config offsets
	uint32_t pmOffset;
	uint32_t msiOffset;
	uint32_t msixOffset;
	uint32_t pcieCapOffset;
	uint32_t deviceCapOffset;
	uint32_t deviceStatContOffset;
	uint32_t linkCapOffset;
	uint32_t linkStatContOffset;

	// NOTE(michiel): Register values
	uint32_t linkWidthCap;
	uint32_t linkSpeedCap;
	uint32_t linkWidth;
	uint32_t linkSpeed;
	uint32_t linkControl;
	uint32_t pmStatControl;
	uint32_t pmCapabilities;
	uint32_t msiControl;
};

struct RegValueRead
{
    XbmDmaControlReg reg;
    const char *name;
};

static struct RegValueRead gRegReads[] = {
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
    struct RegValueRead reader = gRegReads[i]; \
      unsigned int reg_value = reader.reg; \
        if (ioctl(g_devFile, PBE_IOC_RD_BMD_REG, &reg_value) < 0) { \
        printf(" reader.name %s read failed \n", reader.name); \
        return -1; \
    } else { \
            printf(" reader.name %s value 0x%08x \n",reader.name, reg_value); \
    } \
}

//--- read_bmd_regs(): Reads PBE regs and outputs to bmd_regs.txt file
//--- Arguments:  int device file number
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads all the PBE descriptor registers and outputs those values to a text file
//---                          so that it can be displayed in the GUI under the Read_BMD tab
int read_bmd_regs(int g_devFile) {

    // Switch statement reads PBE descriptor register values sequentially and outputs to log file.  We use a switch
    // statement so we can give actual descriptor register names rather than solely offsets from base.
    printf("*** PBE Register Values ***\n");
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
    printf("*** End PBE Register Space ***\n");
    return 1;
  }

#undef DO_READ_REG


//--- read_cfg_regs(): Reads Endpoint Configuration register and outputs contents to cfg_space.txt.
//--- Arguments: int: endpoint device file
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads Endpoint Configuration register and outputs contents to cfg_space.txt.
int cfg_read_regs(int g_devFile) {
    int ii;
    unsigned int reg_value = 0;

    //Read CFG space sequentially and output to text file.
    // Possible future enhancement would be to associate each offset with specific CFG space name
    printf("*** EP Device Type 0 Configuration Space ***\n");
    // Read CFG space up to 120H
    for (ii = 0; ii <0x120;ii=ii+4) {
        reg_value = ii;
        if (ioctl(g_devFile, PBE_IOC_RD_CFG_REG, &reg_value) < 0) {

            // If CFG read fails - state failure in file
            printf("ERROR: Could not read CFG Register 0x%x Verify device is present and driver loaded correctly\n", ii);

            // return failure condition
            return -1;
        } else {
            // read is successful so update file

            switch (ii) {

                case 0: // 128 MPS
                    printf("Device ID/Vendor ID : 0x%08x \n",reg_value);
                    break;
                case 4:  // 256 MPS
                    printf("Status/Command : 0x%08x \n",reg_value);
                    break;
                case 8:  // 512 MPS
                    printf("Class Code/Revision ID : 0x%08x \n",reg_value);
                    break;
                case 12:  // 1024 MPS
                    printf("BIST/Header Type/Lat. Timer/Cache Line size : 0x%08x \n",reg_value);
                    break;
                case 16: // 2048 MPS
                    printf("BAR0 : 0x%08x \n",reg_value);
                    break;
                case 20: // 4096 MPS
                    printf("BAR1 : 0x%08x \n",reg_value);
                    break;
                case 24: // 4096 MPS
                    printf("BAR2 : 0x%08x \n",reg_value);
                    break;
                case 28: // 4096 MPS
                    printf("BAR3 : 0x%08x \n",reg_value);
                    break;
                case 32: // 4096 MPS
                    printf("BAR4 : 0x%08x \n",reg_value);
                    break;
                case 36: // 4096 MPS
                    printf("BAR5 : 0x%08x \n",reg_value);
                    break;
                case 40: // 4096 MPS
                    printf("CIS Pointer : 0x%08x \n",reg_value);
                    break;
                case 44: // 4096 MPS
                    printf("Subsystem ID/ Subsystem Vendor ID : 0x%08x \n",reg_value);
                    break;
                case 48: // 4096 MPS
                    printf("Expansion ROM Base Address : 0x%08x \n",reg_value);
                    break;
                case 52: // 4096 MPS
                    printf("Reserved/Cap. Pointer : 0x%08x \n",reg_value);
                    break;
                case 56: // 4096 MPS
                    printf("Reserved : 0x%08x \n",reg_value);
                    break;
                case 60: // 4096 MPS
                    printf("Max Lat/Min Gnt/INT Pin/ INT Line : 0x%08x \n",reg_value);
                    break;
                default: {
                             printf("Arddess offset : %d value 0x%08x \n", ii, reg_value);  // fall through
                    break;
                         }
            }
        }
    }
    printf("*** End Device Configuration Space ***\n");
    return 1;
} // END read_cfg_regs


int write_data(int file, uint32_t size, const void *buffer)
{
	return write(file, buffer, size);
}

int read_data(int file, uint32_t size, void *buffer)
{
	return read(file, buffer, size);
}

void get_capabilities(struct Config *config, int fd)
{
	int32_t nextCapOffset = 0x34;
	int32_t currCapOffset = 0;
	int32_t capId = 0;

	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
		printf("IOCTL failed reading a config reg\n");
	} else {
		nextCapOffset = nextCapOffset & 0xFF;
	}

	do {
		currCapOffset = nextCapOffset;
		if (ioctl(fd, PBE_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
			printf("IOCTL failed reading a config reg\n");
		} else {
			capId = nextCapOffset & 0xFF;
			nextCapOffset = (nextCapOffset & 0xFF00) >> 8;
		}

		switch (capId) {
			case 1: {
					// NOTE(michiel): Power Management Capability
					config->pmOffset = currCapOffset;
                    printf("*************** Power Management Capability , CapID is 0x%08x . ****************\n", capId);
				} break;

			case 5: {
					// NOTE(michiel): MSI Capability
					config->msiOffset = currCapOffset;
                    printf("*************** MSI Capability , CapID is 0x%08x . ****************\n", capId);
				} break;

			case 17: {
					// NOTE(michiel): MSI Capability
                    config->msixOffset = currCapOffset;
                    printf("*************** MSI-x Capability , CapID is 0x%08x . ****************\n", capId);
				} break;

			case 16: {
					// NOTE(michiel): PCI Express Capability
                    printf("*************** PCI Express Capability , CapID is 0x%08x . ****************\n", capId);
					config->pcieCapOffset = currCapOffset;
					config->deviceCapOffset = currCapOffset + 4;
					config->deviceStatContOffset = currCapOffset + 8;
					config->linkCapOffset = currCapOffset + 12;
					config->linkStatContOffset = currCapOffset + 16;
				} break;

			default: {
					printf("Read Capability is not valid, CapID is 0x%08x .\n", capId);
				} break;
		}
	} while (nextCapOffset != 0);
}

void update_config(struct Config *config, int fd)
{
	uint32_t regValue = 0;

	regValue = config->pmOffset;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading power management capabilities\n");
	} else {
		config->pmCapabilities = regValue >> 16;
	}

	regValue = config->pmOffset + 4;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading PM status/control\n");
	} else {
		config->pmStatControl = regValue & 0xFFFF;
	}

	regValue = config->msiOffset;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading MSI Control\n");
	} else {
		config->msiControl = regValue >> 16;
	}

	regValue = config->linkCapOffset;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading Link Cap offset\n");
	} else {
		config->linkWidthCap = (regValue >> 4) & 0x3F;
		config->linkSpeedCap = regValue & 0xF;
	}

	regValue = config->linkStatContOffset;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading Link control\n");
	} else {
		config->linkControl = regValue & 16;
		config->linkSpeed = (regValue >> 16) & 0xF;
		config->linkWidth = (regValue >> 20) & 0x3F;
	}
}

int main(int argc, char **argv)
{

	int fd = 0;
    uint32_t i = 0;
	bool doRead = false;
	struct Config config;
    uint32_t *writeBuffer = NULL;
    uint32_t *readBuffer  = NULL;

	uint32_t regValue = 0;
	uint32_t maxPayloadSize = 0;
	uint32_t tlpSizeMax = 0;


	uint32_t writeTLPSize = 0;
	uint32_t writeTLPCount = 0;
	uint32_t readTLPSize = 0;
	uint32_t readTLPCount = 0;
	uint32_t testpattern = 0xfeadbeef;

	uint32_t dmaControlReg = 0;

	uint32_t writeWRRCount = 0; // 1;
	uint32_t readWRRCount = 0; //1;
	uint32_t miscControl = 0;

	uint32_t readEnable = 0;
	uint32_t writeEnable = 0;

	uint32_t dmaControlWrite = 0;
	uint32_t dmaControlRead = 0;

    bool writeError = false;

    uint32_t readData = 0;
    uint32_t writeData = 0;

	int32_t linkWidthMultiplier = 0;
	int32_t trnClks = 0;
	int32_t tempWrMbps = 0;
	int32_t tempRdMbps = 0;

    uint32_t aer_regValue = 0;

	char *gen = 0;

	fd = open("/dev/amdpcieexerciser", O_RDWR);
	if (fd <= 0) {
		printf("Could not open file /dev/amdpcieexerciser\n");
		return -1;
	}

	if (argc > 1) {
		doRead = true;
        readEnable = true;
		printf("*************** Do Read, argc : %d ****************\n", argc);
	} else {
		doRead = false;
        writeEnable = true;
		printf("*************** Do Write, argc : %d ****************\n", argc);
    }

	read_bmd_regs(fd);
    return 1;
	get_capabilities(&config, fd);
	update_config(&config, fd);

	//cfg_read_regs(fd);

	writeBuffer = (uint32_t*)malloc(sizeof(uint32_t) * TANSFER_SIZE);
	readBuffer  = (uint32_t*)malloc(sizeof(uint32_t) * TANSFER_SIZE);

	for (i = 0; i < TANSFER_SIZE; ++i) {
		writeBuffer[i] = 0x5a5a5a5a;
		/*writeBuffer[i] = (i + 1) * 7;*/
	}

	for (i = 0; i < TANSFER_SIZE; ++i) {
		readBuffer[i] = 0xDEADBEAD;
	}

	regValue = config.deviceStatContOffset;
	maxPayloadSize = 512;
	tlpSizeMax = 512;

	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_RD_CFG_REG);
		return -1;
	} else {
		maxPayloadSize = (regValue & 0x000000E0) >> 5;
		printf("Max payload size: %d \n", maxPayloadSize);
	}

	switch (maxPayloadSize) {
		// NOTE(michiel): 128 maxPayloadSize;
		case 0: { tlpSizeMax = 5; maxPayloadSize = 128; } break;
			// NOTE(michiel): 256 maxPayloadSize;
		case 1: { tlpSizeMax = 6; maxPayloadSize = 256; } break;
			// NOTE(michiel): 512 maxPayloadSize;
		case 2: { tlpSizeMax = 7; maxPayloadSize = 512; } break;
			// NOTE(michiel): 1024 maxPayloadSize;
		case 3: { tlpSizeMax = 8; maxPayloadSize = 1024; } break;
			// NOTE(michiel): 2048 maxPayloadSize;
		case 4: { tlpSizeMax = 9; maxPayloadSize = 2048; } break;
			// NOTE(michiel): 4096 maxPayloadSize;
		case 5: { tlpSizeMax = 10; maxPayloadSize = 4096; } break;
		default: { printf("Max payload size is invalid\n"); } break;
	}

	printf("Max payload: %d, tlp max size: %d \n", maxPayloadSize, tlpSizeMax);


#ifdef MalformTLP_TEST
    //error injection for MalformTLP
    writeTLPSize = maxPayloadSize/2;
#else
    writeTLPSize = maxPayloadSize/4;
#endif
	writeTLPCount = TANSFER_SIZE/maxPayloadSize;
	readTLPSize = maxPayloadSize/4;
	readTLPCount = TANSFER_SIZE/maxPayloadSize;

	// NOTE(michiel): Copy data to kernel, clear R/W buffer. 
	write_data(fd, TANSFER_SIZE * sizeof(*writeBuffer), writeBuffer);
	printf("Data copied to kernel\n");

	// NOTE(michiel): Setup DMA
	if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
		printf("IOCTL failed setting reset\n");
		return -1;
	} else {
		printf("DMA Reset\n");
	}

/*#ifdef AER_TEST*/
#if 0
    aer_regValue = 11;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif

	if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
		printf("IOCTL failed reading DMA Control\n");
		return -1;
	} else {
		printf("DMA Control: 0x%08X \n", dmaControlReg);
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_PTRN, testpattern) < 0) {
		printf("IOCTL failed setting the write TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_COUNT, writeTLPCount) < 0) {
		printf("IOCTL failed setting the write TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_LEN, writeTLPSize) < 0) {
		printf("IOCTL failed setting the write TLP size\n");
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_PTRN, testpattern) < 0) {
		printf("IOCTL failed setting the read TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_COUNT, readTLPCount) < 0) {
		printf("IOCTL failed setting the read TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_LEN, readTLPSize) < 0) {
		printf("IOCTL failed setting the read TLP size\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_READ_WR_COUNT, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP count\n");
		return -1;
	} else {
		printf("Write TLP count: %u, expected  %u \n", regValue, writeTLPCount);
	}

	if (ioctl(fd, PBE_IOC_READ_WR_LEN, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP size\n");
		return -1;
	} else {
		printf("Write TLP size: %u, expected  %u \n", regValue, writeTLPSize);
	}

	if (ioctl(fd, PBE_IOC_READ_RD_COUNT, &regValue) < 0) {
		printf("IOCTL failed reading the read TLP count\n");
		return -1;
	} else {
		printf("Read TLP count: %u, expected  %u \n", regValue, readTLPCount);
	}

#if 0
	writeWRRCount = 0; // 1;
	readWRRCount = 0; //1;
	// 0x01010000 or 0x01010020
	miscControl = (writeWRRCount << 24) | (readWRRCount << 16);

	if (ioctl(fd, PBE_IOC_WRITE_MISC_CTL, miscControl) < 0) {
		printf("IOCTL failed setting misc control\n");
		return -1;
	}

#endif
#ifdef AER_TEST
    aer_regValue = 14;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif


	dmaControlReg |= (readEnable << 16) | writeEnable;
	if (ioctl(fd, PBE_IOC_WRITE_DMA_CTRL, dmaControlReg) < 0) {
		printf("IOCTL failed setting DMA control\n");
		return -1;
	}

#ifdef AER_TEST
    aer_regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif

#ifdef AER_TEST
    return 0;
#endif

#if 1
    usleep(100000);
#else
	do {
		usleep(1000);
		if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
			printf("IOCTL failed reading DMA control\n");
			return -1;
		}
		printf("*** dmaControlReg : 0x%08x ***\n", dmaControlReg);
	} while ((readEnable && (dmaControlReg & 0x01010000)) ||
			(writeEnable && (dmaControlReg & 0x00000101)));
#endif

	if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &regValue) < 0) {
		printf("IOCTL failed reading DMA control\n");
		return -1;
	} else {
		dmaControlReg = regValue;
		dmaControlWrite = dmaControlReg & 0x0000FFFF;
		dmaControlRead = (dmaControlReg & 0xFFFF0000) >> 16;
	}

	read_bmd_regs(fd);

	read_data(fd, writeTLPSize * writeTLPCount * sizeof(uint32_t), readBuffer);
	printf("Data copied from kernel\n");

	// NOTE(michiel): Read error check
	if (readEnable) {
        for (i = 0; i < (writeTLPSize * writeTLPCount); ++i) {
            readData = readBuffer[i];
            writeData = writeBuffer[i];
            /*printf("wrote %d, got %d \n", writeData, readData);*/
            if (readData != writeData) {
                printf("Number %d , Mismatch : wrote 0x%08x, got 0x%08x \n", i, writeData, readData);
                return -1;
            }
        }
		if ((dmaControlRead & 0x1111) != 0x0101) {
			printf("DMA Read did not complete succesfully, 0x%04X \n",
					dmaControlRead);
			return -1;
		} else {
			printf("DMA Read success!\n");
		}
	}

	// NOTE(michiel): Write error check
	if (writeEnable) {
		for (i = 0; i < (writeTLPSize * writeTLPCount); ++i) {
            readData = readBuffer[i];
            /*printf("Number %d , got data = 0x%08x \n", i, readData);*/
			if (readData != testpattern) {
				printf("Number %d , Mismatch: Expected 0xfeedbeef, but got 0x%08x \n", i, readData);
				writeError = true;
			}
		}

		if (!writeError) {
			if ((dmaControlWrite & 0x1111) != 0x0101) {
				printf("DMA Write did not complete succesfully, 0x%04X \n",
						dmaControlWrite);
				return -1;
			} else {
				printf("DMA Write success!\n");
			}
		}
	}

	regValue = config.deviceStatContOffset;
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("Device status read failed\n");
	} else {
		if ((regValue & 0x00040000) == 0x00040000) {
			printf("Fatal reported by device\n");
		}
		if ((regValue & 0x00020000) == 0x00020000) {
			printf("Non fatal reported by device\n");
		}
		if ((regValue & 0x00010000) == 0x00010000) {
			printf("Correctable reported by device\n");
		}
	}

	//read_bmd_regs(fd);
	switch (config.linkSpeed)
	{
		case 1: {
				gen = "Generation 1";
				switch (config.linkWidth)
				{
					case 1: { linkWidthMultiplier = 31; } break;
					case 2: { linkWidthMultiplier = 62; } break;
					case 4: { linkWidthMultiplier = 125; } break;
					case 8: { linkWidthMultiplier = 250; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;

		case 2: {
				gen = "Generation 2";
				switch (config.linkWidth)
				{
					case 1: { linkWidthMultiplier = 62; } break;
					case 2: { linkWidthMultiplier = 125; } break;
					case 4: { linkWidthMultiplier = 250; } break;
					case 8: { linkWidthMultiplier = 500; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;

		case 3: {
				gen = "Generation 3";
				switch (config.linkWidth)
				{
					case 1: { linkWidthMultiplier = 250; } break;
					case 2: { linkWidthMultiplier = 500; } break;
					case 4: { linkWidthMultiplier = 1000; } break;
					case 8: { linkWidthMultiplier = 2000; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;

		case 4: {
				gen = "Generation 4";
				switch (config.linkWidth)
				{
					case 1: { linkWidthMultiplier = 500; } break;
					case 2: { linkWidthMultiplier = 1000; } break;
					case 4: { linkWidthMultiplier = 2000; } break;
					case 8: { linkWidthMultiplier = 4000; } break;
					case 16: { linkWidthMultiplier = 8000; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;

        case 5: {
				gen = "Generation 5";
                printf("%s: Link width : %d \n", gen, config.linkWidth);
				switch (config.linkWidth)
				{
					case 1: { linkWidthMultiplier  = 31; } break;
					case 2: { linkWidthMultiplier  = 62; } break;
					case 4: { linkWidthMultiplier  = 125; } break;
					case 8: { linkWidthMultiplier  = 250; } break;
					case 16: { linkWidthMultiplier = 500; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;

		default: {
				 printf("Link speed is not valid : %d \n", config.linkSpeed);
			 } break;
	}

	if (writeEnable) {
		if (ioctl(fd, PBE_IOC_READ_WR_PERF, &trnClks) < 0) {
			printf("IOCTL failed reading write performance\n");
			return -1;
		}

		tempWrMbps = (writeTLPSize * 4 * writeTLPCount * linkWidthMultiplier) / trnClks;
		printf("DMA Write Size: %d Bytes \n", writeTLPSize * 4 * writeTLPCount);
		printf("DMA Write TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempWrMbps);
	}

	if (readEnable) {
		if (ioctl(fd, PBE_IOC_READ_RD_PERF, &trnClks) < 0) {
			printf("IOCTL failed reading read performance\n");
			return -1;
		}

		tempRdMbps = (readTLPSize * 4 * readTLPCount * linkWidthMultiplier) / trnClks;
		printf("DMA Read Size: %d Bytes \n", readTLPSize * 4 * readTLPCount);
		printf("DMA Read TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempRdMbps);
	}

	free(writeBuffer);
	free(readBuffer);
    close(fd);

	return 0;
}
