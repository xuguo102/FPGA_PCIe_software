#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "../../driver_h10/amd_pcie_exerciser.h"

#define TANSFER_SIZE 1024

/*ifdef MalformTLP_TEST*/

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


	uint32_t writeTLPSize = 0;
	uint32_t readTLPSize = 0;
	uint32_t testpattern = 0xfeadbeef;

	uint32_t dmaControlReg = 0;

	uint32_t readEnable = 0;
	uint32_t writeEnable = 0;

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

    // Check parameters
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: %s <parameter1> <parameter2>\n\n", argv[0]);
        printf("This program requires two parameters to execute:\n");
        printf("  <parameter1>  -r/R read : -W/w write\n");
        printf("  <parameter2>  Transfer size \n\n");
        printf("Options:\n");
        printf("  -h, --help    Show this help message\n\n");
        printf("Example:\n");
        printf("  %s 100 input.txt\n", argv[0]);
        return 0;
    }

    //  Verify parameters couter
    if (argc < 2) {
        fprintf(stderr, "Error: Invalid number of parameters\n");
        fprintf(stderr, "Use '%s -h' for usage information\n", argv[0]);
        return 1;
    }

    if (argc == 2 && (strcmp(argv[1], "-R") == 0 || strcmp(argv[1], "-r") == 0)) {
		doRead = true;
        readEnable = true;
		printf("Do DMA Read .\n");
	}

    if (argc == 2 && (strcmp(argv[1], "-W") == 0 || strcmp(argv[1], "-w") == 0)) {
		doRead = false;
        writeEnable = true;
		printf(" Do DMA Write .\n");
    }

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

	// NOTE(michiel): Copy data to kernel, clear R/W buffer.
	write_data(fd, TANSFER_SIZE * sizeof(*writeBuffer), writeBuffer);
	printf("Data copied to kernel\n");

	// NOTE(michiel): Setup DMA
	if (ioctl(fd, PBE_IOC_WRITE_WR_PTRN, testpattern) < 0) {
		printf("IOCTL failed setting the write TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_LEN, TANSFER_SIZE) < 0) {
		printf("IOCTL failed setting the write TLP size\n");
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_LEN, TANSFER_SIZE) < 0) {
		printf("IOCTL failed setting the read TLP size\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_READ_RD_LEN, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP size\n");
		return -1;
	} else {
		printf("Write TLP size: %u, expected  %u \n", regValue, TANSFER_SIZE);
	}

	if (ioctl(fd, PBE_IOC_READ_WR_LEN, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP size\n");
		return -1;
	} else {
		printf("Write TLP size: %u, expected  %u \n", regValue, TANSFER_SIZE);
	}

    if (writeEnable) {
        dmaControlReg = 0x4;
        if (ioctl(fd, PBE_IOC_WRITE_WRITE_DMA_CTRL, dmaControlReg) < 0) {
            printf("IOCTL failed setting the write TLP count\n");
            return -1;
        }

        if (ioctl(fd, PBE_IOC_READ_WRITE_DMA_CTRL, &dmaControlReg) < 0) {
            printf("IOCTL failed reading DMA Control\n");
            return -1;
        } else {
            printf("DMA Control: 0x%08X \n", dmaControlReg);
        }

    }

    if (readEnable) {
        dmaControlReg = 0x4;
        if (ioctl(fd, PBE_IOC_WRITE_READ_DMA_CTRL, dmaControlReg) < 0) {
            printf("IOCTL failed setting the write TLP count\n");
            return -1;
        }

        if (ioctl(fd, PBE_IOC_READ_READ_DMA_CTRL, &dmaControlReg) < 0) {
            printf("IOCTL failed reading DMA Control\n");
            return -1;
        } else {
            printf("DMA Control: 0x%08X \n", dmaControlReg);
        }

    }
	read_data(fd, TANSFER_SIZE * sizeof(uint32_t), readBuffer);

	printf("Data copied from kernel\n");

#if 0
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
#endif

	free(writeBuffer);
	free(readBuffer);
    close(fd);

	return 0;
}
