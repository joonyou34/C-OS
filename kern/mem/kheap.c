#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

#define MAX_FREE_ENTRIES (uint32)(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE

struct FreePageEntry
{
	uint32 startAddr;
	uint32 freePages;
};
struct FreePageEntry free_page_list[MAX_FREE_ENTRIES];
int32 freeListSize = -1;
uint32 physToVirt[0xFFFFF000 / 4096];

void initialize_free_list()
{
	freeListSize = 1;
	free_page_list[0].startAddr = ROUNDUP((uint32)limit, PAGE_SIZE) + PAGE_SIZE;
	free_page_list[0].freePages = ROUNDDOWN(KERNEL_HEAP_MAX - free_page_list[0].startAddr, PAGE_SIZE) / PAGE_SIZE;
}

// Initialize the dynamic allocator of kernel heap with the given start address, size & limit
// All pages in the given range should be allocated
// Remember: call the initialize_dynamic_allocator(..) to complete the initialization
// Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	// TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	//  Write your code here, remove the panic and write your code
	// panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

	start = (uint32 *)ROUNDDOWN(daStart, PAGE_SIZE);
	brk = (uint32 *)ROUNDUP((uint32)((char *)start + initSizeToAllocate), PAGE_SIZE);
	limit = (uint32 *)ROUNDUP(daLimit, PAGE_SIZE);

	if ((uint32)limit > KERNEL_HEAP_MAX || (uint32)start < KERNEL_HEAP_START || (uint32)brk > (uint32)limit)
	{
		panic("Unavailable address");
	}

	uint32 finish;
	finish = ROUNDUP((uint32)brk, PAGE_SIZE);

	for (uint32 i = (uint32)start; i < finish; i += PAGE_SIZE)
	{

		uint32 *ptr;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, i, &ptr);

		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM)
		{
			panic("No Memory available");
		}

		// int perms = pt_get_page_permissions(ptr_page_directory,i);
		ret = map_frame(ptr_page_directory, ptr_frame_info, i, PERM_WRITEABLE); // adjust perms

		if (ret == E_NO_MEM)
			panic("No Memory for page table");
		uint32 frameNum = to_physical_address(ptr_frame_info) / PAGE_SIZE;
		physToVirt[frameNum] = i;
	}
	initialize_dynamic_allocator(daStart, initSizeToAllocate);
	return 0;
}

void *sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	// MS2: COMMENT THIS LINE BEFORE START CODING==========
	//  return (void*)-1 ;
	//====================================================

	// TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	//  Write your code here, remove the panic and write your code
	//  panic("sbrk() is not implemented yet...!!");

	if (!numOfPages)
		return brk;
	uint32 numBytes = numOfPages * PAGE_SIZE;
	uint32 *finish = (uint32 *)(((char *)brk) + numBytes);
	if (finish > limit)
		return (void *)-1;

	for (uint32 va = (uint32)brk; (uint32 *)va < finish; va += PAGE_SIZE)
	{
		uint32 *ptr;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, va, &ptr);

		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM)
			return (void *)-1;

		ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
		uint32 frameNum = to_physical_address(ptr_frame_info) / PAGE_SIZE;
		physToVirt[frameNum] = va;
	}
	uint32 *old = brk;
	brk = finish;
	return (void *)old;
}

// TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void *kmalloc(unsigned int size)
{
	// TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	//  Write your code here, remove the panic and write your code
	// kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	if (freeListSize == -1)
		initialize_free_list();
	if (!isKHeapPlacementStrategyFIRSTFIT())
		return NULL;
	if (size == 0 || size > (KERNEL_HEAP_MAX - ((uint32)limit + PAGE_SIZE)))
	{
		return NULL;
	}
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		void *ptr = alloc_block_FF(size);
		if ((uint32)ptr < KERNEL_HEAP_START || (uint32)ptr >= KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE)
			cprintf("address out of bounds for block allocator\n");
		return ptr;
	}

	else
	{
		uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

		for (uint32 i = 0; i < freeListSize; i++)
		{
			if (free_page_list[i].freePages >= numOfPages)
			{
				uint32 firstPageAddr = free_page_list[i].startAddr;

				for (uint32 j = 0; j < numOfPages; j++)
				{
					struct FrameInfo *frame_info = NULL;
					uint32 addr = firstPageAddr + (j * PAGE_SIZE);
					if (allocate_frame(&frame_info) == E_NO_MEM || map_frame(ptr_page_directory, frame_info, addr, PERM_WRITEABLE) == E_NO_MEM)
					{
						// fail
						// for (uint32 rollbackIndex = 0; rollbackIndex < j; rollbackIndex++)
						// {
						// 	unmap_frame(ptr_page_directory, firstPageAddr + (rollbackIndex * PAGE_SIZE));
						// }
						return NULL;
					}

					// setting 9th bit (unused bit) of bufferedVA (to use it in kfree)
					if (j == numOfPages - 1)
					{
						uint32 *table;
						get_page_table(ptr_page_directory, addr, &table);

#define LAST_PAGE 512 // 9TH bit (set when it is the end page in allocation)
						table[PTX(addr)] |= LAST_PAGE;
					}
				}

				if (free_page_list[i].freePages == numOfPages)
				{
					// if fully allocated (remove entry)
					freeListSize--;
					for (; i < freeListSize; i++)
					{
						free_page_list[i] = free_page_list[i + 1];
					}
				}
				else
				{
					free_page_list[i].startAddr += numOfPages * PAGE_SIZE;
					free_page_list[i].freePages -= numOfPages;
				}

				// cprintf("FIRST FIT STRAT(AT SUCCESS): %d\n", isKHeapPlacementStrategyFIRSTFIT());
				return (void *)firstPageAddr;
			}
		}

		return NULL;
	}
}

