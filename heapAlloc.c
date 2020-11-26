////////////////////////////////////////////////////////////////////////////////
// Main File:        heapAlloc.c
// This File:        heapAlloc.c
// Other Files:      files given to us by professor
//                   alloc1.c
//                   alloc1.nospace.c
//                   alloc2.c
//                   alloc2_nospace.c
//                   writeable.c
//                   align1.c
//                   align2.c
//                   alloc3.c
//                   align3.c
//                   free1.c
//                   free2.c
//                   coalesce1.c
//                   coalesce2.c
//                   coalesce3.c
//                   coalesce4.c
//                   coalesce5.c
//                   coalesce6.c 
// Semester:         CS 354 Fall 2019
//
// Author:           lakshay goyal  
// Email:            lgoyal 
// CS Login:         lakshay
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//////////////////////////// 80 columns wide ///////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Copyright 2019-2020 Jim Skrentny
    // Posting or sharing this file is prohibited, including any changes/additions.
    //
    ///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "heapAlloc.h"


/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           
    int size_status;
    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Header:
     *      If the previous block is allocated, size_status should be 27
     *      If the previous block is free, size_status should be 25
     * 
     * 2. Free block of size 24 bytes:
     *    Header:
     *      If the previous block is allocated, size_status should be 26
     *      If the previous block is free, size_status should be 24
     *    Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heapStart = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int allocsize;

/*
 * Additional global variables may be added as needed below
 */ 
int initHeapSize; //Initial heap size

int heapSize;  //space to allocate for the heap

blockHeader * next;

// Helper Function to find the next free pointer
blockHeader* Find(blockHeader * start,int size)
{
        
    int length = start->size_status; //length of block

    while(length % 8 != 0){
        length--;
    }

    blockHeader* next = (blockHeader*)((void*)start + length); // pointer to next free block

    while((next->size_status & 1) == 1 || next->size_status < size){
        if(next == start){
            return NULL; 
        }
        if(next->size_status == 1){
            next = heapStart; 
            continue;
        }
        int length2 = next->size_status;
        while(length2 % 8 != 0){
            length2--;
        }
        next = (blockHeader*)((void*)next + length2);
    }
    return next;

}

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block on success.
 * Returns NULL on failure.
 * This function should:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
 * - Use NEXT-FIT PLACEMENT POLICY to chose a free block
 * - Use SPLITTING to divide the chosen free block into two if it is too large.
 * - Update header(s) and footer as needed.
 * Tips: Be careful with pointer arithmetic and scale factors
 */

void* allocHeap(int size) 
{
    
    if(size<=0||((size+4)>allocsize)){
        return NULL;} 

    int allocsize=0;
  
    allocsize = size + 4;
      
    if(allocsize%8!=0){
        allocsize=((allocsize/8)+1)*8;
        }

    int x = 0;
    switch(next->size_status % 8){
        case 2: {
            x = 2;
            break;
        }
        case 3:{
            x = 2;
            break;
        }
    }

    blockHeader * b1;
    b1= Find(next,allocsize);
    if(b1 != NULL)
        next = b1;
    else{
        return NULL;
        
    }

    int FreeBlockSize = getsize(next->size_status);
      
    if(next==NULL)
        {
        return NULL;
        }
    else if(FreeBlockSize==allocsize){
        
        blockHeader * addr = next;
        next->size_status-=1;
         
        blockHeader* footer  =(blockHeader*) ((void*)next+allocsize-4);
        footer=NULL; // remove the footer

        // decrease size of heap accordingly 
        heapSize -= FreeBlockSize;
       
        return (void *)addr + 4;
    }
    else
        {
            //splitting allocated block
        blockHeader * addr = next;
            // allocation the header in the free space

        next->size_status = allocsize + x + 1;
            // splitting the freespace.
            //adding the header.
        blockHeader * header = (blockHeader*) ((void*)next+allocsize);
        header->size_status = (FreeBlockSize - allocsize + 2);
            //adding the footer
        blockHeader * footer = (blockHeader*) ((void*)next+(allocsize - 4));
        
        footer->size_status = allocsize;
            // decreasing the free heap space
        heapSize -= allocsize;
            // changing the next pointer to the next allocated block.
        
        next=(blockHeader*) ((void*)next+allocsize);
        return (void*)addr + 4;
            // adding the header by the shifting factor of 4
        } 
    return NULL;
    
} 



int getsize(int size){
    return size-(size%8);
}

// Helper Methods to get Status bit 


int getXbit(int size){
    int s=size-(size%8);
    s=size-s;
    if(s==2||s==3)return 1;
    else return 0;
}

int getYbit(int size){
    int s=size-(size%8);
    s=size-s;
    if(s==1||s==3)return 1;
    else return 0;
}

