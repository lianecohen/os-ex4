
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <math.h>
#include <algorithm>

word_t getOffset(uint64_t address);
uint64_t evictFrame(uint64_t wantedPage);
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

int findFrame(int addr, int page, int dep) {
    word_t val;

    if (dep == TABLES_DEPTH-1) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            PMread(addr * PAGE_SIZE + i, &val);
            if (val == page) {
                return addr; // Return the frame number
            }
        }
        return -1; // Indicate that the page was not found
    }

    for (int i = 0; i < PAGE_SIZE; i++) {
        PMread(addr * PAGE_SIZE + i, &val);
        if (val != 0) { // Only traverse non-zero entries
            int ret = findFrame(val, page, dep + 1);
            if (ret != -1) {
                return ret;
            }
        }
    }

    return -1; // Indicate that the page was not found
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
        uint64_t relativeAddr = getRelativeAddr(virtualAddress,i);
        PMread(addr * PAGE_SIZE + relativeAddr, &addr);
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
                PMwrite (lastAddr * PAGE_SIZE + relativeAddr,frameNum);
            }
            else
            {
                frameNum = findNextEmptyFrame (0,0,0);
                if (frameNum + 1 < NUM_FRAMES)
                {
                    PMwrite (lastAddr * PAGE_SIZE + relativeAddr,frameNum+1);
                    // zero the new frame +1
                    for (int off = 0; off < PAGE_SIZE; ++off)
                    {
                        PMwrite ((frameNum+1) * PAGE_SIZE  + off,0);
                    }
                }
                // in case that all of the frames are full, we would check which table we should remove
                else {
                    //todo:: find the frame that connected to the relevant page.
                    uint64_t page = evictFrame(getPage(virtualAddress));
                    int newFrame = findFrame(0,page,0);
                    PMevict(newFrame,page);
                    removeRefrenceToFrame(0,newFrame);
                    //todo:: link to the parent

                    // zero the new frame +1
                    for (int off = 0; off < PAGE_SIZE; ++off)
                    {
                        PMwrite (newFrame * PAGE_SIZE  + off,0);
                    }
                }


            }
        }

    }
    // supposed to bring it from the memory first and than read it
    PMread(addr + getOffset(virtualAddress), value);

}
uint64_t evictPage(uint64_t wantedPage)
{
    int max = 0;
    uint64_t relatedPage;
    word_t val;
    uint64_t addr = 0;
    for (int i = 0; i < NUM_PAGES; ++i)
    {
        //check to any page that is mapped to the frame what cycle
        if(findFrame(0,i,0) > 0) {
            if(max == std::max(max,std::min(static_cast<int>(NUM_PAGES - wantedPage-i), static_cast<int>(wantedPage-i))))
            {
                relatedPage = i;
            }
        }
    }
    // find the frame related to the page we would like to remove
    return relatedPage;
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





