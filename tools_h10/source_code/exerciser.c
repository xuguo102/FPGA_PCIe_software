#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>

#include "../driver/amd_pcie_exerciser.h"

struct Config
{
    // NOTE(michiel): Config offsets
    uint32_t pmOffset;
    uint32_t msiOffset;
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

int write_data(int file, uint32_t size, const void *buffer)
{
    return write(file, buffer, size);
}

int read_data(int file, uint32_t size, void *buffer)
{
    return read(file, buffer, size);
}

void get_capabilities(struct Config *exerciserconfig, int fd)
{
    int32_t nextCapOffset = 0x34;
    int32_t currCapOffset = 0;
    int32_t capId = 0;

    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
        printf("IOCTL failed reading a exerciserconfig reg \n \n");
    } else {
        nextCapOffset = nextCapOffset & 0xFF;
    }

      do {
        currCapOffset = nextCapOffset;
        if (ioctl(fd, PBE_IOC_RD_CFG_REG, &nextCapOffset) < 0) {
            printf("IOCTL failed reading a exerciserconfig reg \n \n");
        } else {
            capId = nextCapOffset & 0xFF;
            nextCapOffset = (nextCapOffset & 0xFF00) >> 8;
        }

        switch (capId) {
            case 1: {
                // NOTE(michiel): Power Management Capability
                exerciserconfig->pmOffset = currCapOffset;
            } break;

            case 5: {
                // NOTE(michiel): MSI Capability
                exerciserconfig->msiOffset = currCapOffset;
            } break;

            case 16: {
                // NOTE(michiel): PCI Express Capability
                exerciserconfig->pcieCapOffset = currCapOffset;
                exerciserconfig->deviceCapOffset = currCapOffset + 4;
                exerciserconfig->deviceStatContOffset = currCapOffset + 8;
                exerciserconfig->linkCapOffset = currCapOffset + 12;
                exerciserconfig->linkStatContOffset = currCapOffset + 16;
            } break;

            default: {
                printf("Read Capability is not valid \n");
            } break;
        }
    } while (nextCapOffset != 0);
}

void update_exerciserconfig(struct Config *exerciserconfig, int fd)
{
    uint32_t regValue = 0;

    regValue = exerciserconfig->pmOffset;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading power management capabilities \n");
    } else {
        exerciserconfig->pmCapabilities = regValue >> 16;
    }

    regValue = exerciserconfig->pmOffset + 4;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading PM status/control \n");
    } else {
        exerciserconfig->pmStatControl = regValue & 0xFFFF;
    }

    regValue = exerciserconfig->msiOffset;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading MSI Control \n");
    } else {
        exerciserconfig->msiControl = regValue >> 16;
    }

    regValue = exerciserconfig->linkCapOffset;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading Link Cap offset \n");
    } else {
        exerciserconfig->linkWidthCap = (regValue >> 4) & 0x3F;
        exerciserconfig->linkSpeedCap = regValue & 0xF;
    }

    regValue = exerciserconfig->linkStatContOffset;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading Link control \n");
    } else {
        exerciserconfig->linkControl = regValue & 16;
        exerciserconfig->linkSpeed = (regValue >> 16) & 0xF;
        exerciserconfig->linkWidth = (regValue >> 20) & 0x3F;
    }
}

