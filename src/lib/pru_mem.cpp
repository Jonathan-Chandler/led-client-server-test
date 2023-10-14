#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sstream>
#include <inttypes.h>
#include <stdexcept>
#include <iostream>

#include "pru_mem.h"
#include "share.h"


PruMem::PruMem(const void *addr)
    : physical_addr(addr)
    , shared_mem_fd(-1)
    , shared_mem_map((volatile uint32_t*) MAP_FAILED)
{
    // create memory map
    allocate_mem(addr);
}

void PruMem::allocate_mem(const void *physical_addr)
{
  // open shared memory file descriptor
  shared_mem_fd = open(SHARED_MEM_MAP_FILE, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  if (shared_mem_fd < 0)
  {
      std::ostringstream err_str;

      err_str << "PruMem failed to open file " << SHARED_MEM_MAP_FILE;
      throw std::runtime_error(err_str.str());
  }

#if DEBUG_NO_SHMEM
  // force file length if debug shmem mode
  if (ftruncate(shared_mem_fd, SHARED_MEM_SIZE) != 0)
  {
      std::ostringstream err_str;

      err_str << "PruMem failed to set file size for " << SHARED_MEM_MAP_FILE;
      throw std::runtime_error(err_str.str());
  }
#endif

  // get memory map on file descriptor
  shared_mem_map = (volatile uint32_t*) mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, SHARED_MEM_START_ADDR);
  if (shared_mem_map == MAP_FAILED) 
  {
      std::ostringstream err_str;

      err_str << "PruMem failed set memory map for " << SHARED_MEM_MAP_FILE;
      throw std::runtime_error(err_str.str());
  }
}

void PruMem::deallocate_mem()
{
  int rc = 0;

  if (shared_mem_map != MAP_FAILED)
  {
      if (munmap((void *)shared_mem_map, getpagesize()) == -1) 
      {
          std::cerr << "Fail to unmap memory at " << physical_addr << std::endl;
      }
  }
  else
  {
    std::cerr << "Skip unmap memory at " << physical_addr << " shared memory map failed" << std::endl;
  }

  // close file desc
  if (shared_mem_fd >= 0)
  {
      if (close(shared_mem_fd))
      {
          std::cerr << "Fail to close file descriptor" << std::endl;
      }
  }
  else
  {
    std::cerr << "Skip closing file (bad file descriptor)" << std::endl;
  }
}

void PruMem::write_mem_led_colors(uint32_t size, const char *buff)
{
    if ((size + SHARED_MEM_LED_START_OFFSET) > getpagesize())
    {
        std::ostringstream err_str;

        err_str << "PruMem::write_mem size " << size << " bytes is greater than allocated (" <<  getpagesize() << " bytes)";
        throw std::invalid_argument(err_str.str());
    }

    if (buff == nullptr)
    {
        std::ostringstream err_str;

        err_str << "PruMem::write_mem attempt to write null buffer";
        throw std::invalid_argument(err_str.str());
    }

    // synchronize color values
    char *shared_mem_bytes = (char*) shared_mem_map;

    // write the values before setting
    for (uint32_t i = SHARED_MEM_LED_START_OFFSET; i < size; i++)
    {
        shared_mem_bytes[i] = buff[i];
    }
}

void PruMem::write_mem_led_count(uint8_t led_count)
{
    // synchronize color values
    char *shared_mem_bytes = (char*) shared_mem_map;

    // set LED count
    shared_mem_bytes[SHARED_MEM_LED_COUNT_OFFSET] = led_count;

}

void PruMem::write_mem_led_start()
{
    // synchronize color values
    char *shared_mem_bytes = (char*) shared_mem_map;

    // begin PRU program to configure colors currently in shared memory
    shared_mem_bytes[SHARED_MEM_LED_BEGIN_WRITE_OFFSET] = 1;
}

void PruMem::write_mem_led_stop()
{
    // synchronize color values
    char *shared_mem_bytes = (char*) shared_mem_map;

    // stop PRU program if writing to LEDs
    shared_mem_bytes[SHARED_MEM_LED_BEGIN_WRITE_OFFSET] = 0;
}
