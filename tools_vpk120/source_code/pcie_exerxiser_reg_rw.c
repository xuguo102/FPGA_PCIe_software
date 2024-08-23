#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>
#include <signal.h>

#include "../driver_vpk120/amd_pcie_exerciser.h"

int fd;

int main(int argc, char **argv) {
    uint32_t regValue = 0;

    fd = open("/dev/amdpcieexerciser", O_RDWR);
    if (fd <= 0) {
        printf("Could not open file /dev/amdpcieexerciser \n");
        return -1;
    }

    // NOTE(michiel): Reset register
    if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
        printf("IOCTL failed setting reset \n");
    } else {
        printf("Register Reset ......\n");
    }

    if (ioctl(fd, PBE_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
    } else {
        printf("Get Error Register value : 0x%08x \n", regValue);
    }

    regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
    } else {
        printf("Set Error Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get Error Register value : 0x%08x \n", regValue);
    }


    regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set Cor Error Register value : 0x%08x \n", regValue);
    }


    if (ioctl(fd, PBE_IOC_READ_UNCOR_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_UNCOR_ERRORINJECT);
    } else {
        printf("Get Error Register value : 0x%08x \n", regValue);
    }

    regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_UNCOR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_UNCOR_ERRORINJECT);
    } else {
        printf("Set UnCor ERRORINJECT Register value : 0x%08x \n", regValue);
    }

    regValue = 1;
    if (ioctl(fd, PBE_IOC_WRITE_AER_CNTL, regValue) < 0) {
        printf("IOCTL failed write 0x%03lX \n", PBE_IOC_WRITE_AER_CNTL);
    } else {
        printf("Clear aer cor states !!!!! \n");
    }

    if (ioctl(fd, PBE_IOC_READ_AER_CNTL, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_AER_CNTL);
    } else {
        printf("Get aer cor value : 0x%08x \n", regValue);
    }

    regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_AER_CNTL, regValue) < 0) {
        printf("IOCTL failed write 0x%03lX \n", PBE_IOC_WRITE_AER_CNTL);
    } else {
        printf("Clear aer cor states !!!!! \n");
    }

    if (ioctl(fd, PBE_IOC_READ_AER_CNTL, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_AER_CNTL);
    } else {
        printf("Get aer cor value : 0x%08x \n", regValue);
    }



    printf("The error register r/w test is done! \n");

    close(fd);

    return 0;
}
