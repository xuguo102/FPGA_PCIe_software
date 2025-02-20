#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "../driver/amd_pcie_exerciser.h"

/*#define TRANSFER_SIZE 1024*1024*/
#define TRANSFER_SIZE 1024

/*#define ReadTestpattern   0x5a5a5a5a*/
#define ReadTestpattern   0xdeadbeef
/*#define WriteTestpattern  0xdeadbeef*/
#define WriteTestpattern (1 << 31) | ((2 & 0xFF) << 16) | ((0 & 0x1F) << 11) | ((0 & 0x7) << 8) | (0 & 0xfffffffc)

/*
// for PCI
address = (1 << 31) | ((1 & 0xFF) << 16) | ((0 & 0x1F) << 11) | ((0 & 0x7) << 8) | (00 & 0xfffffffc);

// write cfg register
IoWrite8(0xcf8, address);
// read data register
uint32_t data8  = IoRead8(0xcfc);
uint32_t data16 = IoRead16(0xcfc);
uint32_t data32 = IoRead32(0xcfc);
*/

struct Config
{
	// NOTE(matthew): Config offsets
	uint32_t pmOffset;
	uint32_t msiOffset;
	uint32_t pcieCapOffset;
	uint32_t deviceCapOffset;
	uint32_t deviceStatContOffset;
	uint32_t linkCapOffset;
	uint32_t linkStatContOffset;

	// NOTE(matthew): Register values
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
        if (ioctl(g_devFile, XBMD_IOC_RD_BMD_REG, &reg_value) < 0) { \
        printf(" reader.name %s read failed \n", reader.name); \
        return -1; \
    } else { \
            printf(" reader.name %s value 0x%08x \n",reader.name, reg_value); \
    } \
}

//--- read_bmd_regs(): Reads XBMD regs and outputs to bmd_regs.txt file
//--- Arguments:  int device file number
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads all the XBMD descriptor registers and outputs those values to a text file
//---                          so that it can be displayed in the GUI under the Read_BMD tab
int read_bmd_regs(int g_devFile) {

    // Switch statement reads XBMD descriptor register values sequentially and outputs to log file.  We use a switch
    // statement so we can give actual descriptor register names rather than solely offsets from base.
    printf("*** XBMD Register Values ***\n");
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
    printf("*** End XBMD Register Space ***\n");
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
        if (ioctl(g_devFile, XBMD_IOC_RD_CFG_REG, &reg_value) < 0) {

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

	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
		printf("IOCTL failed reading a config reg\n");
	} else {
		nextCapOffset = nextCapOffset & 0xFF;
	}

	do {
		currCapOffset = nextCapOffset;
		if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
			printf("IOCTL failed reading a config reg\n");
		} else {
			capId = nextCapOffset & 0xFF;
			nextCapOffset = (nextCapOffset & 0xFF00) >> 8;
		}

		switch (capId) {
			case 1: {
					// NOTE(matthew): Power Management Capability
					config->pmOffset = currCapOffset;
				} break;

			case 5: {
					// NOTE(matthew): MSI Capability
					config->msiOffset = currCapOffset;
				} break;

			case 16: {
					// NOTE(matthew): PCI Express Capability
					config->pcieCapOffset = currCapOffset;
					config->deviceCapOffset = currCapOffset + 4;
					config->deviceStatContOffset = currCapOffset + 8;
					config->linkCapOffset = currCapOffset + 12;
					config->linkStatContOffset = currCapOffset + 16;
				} break;

			default: {
					printf("Read Capability is not valid\n");
				} break;
		}
	} while (nextCapOffset != 0);
}

