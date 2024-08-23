#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../driver/amd_pcie_exerciser.h"

int main(int argc, char **argv)
{
	int fd = open("/dev/amdpcieexerciser", O_RDWR);
	if (fd <= 0) {
		printf("Could not open file /dev/amdpcieexerciser \n");
	}

	uint32_t *writeBuffer = (uint32_t*)malloc(sizeof(uint32_t) * 1024);
	uint32_t *readBuffer  = (uint32_t*)malloc(sizeof(uint32_t) * 1024);

	for (uint32_t i = 0; i < 1024; ++i) {
		writeBuffer[i] = (i + 1) * 3;
	}

	for (uint32_t i = 0; i < 1024; ++i) {
		readBuffer[i] = 0xDEADBEAD;
	}

	read(fd, readBuffer, 1024 * sizeof(uint32_t));
	printf("Data copied from kernel \n");

	// NOTE(michiel): Copy data to kernel
	write(fd, writeBuffer, 1024 * sizeof(uint32_t));
	printf("Data copied to kernel \n");

	read(fd, readBuffer, 1024 * sizeof(uint32_t));
	printf("Data copied from kernel \n");

	// NOTE(michiel): Write error check
	for (uint32_t i = 0; i < 1024; ++i) {
		uint32_t readData = readBuffer[i];
		uint32_t writeData = writeBuffer[i];
		if (readData != writeData) {
			printf("Mismatch: wrote %d, got %d \n", writeData, readData);
		}
	}

	free(writeBuffer);
	free(readBuffer);
	close(fd);

	return 0;
}
