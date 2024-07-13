
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <math.h>

uint64_t getRelativeAddr(uint64_t virtualAddress,int x)
{
    return ((virtualAddress >> uint64_t(CEIL(x * log(PAGE_SIZE))))%PAGE_SIZE);
}
/*
 * Initialize the virtual memory.
 */
void VMinitialize()
{
    for (int i = 0; i < PAGE_SIZE; i++) 
    {
        PMwrite(i,0);
    }
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value)
{
    uint64_t addr = 0;
    for (int i = 0; i < TABLES_DEPTH; i++)
    {
        PMread(addr * PAGE_SIZE + getRelativeAddr(virtualAddress,i));
    }
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value)
{

}