void update_config(struct Config *config, int fd)
{
	uint32_t regValue = 0;

	regValue = config->pmOffset;
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading power management capabilities\n");
	} else {
		config->pmCapabilities = regValue >> 16;
	}

	regValue = config->pmOffset + 4;
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading PM status/control\n");
	} else {
		config->pmStatControl = regValue & 0xFFFF;
	}

	regValue = config->msiOffset;
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading MSI Control\n");
	} else {
		config->msiControl = regValue >> 16;
	}

	regValue = config->linkCapOffset;
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading Link Cap offset\n");
	} else {
		config->linkWidthCap = (regValue >> 4) & 0x3F;
		config->linkSpeedCap = regValue & 0xF;
	}

	regValue = config->linkStatContOffset;
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
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

	char *gen = 0;
    uint32_t timeout = 1000;
    uint32_t cfg_space_io_address = (2<<16)|(0<<11)|(0<<8);
    printf("***Cfg Space address : 0x%08x *****\n", cfg_space_io_address);

	fd = open("/dev/amdpcieexerciser", O_RDWR);
	if (fd <= 0) {
		printf("Could not open file /dev/amdpcieexerciser\n");
		return -1;
	}

	if (argc > 1) {
		doRead = true;
		printf("*** Do Read true, argc : %d ***\n", argc);
	}

	get_capabilities(&config, fd);
	update_config(&config, fd);

	//cfg_read_regs(fd);

	writeBuffer = (uint32_t*)malloc(sizeof(uint32_t) * TRANSFER_SIZE);
	readBuffer  = (uint32_t*)malloc(sizeof(uint32_t) * TRANSFER_SIZE);

	for (i = 0; i < TRANSFER_SIZE; ++i) {
		writeBuffer[i] = (i + 1) * 7;
	}

	for (i = 0; i < TRANSFER_SIZE; ++i) {
		readBuffer[i] = i;
	}

	regValue = config.deviceStatContOffset;
	maxPayloadSize = 32;
	tlpSizeMax = 0;

	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_RD_CFG_REG);
		return -1;
	} else {
		maxPayloadSize = (regValue & 0x000000E0) >> 5;
		printf("Max payload size: %d \n", maxPayloadSize);
	}

	switch (maxPayloadSize) {
		// NOTE(matthew): 128 maxPayloadSize;
		case 0: { tlpSizeMax = 5; maxPayloadSize = 128; } break;
			// NOTE(matthew): 256 maxPayloadSize;
		case 1: { tlpSizeMax = 6; maxPayloadSize = 256; } break;
			// NOTE(matthew): 512 maxPayloadSize;
		case 2: { tlpSizeMax = 7; maxPayloadSize = 512; } break;
			// NOTE(matthew): 1024 maxPayloadSize;
		case 3: { tlpSizeMax = 8; maxPayloadSize = 1024; } break;
			// NOTE(matthew): 2048 maxPayloadSize;
		case 4: { tlpSizeMax = 9; maxPayloadSize = 2048; } break;
			// NOTE(matthew): 4096 maxPayloadSize;
		case 5: { tlpSizeMax = 10; maxPayloadSize = 4096; } break;
		default: { printf("Max payload size is invalid\n"); } break;
	}

	printf("Max payload: %d, tlp max size: %d \n", maxPayloadSize, tlpSizeMax);

	/*writeTLPSize = maxPayloadSize;*/
	writeTLPSize = 1;
	/*writeTLPCount = TRANSFER_SIZE/maxPayloadSize;*/
	writeTLPCount = TRANSFER_SIZE;
	/*readTLPSize = maxPayloadSize;*/
	readTLPSize = 1;
	readTLPCount = TRANSFER_SIZE;

	// NOTE(matthew): Copy data to kernel
	write_data(fd, TRANSFER_SIZE * sizeof(*writeBuffer), writeBuffer);
	printf("Data copied to kernel\n");

	// NOTE(matthew): Setup DMA
	if (ioctl(fd, XBMD_IOC_RESET, 0) < 0) {
		printf("IOCTL failed setting reset\n");
		return -1;
	} else {
		printf("DMA Reset\n");
	}

	if (ioctl(fd, XBMD_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
		printf("IOCTL failed reading DMA Control\n");
		return -1;
	} else {
		printf("DMA Control: 0x%08X \n", dmaControlReg);
	}

#if 1
    if (ioctl(fd, XBMD_IOC_WRITE_WR_PTRN, WriteTestpattern) < 0) {
        printf("IOCTL failed setting the write TLP Pattern\n");
        return -1;
    }
#endif

    if (ioctl(fd, XBMD_IOC_WRITE_WR_COUNT, writeTLPCount) < 0) {
        printf("IOCTL failed setting the write TLP count\n");
        return -1;
    }

    if (ioctl(fd, XBMD_IOC_WRITE_WR_LEN, writeTLPSize) < 0) {
        printf("IOCTL failed setting the write TLP size\n");
    }

    if (ioctl(fd, XBMD_IOC_READ_WR_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP size\n");
        return -1;
    } else {
        printf("Write TLP size: %u, expected  %u \n", regValue, writeTLPSize);
    }

#if 1
    if (ioctl(fd, XBMD_IOC_READ_WR_PTRN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP Pattern\n");
        return -1;
    } else {
        printf("Write TLP Pattern: 0x%08x, expected  0x%08x \n", regValue, WriteTestpattern);
    }
#endif


#if 0
    if (ioctl(fd, XBMD_IOC_WRITE_RD_PTRN, ReadTestpattern) < 0) {
        printf("IOCTL failed setting the Read TLP Pattern\n");
        return -1;
    }
    if (ioctl(fd, XBMD_IOC_READ_WR_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP size\n");
        return -1;
    } else {
        printf("Write TLP size: %u, expected  %u \n", regValue, writeTLPSize);
    }

    if (ioctl(fd, XBMD_IOC_WRITE_RD_COUNT, readTLPCount) < 0) {
        printf("IOCTL failed setting the read TLP count\n");
        return -1;
    }

    if (ioctl(fd, XBMD_IOC_WRITE_RD_LEN, readTLPSize) < 0) {
        printf("IOCTL failed setting the read TLP size\n");
        return -1;
    }

    if (ioctl(fd, XBMD_IOC_READ_RD_PTRN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP Pattern\n");
        return -1;
    } else {
        printf("Read TLP Pattern: 0x%08x, expected  0x%08x \n", regValue, ReadTestpattern);
    }

    if (ioctl(fd, XBMD_IOC_READ_RD_COUNT, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP count\n");
        return -1;
    } else {
        printf("Read TLP count: %u, expected  %u \n", regValue, readTLPCount);
    }

    if (ioctl(fd, XBMD_IOC_READ_RD_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP size\n");
        return -1;
    } else {
        printf("Read TLP size: %u, expected %u \n", regValue, readTLPSize);
    }
#endif

	writeWRRCount = 0; // 1;
	readWRRCount = 0; //1;
	// 0x01010000 or 0x01010020
	miscControl = (writeWRRCount << 24) | (readWRRCount << 16);

	if (ioctl(fd, XBMD_IOC_WRITE_MISC_CTL, miscControl) < 0) {
		printf("IOCTL failed setting misc control\n");
		return -1;
	}

	readEnable = doRead ? 1 : 0;
	if (readEnable) {
		printf("*************** readEnable ****************\n");
	}
	writeEnable = doRead ? 0 : 1;
	if (writeEnable) {
		printf("*************** writeEnable ****************\n");
	}

    dmaControlReg |= (readEnable << 16) | writeEnable;
    /*dmaControlReg |= (1 << 16) | 1;*/
    dmaControlReg &= ~((1 << 7) | (1 << 23));
	if (ioctl(fd, XBMD_IOC_WRITE_DMA_CTRL, dmaControlReg) < 0) {
		printf("IOCTL failed setting DMA control\n");
		return -1;
	}

#if 0
	usleep(1000000);
#else
	do {
		usleep(1000);
		if (ioctl(fd, XBMD_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
			printf("IOCTL failed reading DMA control\n");
			return -1;
		}
		printf("*** dmaControlReg : 0x%08x ***\n", dmaControlReg);
	} while ((!(dmaControlReg & (1 << 24) >> 24) &&
			!(dmaControlReg & (1 << 8) >> 8)) && timeout--);
#endif

	if (ioctl(fd, XBMD_IOC_READ_DMA_CTRL, &regValue) < 0) {
		printf("IOCTL failed reading DMA control\n");
		return -1;
	} else {
		dmaControlReg = regValue;
		dmaControlWrite = dmaControlReg & 0x0000FFFF;
		dmaControlRead = (dmaControlReg & 0xFFFF0000) >> 16;
	}

	/*read_bmd_regs(fd);*/

	read_data(fd, writeTLPSize * writeTLPCount * sizeof(uint32_t), readBuffer);
	printf("Data copied from kernel\n");

	// NOTE(matthew): Read error check
	if (readEnable) {
		if ((dmaControlRead & 0x1111) != 0x0101) {
			printf("DMA Read did not complete succesfully, 0x%04X \n",
					dmaControlRead);
			return -1;
		} else {
			printf("DMA Read success!\n");
		}
	}

	// NOTE(matthew): Write error check
	if (writeEnable) {
		for (i = 0; i < (writeTLPSize * writeTLPCount); ++i) {
			readData = readBuffer[i];
            /*writeData = writeBuffer[i];*/
            printf("########: Read index %d, read value : 0x%08x ########## \n", i ,readData);
            if (WriteTestpattern != readData) {
			/*if (writeData != readData) {*/
				printf("Mismatch: wrote index %d, wrote value : 0x%08x, got value: 0x%08x \n", i, WriteTestpattern, readData);
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
	if (ioctl(fd, XBMD_IOC_RD_CFG_REG, &regValue) < 0) {
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
		if ((regValue & 0x00080000) == 0x00080000) {
			printf("Unsupported Request Detected device\n");
		}
	}

    read_bmd_regs(fd);
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
					case 1: { linkWidthMultiplier = 500; } break;
					case 2: { linkWidthMultiplier = 1000; } break;
					case 4: { linkWidthMultiplier = 2000; } break;
					case 8: { linkWidthMultiplier = 4000; } break;
					default: { printf("%s: Link width : %d is not valid \n", gen, config.linkWidth); } break;
				}
			} break;


		default: {
				 printf("Link speed is not valid : %d \n", config.linkSpeed);
			 } break;
	}

	if (writeEnable) {
		if (ioctl(fd, XBMD_IOC_READ_WR_PERF, &trnClks) < 0) {
			printf("IOCTL failed reading write performance\n");
			return -1;
		}

		tempWrMbps = (writeTLPSize * 4 * writeTLPCount * linkWidthMultiplier) / trnClks;
		printf("DMA Write TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempWrMbps);
	}

	if (readEnable) {
		if (ioctl(fd, XBMD_IOC_READ_RD_PERF, &trnClks) < 0) {
			printf("IOCTL failed reading read performance\n");
			return -1;
		}

		tempRdMbps = (readTLPSize * 4 * readTLPCount * linkWidthMultiplier) / trnClks;
		printf("DMA Read TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempRdMbps);
	}

	// NOTE(matthew): Setup DMA
	if (ioctl(fd, XBMD_IOC_RESET, 0) < 0) {
		printf("IOCTL failed setting reset\n");
		return -1;
	} else {
		printf("DMA Reset\n");
	}
	free(writeBuffer);
	free(readBuffer);
	close(fd);

	return 0;
}
