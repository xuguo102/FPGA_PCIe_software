CC ?= gcc

all: pcie_error_injection dma_traffic
#all: dma_traffic

dma_traffic: dma_traffic.o
	$(CC) -o $@ $<

#performance: performance.o
	#$(CC) -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

#direct: direct.o
	#$(CC) -o $@ $<

#test_chrdev: test_chrdev.o
	#$(CC) -o $@ $<

#exerciser: exerciser.o
	#$(CC) -o $@ $<

exerciser: pcie_error_injection.o
	$(CC) -o $@ $<

%.o: %.c
	$(CC) -c -std=c99 -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

clean:
	rm -rf reg_rw *.o *.bin dma_traffic dma_traffic_write dma_traffic_read performance test_chrdev pcie_error_injection exerciser
