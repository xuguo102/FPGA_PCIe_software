#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../../driver_h10/amd_pcie_exerciser.h"

int fd;

typedef union RegWrite {
    uint64_t raw;
    struct {
        int reg;
        int value;
    };
} RegWrite;

int main(int argc, char **argv) {

    RegWrite Argvalue = {0};
    char *endptr;
    char *endptr1;

    char *input = argv[1];
    char *input1 = argv[2];

    // Check parameters
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: %s <parameter1> <parameter2>\n\n", argv[0]);
        printf("This program requires two parameters to execute:\n");
        printf("  <parameter1>  Register Address\n");
        printf("  <parameter2>  Register Value \n\n");
        printf("Options:\n");
        printf("  -h, --help    Show this help message\n\n");
        printf("Example:\n");
        printf("  %s 100 input.txt\n", argv[0]);
        return 0;
    }

    //  Verify parameters couter
    if ((argc > 3) || (argc < 2)) {
        fprintf(stderr, "Error: Invalid number of parameters\n");
        fprintf(stderr, "Use '%s -h' for usage information\n", argv[0]);
        return 1;
    }

    // remove "0x" or "0X"
    if (input[0] == '0' && (input[1] == 'x' || input[1] == 'X')) {
        input += 2;
    }

    // Check if the remaining string is valid
    if (*input == '\0') {
        fprintf(stderr, "Error: Input is empty \n");
        return 1;
    }

    Argvalue.reg = strtoul(input, &endptr, 16); // Hexadecimal conversion
    printf("Register arrdess : 0x%x \n", Argvalue.reg);

    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid character '%c'\n", *endptr);
        return 1;
    }

    if (argc == 3) {
        if (input1[0] == '0' && (input1[1] == 'x' || input1[1] == 'X')) {
            input1 += 2;
        }

        if (*input1 == '\0') {
            fprintf(stderr, "Error: Input is empty \n");
            return 1;
        }

        Argvalue.value = strtoul(input1, &endptr1, 16); // Hexadecimal conversion
        printf("Write value : 0x%x \n", Argvalue.value);

        if (*endptr1 != '\0') {
            fprintf(stderr, "Error: Invalid character '%c'\n", *endptr1);
            return 1;
        }
    }

    fd = open("/dev/amdpcieexerciser", O_RDWR);
    if (fd <= 0) {
        printf("Could not open file /dev/amdpcieexerciser \n");
        return -1;
    }

    printf("Program executed \n");
    // Read
    if (argc == 2) {
        printf("Parameter 1: %s\n", argv[1]);

        if (ioctl(fd, PBE_IOC_RD_ANY_REG, &Argvalue.raw) < 0) {
            printf("IOCTL failed write 0x%03lX \n", PBE_IOC_RD_ANY_REG);
        } else {
            printf("Read : Register 0x%x , value 0x%x . \n", Argvalue.reg, Argvalue.value);
        }
    }

    // Write
    if (argc == 3) {
        printf("Parameter 1: %s\n", argv[1]);
        printf("Parameter 2: %s\n", argv[2]);

        if (ioctl(fd, PBE_IOC_WR_ANY_REG, &Argvalue.raw) < 0) {
            printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WR_ANY_REG);
        } else {
            printf("Write : Register 0x%x , value 0x%x . \n", Argvalue.reg, Argvalue.value);
        }
    }

    printf("The register r/w test is done! \n");

    close(fd);

    return 0;
}
