#include <inc/lib.h>

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



struct Page_Block_Entry
{
	uint32 startAddr;
	uint32 size;
};

#define MAX_FREE_ENTRIES 131072		//(USER_HEAP_MAX - USER_HEAP_START)/PAGE_SIZE
struct Page_Block_Entry freeList[MAX_FREE_ENTRIES];
uint32 freeListSize = -1;

void init_free_list(){
	freeListSize = 1;
	freeList[0].startAddr = ROUNDUP((uint32)(myEnv->limit),PAGE_SIZE) + PAGE_SIZE;
	freeList[0].size = ROUNDDOWN(USER_HEAP_MAX - freeList[0].startAddr,PAGE_SIZE);
}



struct Page_Block_Entry allocatedList[MAX_FREE_ENTRIES];
uint32 allocatedListSize = 0;

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	// cprintf("ittt in malloc()\n");
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
	}


	uint32 numOfPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	for (uint32 i = 0; i < freeListSize; i++)
	{
		if(freeList[i].size >= size){
			uint32 returAddr = freeList[i].startAddr;
			sys_allocate_user_mem(freeList[i].startAddr, size);

			allocatedList[allocatedListSize].size = ROUNDUP(size,PAGE_SIZE);
			allocatedList[allocatedListSize].startAddr = ROUNDDOWN(freeList[i].startAddr,PAGE_SIZE);
			allocatedListSize++;

			size = ROUNDUP(size,PAGE_SIZE);
			if(freeList[i].size > size){
				freeList[i].size -= size;
				freeList[i].startAddr += size;
				
			}
			else {
				// remove the current entry
				freeListSize--;
				for(; i < freeListSize; i++){
					freeList[i] = freeList[i+1];
				}
			}
			return (void*)returAddr;

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
	if ((uint32)virtual_address < USER_HEAP_START || (uint32)virtual_address >= USER_HEAP_MAX)
		panic("INVALID ADDRESS\n");
	if ((uint32)virtual_address < (uint32)(myEnv->limit))
	{
		free_block(virtual_address);
		return;
	}

	for (uint32 i = 0; i < allocatedListSize; i++)
	{
		if(allocatedList[i].startAddr == (uint32)virtual_address){
			sys_free_user_mem(allocatedList[i].startAddr, allocatedList[i].size);

						
			uint32 j = 0;
			for(; j < freeListSize; j++){
				if(freeList[j].startAddr > allocatedList[i].startAddr){
					break;
				}
			}
			
			// merge with the back block
			if(j > 0 && ROUNDUP(freeList[j-1].startAddr + freeList[j-1].size, PAGE_SIZE) == ROUNDDOWN(allocatedList[i].startAddr, PAGE_SIZE)){
				
				freeList[j-1].size += allocatedList[i].size;
				
				// merge back and next
				if(ROUNDUP(freeList[j-1].startAddr + freeList[j-1].size,PAGE_SIZE) == ROUNDDOWN(freeList[j].startAddr,PAGE_SIZE)){ 
					freeListSize--;
					freeList[j-1].size += freeList[j].size;
					for(; j<freeListSize; j++){
						freeList[j] = freeList[j+1];
					}
				}
			}

			// merge with the next block
			else if(j < freeListSize && ROUNDUP(allocatedList[i].startAddr + allocatedList[i].size,PAGE_SIZE) == ROUNDDOWN(freeList[j].startAddr,PAGE_SIZE)){
				freeList[j].size += allocatedList[i].size;
				freeList[j].startAddr = allocatedList[i].startAddr;
			}

			else
			{
				for(uint32 k = freeListSize; k > j; k--){
					freeList[k] = freeList[k-1];
				}

				freeListSize++;

				freeList[j] = allocatedList[i];
			}

			// delete the entry
			allocatedListSize--;
			for(; i < allocatedListSize; i++)
				allocatedList[i] = allocatedList[i+1];
			
			return;
		}
		
	}

	panic("INVALID ADDRESS\n");
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

	if(freeListSize == -1)
	{
		init_free_list();
	}

	
	uint32 numOfPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	for (uint32 i = 0; i < freeListSize; i++)
	{
		if(freeList[i].size >= size){
			uint32 returAddr = freeList[i].startAddr;
			int32 returnid = sys_createSharedObject(sharedVarName,size, isWritable, (void*)returAddr);
			if(returnid == E_SHARED_MEM_EXISTS || returnid == E_NO_SHARE)
			{
				return NULL;
			}
			allocatedList[allocatedListSize].size = ROUNDUP(size,PAGE_SIZE);
			allocatedList[allocatedListSize].startAddr = ROUNDDOWN(freeList[i].startAddr,PAGE_SIZE);
			allocatedListSize++;

			size = ROUNDUP(size,PAGE_SIZE);
			if(freeList[i].size > size){
				freeList[i].size -= size;
				freeList[i].startAddr += size;
				
			}
			else {
				// remove the current entry
				freeListSize--;
				for(; i < freeListSize; i++){
					freeList[i] = freeList[i+1];
				}
			}
			return (void*)returAddr;
		}
	}
	
	return NULL;
	
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void *sget(int32 ownerEnvID, char *sharedVarName)
{
	// TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	//  Write your code here, remove the panic and write your code
	cprintf("I am here mortal...\n");
	uint32 size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	cprintf("size is: %u\n", size);
	if(size == 0)
	{
		return NULL;
	}

	if(freeListSize == -1)
	{
		init_free_list();
	}


	int32 numOfPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	for (uint32 i = 0; i < freeListSize; i++)
	{
		if(freeList[i].size >= size){
			uint32 returAddr = freeList[i].startAddr;
			uint32 returnid = sys_getSharedObject(ownerEnvID, sharedVarName, (void*)returAddr);
			if(returnid == E_SHARED_MEM_NOT_EXISTS)
			{
				return NULL;
			}
			allocatedList[allocatedListSize].size = ROUNDUP(size,PAGE_SIZE);
			allocatedList[allocatedListSize].startAddr = ROUNDDOWN(freeList[i].startAddr,PAGE_SIZE);
			allocatedListSize++;

			size = ROUNDUP(size,PAGE_SIZE);
			if(freeList[i].size > size){
				freeList[i].size -= size;
				freeList[i].startAddr += size;
				
			}
			else {
				// remove the current entry
				freeListSize--;
				for(; i < freeListSize; i++){
					freeList[i] = freeList[i+1];
				}
			}
			return (void*)returAddr;
		}
	}
	
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
