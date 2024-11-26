#include <inc/lib.h>


//Helper Functions

//check if a page is free
//1 == page is free
//0 == page is allocated
int isPageFree(uint32 va)
{
    uint32 pdx = PDX(va);
    uint32 ptx = PTX(va);

    // check if page directory is present (means that something created it)
    if (!(myEnv->env_page_directory[pdx] & PERM_PRESENT))
	{
        return 1;
    }

    // check if page table is present (means that something created it)
    uint32 *page_table = (uint32 *)myEnv->env_page_directory[pdx];
    if (!(page_table[ptx] & PERM_PRESENT))
	{
        return 1;
    }

    return 0;
}

// apply FF on the Uheap
// return va if success (uint32)
// return 0 if fail
uint32 find_free_pages_ff(uint32 start, uint32 end, uint32 numOfPages)
{
    uint32 freePages = 0;
    uint32 firstPageAddr = 0;

    for (uint32 addr = start; addr < end; addr += PAGE_SIZE)
	{
        if (isPageFree(addr))
		{
            if (freePages == 0)
			{
            	firstPageAddr = addr;
			}
            
			freePages++;

            if (freePages >= numOfPages)
			{
                return firstPageAddr;
			}
        }
		else
		{
        	freePages = 0;
		}
    }
    return 0;
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void *sbrk(int increment)
{
	return (void *)sys_sbrk(increment);
}


struct freePagesEntry
{
	uint32 startAddr;
	uint32 pagesCount;
};

#define MAX_FREE_ENTRIES 131072		//(USER_HEAP_MAX - USER_HEAP_START)/PAGE_SIZE
struct freePagesEntry freeList[MAX_FREE_ENTRIES];
uint32 freeListSize = -1;

void init_free_list(){
	freeListSize = 1;
	freeList[0].startAddr = ROUNDUP((uint32)(myEnv->limit),PAGE_SIZE) + PAGE_SIZE;
	freeList[0].pagesCount = (USER_HEAP_MAX - freeList[0].startAddr) / PAGE_SIZE;
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	cprintf("ittt in malloc()\n");
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	// panic("malloc() is not implemented yet...!!");
	
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
	if(!sys_isUHeapPlacementStrategyFIRSTFIT())
		return NULL;

	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		return alloc_block_FF(size);
	}

	if(freeListSize == -1){	// first allocation
		init_free_list();
		// cprintf("freelistnum of pages int init= %d\n",freeList[0].pagesCount);
	}


	uint32 numOfPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	// cprintf("ittt in malloc(), number of pages: %d\n",numOfPages);
	for (uint32 i = 0; i < freeListSize; i++)
	{
		if(freeList[i].pagesCount >= numOfPages){
			uint32 returAddr = freeList[i].startAddr;
			sys_allocate_user_mem(freeList[i].startAddr, size);

			if(freeList[i].pagesCount > numOfPages){
				freeList[i].pagesCount -= numOfPages;
				freeList[i].startAddr += numOfPages * PAGE_SIZE;
				
			}
			else {
				// remove the current entry
				freeListSize--;
				for(; i < freeListSize; i++){
					freeList[i] = freeList[i+1];
				}
			}
				
			// cprintf("freelistsize = %d\n",freeListSize);
			// cprintf("freelistnum of addr = %d\n",returAddr);
			cprintf("return done\n");
			return (void*)returAddr;
			cprintf("return not done\n");

		}
	}
	
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	//  Write your code here, remove the panic and write your code
	//  panic("free() is not implemented yet...!!");
	if ((uint32)virtual_address < USER_HEAP_START || (uint32)virtual_address > USER_HEAP_MAX)
		panic("INVALID ADDRESS\n");
	if ((uint32)virtual_address < USER_LIMIT)
	{
		free_block(virtual_address);
		return;
	}
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void *smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	// DON'T CHANGE THIS CODE========================================
	if (size == 0)
		return NULL;
	//==============================================================
	// TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	//return NULL;
	
	uint32 pgAllocStartArea = (uint32)myEnv->limit + PAGE_SIZE;
	uint32 pgAllocEndArea = (uint32)USER_HEAP_MAX;
	uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	uint32 firstPageAddr = find_free_pages_ff(pgAllocStartArea, pgAllocEndArea, numOfPages);
	
	if (firstPageAddr == 0)
		return NULL;
	else
	{
		sys_createSharedObject(sharedVarName,size, isWritable, (void*)firstPageAddr);
		return (void *)firstPageAddr;
	}
	
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void *sget(int32 ownerEnvID, char *sharedVarName)
{
	// TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	//  Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	//  Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;
}

//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");
}
void shrink(uint32 newSize)
{
	panic("Not Implemented");
}
void freeHeap(void *virtual_address)
{
	panic("Not Implemented");
}
