
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <math.h>
#include <algorithm>

word_t getOffset(uint64_t address);

uint64_t getRelativeAddr(uint64_t virtualAddress, int x);


uint64_t getPage(uint64_t address);

int findNextEmptyFrame(int addr, int max, int dep)
{
    if (dep == TABLES_DEPTH-1)
    {
        return std::max(addr, max);
    }
    word_t val;
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        PMread(addr*PAGE_SIZE + i, &val);
        if (val != 0)
        {
            max = std::max(max, findNextEmptyFrame(val/PAGE_SIZE,std::max(addr,max),dep+1));
        }
    }
    return std::max(addr,max);
}

int findUnusedFrame(int addr, int min, int dep)
{
    word_t val;
    bool allZero = true;
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        PMread(addr*PAGE_SIZE + i, &val);
        if (val != 0)
        {
            allZero = false;
        }
    }
    if (allZero)
    {
        return addr;
    }
    if (dep == TABLES_DEPTH-1)
    {
        return NUM_FRAMES;
    }
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        PMread(addr*PAGE_SIZE + i, &val);
        if (val != 0)
        {
            min = std::min(findUnusedFrame (val/PAGE_SIZE,min,dep+1),min);
        }
    }
    return min;
}

void removeRefrenceToFrame(int addr,int frameNum)
{
    word_t val;
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        PMread(addr*PAGE_SIZE + i, &val);
        if (val == frameNum)
        {
            PMwrite (addr*PAGE_SIZE + i, 0);
            break;
        }
        else
        {
            removeRefrenceToFrame (val*PAGE_SIZE, frameNum);
        }
    }
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
    word_t addr = 0;
    word_t lastAddr;
    for (int i = 0; i < TABLES_DEPTH; i++)
    {
        lastAddr = addr;
        PMread(addr * PAGE_SIZE + getRelativeAddr(virtualAddress,i), &addr);
        //TODO: case addr is zero
        if (addr == 0 and i != (TABLES_DEPTH-1))
        {
            //TODO: check if this is the right not-end-iteration

            //check if there is empty table
            int frameNum = findUnusedFrame(0,NUM_FRAMES,0);
            if (frameNum != NUM_FRAMES && frameNum != 0)
            {
                //set the new frame
                removeRefrenceToFrame(0,frameNum);
                PMwrite (lastAddr * PAGE_SIZE + getRelativeAddr(virtualAddress,i),frameNum);
            }
            else
            {
                frameNum = findNextEmptyFrame (0,0,0);
                if (frameNum + 1 < NUM_FRAMES)
                {
                    PMwrite (lastAddr * PAGE_SIZE + getRelativeAddr(virtualAddress,
                                                                    i),frameNum+1);
                    //TODO: zero the new frame, frameNum+1
                    for (int off = 0; off < PAGE_SIZE; ++off)
                    {
                        PMwrite ((frameNum+1) * PAGE_SIZE  + off,0);
                    }
                }
            }
        }
    }
    // supposed to bring it from the memory first and than read it
    PMrestore(addr, getPage(virtualAddress));
    PMread(addr + getOffset(virtualAddress), value);

}

uint64_t getPage(uint64_t address)
{
    return address >> OFFSET_WIDTH;
}



//uint64_t helperEmptyFrame(uint64_t x)
//{
//    uint64_t max = 0;
//    for (int i = 0; i < PAGE_SIZE; i++)
//    {
//
//    }
//}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value)
{

}


word_t getOffset(uint64_t address)
{
    return word_t(address % (1 << OFFSET_WIDTH));
}

uint64_t getRelativeAddr(uint64_t virtualAddress, int x)
{
    return ((virtualAddress >> uint64_t(VIRTUAL_ADDRESS_WIDTH - CEIL(x * log(PAGE_SIZE))))%PAGE_SIZE);
}





