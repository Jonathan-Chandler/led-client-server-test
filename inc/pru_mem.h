#ifndef __PRU_SHMEM_H__
#define __PRU_SHMEM_H__
#include <stdint.h>
#include <fcntl.h>
#include "led.h"

class PruMem
{
public:
    PruMem(const void *addr);
    ~PruMem();

    void write_mem_led_colors(uint32_t size, const char *buff);
    void write_mem_led_count(uint8_t led_count);
    void write_mem_led_start();
    void write_mem_led_stop();

private:
    const void *physical_addr;
    int shared_mem_fd;
    volatile uint32_t* shared_mem_map;

    void allocate_mem(const void *physical_addr);
    void deallocate_mem();
};

#endif
