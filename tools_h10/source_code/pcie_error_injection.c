#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>
#include <signal.h>

#include "../driver_vpk120/amd_pcie_exerciser.h"

/*#define Reg_RW_Test*/

/*#define TIMEOUT_TEST*/

int fd;
int eflag;

void input_handler(int num) {

    int regValue = 0;

    //Read and output input on STDIN_FILENO
    eflag = 0;
    printf("**** interupt num : %d ****\n", num);
    printf("Get the SIGIO signal, we exit the application!\n");

#ifndef TIMEOUT_TEST
    printf("Clear the error !\n");

    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed wite 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("clear COR Error Register !!!! \n");
    }

#if 0
    if (ioctl(fd, PBE_IOC_WRITE_UNCOR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed wite 0x%03lX \n", PBE_IOC_READ_UNCOR_ERRORINJECT);
    } else {
        printf("Clear UNCOR Error Register !!!! \n");
    }
#endif

    if (ioctl(fd, PBE_IOC_WRITE_AER_CNTL, 1) < 0) {
        printf("IOCTL failed write 0x%03lX \n", PBE_IOC_WRITE_AER_CNTL);
    } else {
        printf("Clear aer cor states !!!!! \n");
    }

#endif
}

int main(int argc, char **argv) {
    int oflags;
    uint32_t timeout = 1000000;
    uint32_t regValue = 0;

    fd = open("/dev/amdpcieexerciser", O_RDWR);
    if (fd <= 0) {
        printf("Could not open file /dev/amdpcieexerciser \n");
        return -1;
    }

    //Start signal
    signal(SIGIO, input_handler);
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);

#ifdef Reg_RW_Test
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

    regValue = 0x5a5a5a5a;
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


    regValue = 0x5a5a5a5a;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set Error Register value : 0x%08x \n", regValue);
    }


    if (ioctl(fd, PBE_IOC_READ_UNCOR_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_UNCOR_ERRORINJECT);
    } else {
        printf("Get Error Register value : 0x%08x \n", regValue);
    }

    regValue = 0x5a5a5a5a;
    if (ioctl(fd, PBE_IOC_WRITE_UNCOR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_UNCOR_ERRORINJECT);
    } else {
        printf("Set Error Register value : 0x%08x \n", regValue);
    }

    printf("The error register r/w test is done!");

    return 1;
#endif

#if 0
    if (ioctl(fd, PBE_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
    } else {
        printf("Error inject ctl Register value : 0x%08x \n", regValue);
    }
#endif

    eflag = 1;

#if 0
    printf("Seting the error register!");
    regValue = 0x1;
    if (ioctl(fd, PBE_IOC_WRITE_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
    } else {
        printf("Set Error inject ctl Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_ERRORINJECT);
    } else {
        printf("Get Error inject ctl Register value : 0x%08x \n", regValue);
    }

    }
#endif

    // Clear error states
#if 0
    regValue = 1;
    if (ioctl(fd, PBE_IOC_WRITE_AER_CNTL, regValue) < 0) {
        printf("IOCTL failed write 0x%03lX \n", PBE_IOC_WRITE_AER_CNTL);
    } else {
        printf("Clear aer cor states !!!!! \n");
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
        printf("Get COR Error states value : 0x%08x \n", regValue);
    }
#endif

    // Set AER error
    printf("Set AER error!\n");
    regValue = 2;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", regValue);
    }

    /*
     *if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &regValue) < 0) {
     *    printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
     *} else {
     *    printf("Get COR Error Register value : 0x%08x \n", regValue);
     *}
     */

#if 0
    regValue = 0x1;
    if (ioctl(fd, PBE_IOC_WRITE_UNCOR_ERRORINJECT, regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_UNCOR_ERRORINJECT);
    } else {
        printf("Set UNCOR Error Register value : 0x%08x \n", regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_UNCOR_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_UNCOR_ERRORINJECT);
    } else {
        printf("Get UNCOR Error Register value : 0x%08x \n", regValue);
    }
#endif
    
    printf("Triger error ....... !\n");
    
    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Triger error done ....... !\n");
    }

    printf("Wait for error comming ....... !\n");
    while (eflag) {
    } 
    printf("AER error done ....... !!!!!!\n");


#ifdef TIMEOUT_TEST
    printf("Wait 5s for timeout done.");
    sleep(10);
    printf("Clear the error !\n");

    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, 0) < 0) {
        printf("IOCTL failed wite 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("clear COR Error Register !!!! \n");
    }

    if (ioctl(fd, PBE_IOC_WRITE_AER_CNTL, 1) < 0) {
        printf("IOCTL failed write 0x%03lX \n", PBE_IOC_WRITE_AER_CNTL);
    } else {
        printf("Clear aer cor states !!!!! \n");
    }

#endif


    close(fd);

    return 0;
}
