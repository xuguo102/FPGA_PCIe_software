#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>
#include <signal.h>

#include "../driver/amd_pcie_exerciser.h"

int fd;
int eflag;

void input_handler(int num)
{

    int regValue = 0;

    //Read and output input on STDIN_FILENO
    eflag = 0;
    printf("**** interupt num : %d ****\n", num);
    printf("Get the SIGIO signal, we exit the application!\n");

#if 1
    printf("Clear the error \n!\n");
    if (ioctl(fd, XBMD_IOC_WRITE_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Set Error inject ctl Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Get Error inject ctl Register value : 0x%08x \n", regValue);
    }
#endif

}

int main(int argc, char **argv)
{
    int oflags;
    uint32_t timeout = 1000000;
    uint32_t regValue = 0;

    fd = open("/dev/amdpcieexerciser", O_RDWR);
    if (fd <= 0) {
        printf("Could not open file /dev/amdpcieexerciser \n");
    }

    //Start signal
    signal(SIGIO, input_handler);
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);

#if 0
    // NOTE(michiel): Setup DMA
    if (ioctl(fd, XBMD_IOC_RESET, 0) < 0) {
        printf("IOCTL failed setting reset \n");
    } else {
        printf("DMA Reset \n");
    }

    do {
        if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
        } else {
            printf("Error Register value : 0x%08x \n", regValue);
        }

        regValue = 0x23;
        if (ioctl(fd, XBMD_IOC_WRITE_ERRORINJECT, regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
        } else {
            printf("Set Error Register value : 0x%08x \n", regValue);
        }


        if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
        } else {
            printf("Get Error Register value : 0x%08x \n", regValue);
        }


        regValue = 0x5a5a5a5a;
        if (ioctl(fd, XBMD_IOC_WRITE_ERRORINJECT, regValue) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
        } else {
            printf("Set Error Register value : 0x%08x \n", regValue);
        }

    } while (timeout-- > 0);

    printf("The error register r/w test is done!");
#endif

    if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Error inject ctl Register value : 0x%08x \n", regValue);
    }

    eflag = 1;
    printf("Seting the error register!");
    regValue = 0x1;
    if (ioctl(fd, XBMD_IOC_WRITE_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Set Error inject ctl Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Get Error inject ctl Register value : 0x%08x \n", regValue);
    }


    printf("sleep 3s ..... \n");
    sleep(3);
    printf("sleep over ..... \n");

    printf("wait for error comming ....... !");
    while (eflag);

#if 0
    printf("Clear the error \n!");
    regValue = 0;
    if (ioctl(fd, XBMD_IOC_WRITE_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Set Error inject ctl Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, XBMD_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", XBMD_IOC_READ_ERRORINJECT);
    } else {
        printf("Get Error inject ctl Register value : 0x%08x \n", regValue);
    }
#endif
    close(fd);

    return 0;
}