// Helper Function that coalesces to chunks of memory together
void coalesceHelper(blockHeader * first,blockHeader * second){
    
    if(first==NULL||second==NULL)
        return;

    int size1,size2,x1,x2,y1,y2;

    int total_size,final_x,final_y;

    size1 = getsize(first->size_status); //Size of first block
    size2 = getsize(second->size_status); //Size of second block

    
    x1 = getXbit(first->size_status); 
    x2 = getXbit(second->size_status);

    y1=getYbit(first->size_status);
    y2=getYbit(second->size_status);

    total_size=size1+size2; // total size of the blocks

    final_y=0;
    final_x=x1;

    // Adding header
    first->size_status = total_size + final_x*2 + final_y;
    
    //Setting the nextFit to header
    next=first;

    // Removes the footer of the first block
    first=(blockHeader*)((void*)first+size1-4);
    first=NULL;

    // changes the footer of the second block and removes the header of the first block
    blockHeader * footer= (blockHeader*)((void*)second+size2-4);
    footer->size_status=total_size;

    second=NULL;
        
}

// Helper function that's used to coalesce two blocks
void coalesce(){

    blockHeader * start= heapStart;
    //blockHeader * start2= heapStart;

    int counter = initHeapSize;
    while(counter--){

        start= heapStart;
       
        int found1=0;

        blockHeader * first=NULL;
        blockHeader * second=NULL;

        for(int i =0;i<=initHeapSize-1;i++){

            if(start==NULL)continue;

            int size= getsize(start->size_status);
            int a1= getYbit(start->size_status);

            if(a1==0){found1++;
                if(found1==1){
                    first=start;
                }
                if(found1==2){
                    second = start;
                }
            }
            if(a1==1)found1=0;
            
             
            // Checking for 2 non coalesced blocks, also checks for NULL values
            if(found1==2 && first!=NULL && second!=NULL){
                
                found1=0;
                
                coalesceHelper(first,second);// call to helper function that coalesces two blocks

                break;
            }
            //check if next block is the last block
            blockHeader * check = (blockHeader*)((void*)start+size);
            if(check->size_status==1)break;
            start=(blockHeader*)((void*)start+size);

        }
        
    }
}
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
 * - Update header(s) and footer as needed.
 */                    
int freeHeap(void *ptr) {  
      
    if(ptr==NULL)
        return -1; //returns -1 if ptr is NULL.

    if((unsigned int)ptr%8!=0)
        return -1; // returns -1 if ptr is not a multiple of 8.
        
    if(ptr<(void*)heapStart||ptr>=((void*)heapStart+allocsize))
        return -1; // returns -1 if ptr is outside of the heap space.

    blockHeader* cur=(blockHeader*)(ptr-4);
   
    if(getYbit(cur->size_status)==0)
        return -1;

    blockHeader * start =(blockHeader*)(ptr-4);
    
    
    start->size_status -= 1;
    coalesce(); // Call to coalesce helper Function
    return 0;
} 

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int initHeap(int sizeOfRegion) {  

    // assigning the required heapspace to the global variable.
    heapSize = sizeOfRegion-8;
    initHeapSize = sizeOfRegion-8;
    
    static int allocated_once = 0; //prevent multiple initHeap calls
    
    int pagesize;  // page size
    int padsize;   // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int fd;
    
    blockHeader* endMark;
    
    if (0 != allocated_once) {
        fprintf(stderr, 
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }
    
        // Get the pagesize
    pagesize = getpagesize();
    
        // Calculate padsize as the padding required to round up sizeOfRegion 
        // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;
    
    allocsize = sizeOfRegion + padsize;
    
        // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
        // alocate the block in the main file where it exists
    allocated_once = 1;
    
        // for double word alignment and end mark
    allocsize -= 8;
    
        // Initially there is only one big free block in the heap.
        // Skip first 4 bytes for double word alignment requirement.
    heapStart = (blockHeader*) mmap_ptr + 1;
    
        // Set the end mark
    endMark = (blockHeader*)((void*)heapStart + allocsize);
    endMark->size_status = 1;
    
        // Set size in header
    heapStart->size_status = allocsize;
    
        // Set p-bit as allocated in header
        // note a-bit left at 0 for free
    heapStart->size_status += 2;
    
        // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heapStart + allocsize - 4);
    footer->size_status = allocsize;
    next=heapStart;
    return 0;
} 

/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void dumpMem() {     
    int counter;
    char status[5];
    char p_status[5];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;
    
    blockHeader *current = heapStart;
    counter = 1;
    
    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;
    
    fprintf(stdout, "************************************Block list***\
            ********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, "-------------------------------------------------\
            --------------------------------\n");
    
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
        
        if (t_size & 1) {
                // LSB = 1 => used block
            strcpy(status, "used");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "Free");
            is_used = 0;
        }
        
        if (t_size & 2) {
            strcpy(p_status, "used");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "Free");
        }
        
        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;
        
        t_end = t_begin + t_size - 1;
        
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status, 
                p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
        
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }
    
    fprintf(stdout, "---------------------------------------------------\
            ------------------------------\n");
    fprintf(stdout, "***************************************************\
            ******************************\n");
    fprintf(stdout, "Total used size = %d\n", used_size);
    fprintf(stdout, "Total free size = %d\n", free_size);
    fprintf(stdout, "Total size = %d\n", used_size + free_size);
    fprintf(stdout, "***************************************************\
            ******************************\n");
    fflush(stdout);
    
    return;  
} 