void kfree(void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	//  Write your code here, remove the panic and write your code
	// panic("kfree() is not implemented yet...!!");

	// you need to get the size of the given allocation using its address
	// refer to the project presentation and documentation for details

	if (!isKHeapPlacementStrategyFIRSTFIT())
		return;
	if ((uint32)virtual_address < KERNEL_HEAP_START || (uint32)virtual_address >= KERNEL_HEAP_MAX)
	{
		panic("Invalid address to free, outside kernel heap bounds");
	}

	if ((uint32)virtual_address < (uint32)limit)
	{

		if ((uint32)virtual_address < (uint32)brk)
			free_block(virtual_address);

		return;
	}

	virtual_address = (void *)ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);

	uint32 pages_count = 0;
	for (uint32 addr = (uint32)virtual_address;; addr += PAGE_SIZE)
	{
		pages_count++;
		uint32 *table;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, addr, &table);

		if (ptr_frame_info == NULL)
		{
			panic("Cannot free an already freed page");
		}

		if (table[PTX(addr)] & LAST_PAGE)
		{
			// cprintf("KHeapStart: %d \t KHeapMax: %d \t ", KERNEL_HEAP_START, KERNEL_HEAP_MAX);
			// cprintf("pages count: %d \n", (KERNEL_HEAP_START - KERNEL_HEAP_MAX)/PAGE_SIZE);
			table[PTX(addr)] &= ~LAST_PAGE; // reset the bit

			free_frame(ptr_frame_info);
			unmap_frame(ptr_page_directory, addr);

			// update free_page_list
			uint32 page_before = (uint32)virtual_address - PAGE_SIZE;
			uint32 page_after = addr + PAGE_SIZE;

			uint32 i = 0;
			for (; i < freeListSize; i++)
			{
				if (free_page_list[i].startAddr > addr)
					break;
			}

			if (i > 0 && ROUNDDOWN(free_page_list[i - 1].startAddr, PAGE_SIZE) + free_page_list[i - 1].freePages * PAGE_SIZE == (uint32)virtual_address)
			{
				free_page_list[i - 1].freePages += pages_count;

				if (i < freeListSize && ROUNDDOWN(free_page_list[i - 1].startAddr, PAGE_SIZE) + free_page_list[i - 1].freePages * PAGE_SIZE == ROUNDDOWN(free_page_list[i].startAddr, PAGE_SIZE))
				{
					freeListSize--;
					free_page_list[i - 1].freePages += free_page_list[i].freePages;
					// delete the entry
					for (; i < freeListSize; i++)
						free_page_list[i] = free_page_list[i + 1];
				}
			}
			else if (i < freeListSize && (uint32)virtual_address + pages_count * PAGE_SIZE == ROUNDDOWN(free_page_list[i].startAddr, PAGE_SIZE))
			{
				free_page_list[i].freePages += pages_count;
				free_page_list[i].startAddr = (uint32)virtual_address;
			}

			else
			{
				for (uint32 j = freeListSize; j > i; j--)
				{
					free_page_list[j] = free_page_list[j - 1];
				}

				freeListSize++;
				free_page_list[i].freePages = pages_count;
				free_page_list[i].startAddr = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);
			}
			return;
		}

		free_frame(ptr_frame_info);
		unmap_frame(ptr_page_directory, addr);
	}
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	//  Write your code here, remove the panic and write your code
	// panic("kheap_physical_address() is not implemented yet...!!");
	uint32 offset = PGOFF(virtual_address);
	uint32 *pgTablePtr = NULL;
	get_page_table(ptr_page_directory, virtual_address, &pgTablePtr);
	if (pgTablePtr == NULL)
		return 0;
	uint32 pgTableEntry = pgTablePtr[PTX(virtual_address)];
	if (!(pgTableEntry & PERM_PRESENT))
		return 0;
	uint32 frameNum = pgTableEntry >> 12;
	uint32 pa = frameNum * PAGE_SIZE + offset;
	return pa;
	// get corresponding frame number
	// return the physical address corresponding to given virtual_address
	// refer to the project presentation and documentation for details

	// EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	// TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	//  Write your code here, remove the panic and write your code
	// panic("kheap_virtual_address() is not implemented yet...!!");
	uint32 frameNum = physical_address / PAGE_SIZE;
	uint32 offset = physical_address % PAGE_SIZE;
	uint32 virtualAddressBase = physToVirt[frameNum];

	if (virtualAddressBase == 0)
		return 0;

	return virtualAddressBase + offset;

	// return the virtual address corresponding to given physical_address
	// refer to the project presentation and documentation for details

	// EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	// TODO LOGIC
	// if less than 2kb = realloc
	// else if more kmalloc, kfree
	// else if 0 = kfree
	// else if address == null = kmalloc

	if (virtual_address == NULL)
	{
		if (new_size > DYN_ALLOC_MAX_BLOCK_SIZE)
			return kmalloc(new_size);

		return alloc_block_FF(new_size);
	}

	if (new_size == 0)
	{
		kfree(virtual_address);
		return NULL;
	}

	uint32 currsize;
	bool isBlock = 0;

	if ((uint32)virtual_address <= (uint32)limit)
	{ // it was block
		currsize = get_block_size(virtual_address);
		isBlock = 1;
	}

	else
	{ // it was groub of pages
		uint32 cntr = 0;
		for (uint32 addr = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);; addr += PAGE_SIZE)
		{

			uint32 *table;
			struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, addr, &table);

			// gave a wrong va?
			if (ptr_frame_info == NULL)
			{
				return NULL;
			}

			if (table[PTX(addr)] & LAST_PAGE)
			{
				break;
			}

			cntr++;
		}
		currsize = PAGE_SIZE * cntr;
	}

	uint32 min_size = MIN(currsize, new_size);

	if (new_size > DYN_ALLOC_MAX_BLOCK_SIZE) /*it will be groub pages*/
	{
		// allocate block using kmalloc
		//  if allocated then copy the memory and free the old
		//  else then return null

		void *va = kmalloc(new_size);

		if (va != NULL)
		{
			memcpy(va, virtual_address, min_size);
			if (isBlock)
				free_block(virtual_address);
			else
				kfree(virtual_address);
		}
		return va;
	}
	else /*it will be block*/
	{
		// alocate the block using allocff
		void *new_block = alloc_block_FF(new_size);
		if (new_block != NULL)
		{
			memcpy(new_block, virtual_address, min_size);
			if (isBlock)
				free_block(virtual_address);
			else
				kfree(virtual_address);
		}
		return new_block;
	}
}