int main(int argc, char **argv)
{
    int fd = open("/dev/amdpcieexerciser", O_RDWR);
    if (fd <= 0) {
        printf("Could not open file /dev/amdpcieexerciser \n");
    }

    int32_t doRead = false;
    if (argc > 1) {
        doRead = true;
    }

    struct Config exerciserconfig;
    get_capabilities(&exerciserconfig, fd);

    update_exerciserconfig(&exerciserconfig, fd);

    uint32_t *writeBuffer = malloc(sizeof(uint32_t)*1024);
    uint32_t *readBuffer  = malloc(sizeof(uint32_t)*1024);

    for (uint32_t i = 0; i < 1024; ++i) {
        writeBuffer[i] = (i + 1) * 7;
    }

    for (uint32_t i = 0; i < 1024; ++i) {
        readBuffer[i] = 0xDEADBEAD;
    }

    uint32_t regValue = exerciserconfig.deviceStatContOffset;
    uint32_t maxPayloadSize = 32;
    uint32_t tlpSizeMax = 0;

#if 1
    // NOTE(michiel): Setup DMA
    if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
        printf("IOCTL failed setting reset \n");
    } else {
        printf("DMA Reset \n");
    }

    uint32_t timeout = 1000000;
    regValue = 0;
    do {
        if (ioctl(fd, PBE_IOC_READ_ERRORINJECT, &regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
        } else {
            printf("Error Register value : 0x%08x \n", regValue);
        }

        regValue = 0x23;
        if (ioctl(fd, PBE_IOC_WRITE_ERRORINJECT, regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
        } else {
            printf("Set Error Register value : 0x%08x \n", regValue);
        }


        if (ioctl(fd, PBE_IOC_READ_ERRORINJECT, &regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
        } else {
            printf("Get Error Register value : 0x%08x \n", regValue);
        }


        regValue = 0x5a5a5a5a;
        if (ioctl(fd, PBE_IOC_WRITE_ERRORINJECT, regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
        } else {
            printf("Set Error Register value : 0x%08x \n", regValue);
        }

    } while (timeout-- > 0);

    printf("The error register r/w test is done!");
    return 0;
#endif


    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_RD_CFG_REG);
    } else {
        maxPayloadSize = (regValue & 0x000000E0) >> 5;
        printf("Max payload size: %d \n", maxPayloadSize);
    }

    printf("*************** file : %s, func : %s, line : %d ****************\n", __FILE__, __func__, __LINE__);
    printf("sleep 5s ..... \n");
    sleep(5);

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
        default: { printf("Max payload size is invalid \n"); } break;
    }

    printf("Max payload: %d, tlp max size: %d \n", maxPayloadSize, tlpSizeMax);

    uint32_t writeTLPSize = 256;
    uint32_t writeTLPCount = 4;
    uint32_t readTLPSize = 256;
    uint32_t readTLPCount = 4;

    // NOTE(michiel): Copy data to kernel
    /*write_data(fd, 1024 * sizeof(*writeBuffer), writeBuffer);*/
    /*printf("Data copied to kernel \n");*/

    // NOTE(michiel): Setup DMA
    if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
        printf("IOCTL failed setting reset \n");
    } else {
        printf("DMA Reset \n");
    }

    uint32_t dmaControlReg = 0;
    printf("Set writeTLPSize = 256, writeTLPCount = 4, readTLPSize = 256, readTLPCount = 4. \n");
    printf(" Get dmaControlReg after reset ..... \n");
    if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
        printf("IOCTL failed reading DMA Control \n");
    } else {
        printf("DMA Control: 0x%08X \n", dmaControlReg);
    }

    if (ioctl(fd, PBE_IOC_WRITE_WR_COUNT, writeTLPCount) < 0) {
        printf("IOCTL failed setting the write TLP count \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_WR_LEN, writeTLPSize) < 0) {
        printf("IOCTL failed setting the write TLP size \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_RD_COUNT, readTLPCount) < 0) {
        printf("IOCTL failed setting the read TLP count \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_RD_LEN, readTLPSize) < 0) {
        printf("IOCTL failed setting the read TLP size \n");
    }

    if (ioctl(fd, PBE_IOC_READ_WR_COUNT, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP count \n");
    } else {
        printf("Write TLP count: %u, expected %u \n", regValue, writeTLPCount);
    }

    if (ioctl(fd, PBE_IOC_READ_WR_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP size \n");
    } else {
        printf("Write TLP size: %u, expected %u \n", regValue, writeTLPSize);
    }

    if (ioctl(fd, PBE_IOC_READ_RD_COUNT, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP count \n");
    } else {
        printf("Read TLP count: %u, expected %u \n", regValue, readTLPCount);
    }

    if (ioctl(fd, PBE_IOC_READ_RD_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP size \n");
    } else {
        printf("Read TLP size: %u, expected %u \n", regValue, readTLPSize);
    }

    printf("*************** file : %s, func : %s, line : %d ****************\n", __FILE__, __func__, __LINE__);
    printf("sleep 15s .....\n \n");
    sleep(15);


    writeTLPSize = 4096;
    writeTLPCount = 8;
    readTLPSize = 4096;
    readTLPCount = 8;

    // NOTE(michiel): Setup DMA
    if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
        printf("IOCTL failed setting reset \n");
    } else {
        printf("DMA Reset \n");
    }

    printf("Set writeTLPSize = 4096, writeTLPCount = 8, readTLPSize = 4096, readTLPCount = 8. \n");
    printf(" Get dmaControlReg after reset ##### \n");
    if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
        printf("IOCTL failed reading DMA Control \n");
    } else {
        printf("DMA Control: 0x%08X \n", dmaControlReg);
    }

    if (ioctl(fd, PBE_IOC_WRITE_WR_COUNT, writeTLPCount) < 0) {
        printf("IOCTL failed setting the write TLP count \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_WR_LEN, writeTLPSize) < 0) {
        printf("IOCTL failed setting the write TLP size \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_RD_COUNT, readTLPCount) < 0) {
        printf("IOCTL failed setting the read TLP count \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_RD_LEN, readTLPSize) < 0) {
        printf("IOCTL failed setting the read TLP size \n");
    }

    if (ioctl(fd, PBE_IOC_READ_WR_COUNT, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP count \n");
    } else {
        printf("Write TLP count: %u, expected %u \n", regValue, writeTLPCount);
    }

    if (ioctl(fd, PBE_IOC_READ_WR_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the write TLP size \n");
    } else {
        printf("Write TLP size: %u, expected %u \n", regValue, writeTLPSize);
    }

    if (ioctl(fd, PBE_IOC_READ_RD_COUNT, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP count \n");
    } else {
        printf("Read TLP count: %u, expected %u \n", regValue, readTLPCount);
    }

    if (ioctl(fd, PBE_IOC_READ_RD_LEN, &regValue) < 0) {
        printf("IOCTL failed reading the read TLP size \n");
    } else {
        printf("Read TLP size: %u, expected %u \n", regValue, readTLPSize);
    }

    printf("*************** file : %s, func : %s, line : %d ****************\n", __FILE__, __func__, __LINE__);
    printf("sleep 15s .....\n \n");
    sleep(15);


#if 0
    uint32_t writeWRRCount = 0; // 1;
    uint32_t readWRRCount = 0; //1;
    // 0x01010000 or 0x01010020
    uint32_t miscControl = (writeWRRCount << 24) | (readWRRCount << 16);

    if (ioctl(fd, PBE_IOC_WRITE_MISC_CTL, miscControl) < 0) {
        printf("IOCTL failed setting misc control \n");
    }

    uint32_t readEnable = doRead ? 0 : 1;
    uint32_t writeEnable = doRead ? 1 : 0;
    dmaControlReg |= (readEnable << 16) | writeEnable;
    if (ioctl(fd, PBE_IOC_WRITE_DMA_CTRL, dmaControlReg) < 0) {
        printf("IOCTL failed setting DMA control \n");
    }

#if 0
    usleep(1000000);
    #else
    do {
    usleep(1000);
        if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
            printf("IOCTL failed reading DMA control \n");
        }
    } while ((readEnable && (dmaControlReg & 0x01010000)) ||
             (writeEnable && (dmaControlReg & 0x00000101)));
    #endif

    uint32_t dmaControlWrite = 0;
    uint32_t dmaControlRead = 0;

    if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &regValue) < 0) {
        printf("IOCTL failed reading DMA control \n");
    } else {
        dmaControlReg = regValue;
        dmaControlWrite = dmaControlReg & 0x0000FFFF;
        dmaControlRead = (dmaControlReg & 0xFFFF0000) >> 16;
    }

    read_data(fd, writeTLPSize * writeTLPCount * sizeof(uint32_t), readBuffer);
    printf("Data copied from kernel \n");

    // NOTE(michiel): Read error check
    if (readEnable) {
        if ((dmaControlRead & 0x1111) != 0x0101) {
            printf("DMA Read did not complete succesfully, 0x%04X \n",
                   dmaControlRead);
        } else {
            printf("DMA Read success! \n");
        }
    }

    // NOTE(michiel): Write error check
    if (writeEnable) {
        int32_t writeError = false;
        for (uint32_t i = 0; i < (writeTLPSize * writeTLPCount); ++i) {
            uint32_t readData = readBuffer[i];
            uint32_t writeData = writeBuffer[i];
            if (readData != writeData) {
                printf("Mismatch: wrote %d, got %d \n", writeData, readData);
                writeError = true;
            }
        }

        if (!writeError) {
            if ((dmaControlWrite & 0x1111) != 0x0101) {
                printf("DMA Write did not complete succesfully, 0x%04X \n",
                       dmaControlWrite);
            } else {
                printf("DMA Write success! \n");
            }
        }
    }

    regValue = exerciserconfig.deviceStatContOffset;
    if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
        printf("Device status read failed \n");
    } else {
        if ((regValue & 0x00040000) == 0x00040000) {
            printf("Fatal reported by device \n");
        }
        if ((regValue & 0x00020000) == 0x00020000) {
            printf("Non fatal reported by device \n");
        }
        if ((regValue & 0x00010000) == 0x00010000) {
            printf("Correctable reported by device \n");
        }
    }

    int32_t linkWidthMultiplier = 0;
    int32_t trnClks = 0;
    int32_t tempWrMbps = 0;
    int32_t tempRdMbps = 0;

    char *gen = 0;
    switch (exerciserconfig.linkSpeed)
    {
        case 1: {
            gen = "Generation 1";
            switch (exerciserconfig.linkWidth)
            {
                case 1: { linkWidthMultiplier = 31; } break;
                case 2: { linkWidthMultiplier = 62; } break;
                case 3: { linkWidthMultiplier = 125; } break;
                case 4: { linkWidthMultiplier = 250; } break;
                default: { printf("%s: Link width is not valid \n", gen); } break;
        }
        } break;

        case 2: {
            gen = "Generation 2";
            switch (exerciserconfig.linkWidth)
            {
                case 1: { linkWidthMultiplier = 62; } break;
                case 2: { linkWidthMultiplier = 125; } break;
                case 3: { linkWidthMultiplier = 250; } break;
                case 4: { linkWidthMultiplier = 500; } break;
                default: { printf("%s: Link width is not valid \n", gen); } break;
            }
        } break;

        default: {
            printf("Link speed is not valid \n");
        } break;
    }

    if (writeEnable) {
        if (ioctl(fd, PBE_IOC_READ_WR_PERF, &trnClks) < 0) {
            printf("IOCTL failed reading write performance \n");
        }

        tempWrMbps = (writeTLPSize * 4 * writeTLPCount * linkWidthMultiplier) / trnClks;
        printf("DMA Write TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempWrMbps);
    }

    if (readEnable) {
        if (ioctl(fd, PBE_IOC_READ_RD_PERF, &trnClks) < 0) {
            printf("IOCTL failed reading read performance \n");
        }

        tempRdMbps = (readTLPSize * 4 * readTLPCount * linkWidthMultiplier) / trnClks;
        printf("DMA Read TRN Clocks: %d, Perf: %d MB/s \n", trnClks, tempRdMbps);
    }

    free(writeBuffer);
    free(readBuffer);
#endif

    close(fd);

    return 0;
}