/*

void *krealloc(void *virtual_address, uint32 new_size)
{
	// TODO LOGIC
	// if less than 2kb = realloc
	// else if more kmalloc, kfree
	// else if 0 = kfree
	// else if address == null = kmalloc
	panic("krealloc not imp!");
	// if(virtual_address == NULL)
	// {
	// 	if(new_size == 0)
	// 		return NULL;
	// 	if(new_size > DYN_ALLOC_MAX_BLOCK_SIZE)
	// 		return kmalloc(new_size);
	// 	else
	// 		return alloc_block_FF(new_size);
	// }

	// if(new_size == 0)
	// {
	// 	kfree(virtual_address);
	// 	return NULL;
	// }

	// if(new_size > DYN_ALLOC_MAX_BLOCK_SIZE) //it was pages and going to pages
// {
// 	//allocate block using kmalloc
// 	// if allocated then copy the memory and free the old
// 	// else then return null

// 	void* va = kmalloc(new_size);

// 	if(va == NULL)
// 	{
// 		return NULL;
// 	}
// 	else
// 	{
// 		uint32 numOfPages = 0;
// 		uint32* table;
// 		get_frame_info(ptr_page_directory, virtual_address, &table);

// 		while(1){
// 			uint32* curr_table;
// 			struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, virtual_address, &curr_table);
// 			if(ptr_frame_info == NULL || table != curr_table){
// 				break;
// 			}
// 			numOfPages++;
// 		}
// 		uint32 oldSize = numOfPages * PAGE_SIZE;
// 		memcpy(va, virtual_address, oldSize);
// 		kfree(virtual_address);
// 	}
// }
// else //it was pages and going to block
// {
// 	//alocate the block using allocff
// 	void* new_block = alloc_block_FF(new_size);
// 	if(new_block==NULL)
// 	{
// 		return NULL;
// 	}
// 	else
// 	{
// 		uint32 numOfPages = 0;
// 		uint32* table;
// 		get_frame_info(ptr_page_directory, virtual_address, &table);

// 		while(1){
// 			uint32* curr_table;
// 			struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, virtual_address, &curr_table);
// 			if(ptr_frame_info == NULL || table != curr_table){
// 				break;
// 			}
// 			numOfPages++;
// 		}
// 		uint32 oldSize = numOfPages * PAGE_SIZE;
// 		memcpy(new_block, virtual_address, oldSize);
// 		free_block(virtual_address);
// 		return new_block;
// 	}
// }
}

*/