#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

#define KALLOC_START (uint32)limit + PAGE_SIZE

uint32 physToVirt[0xFFFFF000 / 4096];
uint32 lstAddr = 0;
uint32 lstPageCt = -1;
struct spinlock klock;

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
		{
			panic("No Memory for page table");
		}
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
	{
		return brk;
	}
	uint32 numBytes = numOfPages * PAGE_SIZE;
	
	uint32 *finish = (uint32 *)(((char *)brk) + numBytes);
	if (finish > limit)
	{
		return (void *)-1;
	}

	for (uint32 va = (uint32)brk; (uint32 *)va < finish; va += PAGE_SIZE)
	{
		uint32 *ptr;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, va, &ptr);

		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM)
		{
			return (void *)-1;
		}

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
	if (size == 0 || size > (KERNEL_HEAP_MAX - ((uint32)limit + PAGE_SIZE)))
	{
		return NULL;
	}
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		acquire_spinlock(&klock);
		void *ptr = alloc_block_FF(size);
		if ((uint32)ptr < KERNEL_HEAP_START || (uint32)ptr >= KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE)
			cprintf("address out of bounds for block allocator\n");
    	release_spinlock(&klock);
		return ptr;
	}
	else
	{
		acquire_spinlock(&klock);
		uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		if(numOfPages < lstPageCt) {
			lstPageCt = numOfPages;
			lstAddr = KALLOC_START;
		}
		uint32 pgAllocStartArea = lstAddr;
		uint32 pgAllocEndArea = KERNEL_HEAP_MAX;
		uint32 pagesFound = 0, firstPageAddr = 0;

		
		// get the first Page Address
		for (uint32 addr = pgAllocStartArea; addr < pgAllocEndArea; addr += PAGE_SIZE)
		{
			uint32 *ptr_table;
			struct FrameInfo *frame_info = get_frame_info(ptr_page_directory, addr, &ptr_table);
			if (frame_info == NULL)
			{
				if (pagesFound == 0)
					firstPageAddr = addr;
				pagesFound++;
				if (pagesFound == numOfPages)
					break;
			}
			else
			{
				pagesFound = 0;
				firstPageAddr = 0;
			}
		}

		if (pagesFound < numOfPages)
		{
    		release_spinlock(&klock);
			return NULL;
		}

		for (uint32 addr = firstPageAddr; addr < (firstPageAddr + numOfPages * PAGE_SIZE); addr += PAGE_SIZE)
		{
			struct FrameInfo *frame_info = NULL;
			if (allocate_frame(&frame_info) || map_frame(ptr_page_directory, frame_info, addr, PERM_WRITEABLE))
			{
				// Roll back if allocation fails
				for (uint32 rollback_addr = firstPageAddr; rollback_addr < addr; rollback_addr += PAGE_SIZE)
				{
					unmap_frame(ptr_page_directory, rollback_addr);
				}

   				release_spinlock(&klock);
				return NULL;
			}
			uint32 frameNum = to_physical_address(frame_info) / PAGE_SIZE;
			physToVirt[frameNum] = addr;

			// setting 9th bit (unused bit) of address (to use it in kfree)
			if (addr == (firstPageAddr + (numOfPages - 1) * PAGE_SIZE))
			{
				uint32 *table;
				get_page_table(ptr_page_directory, addr, &table);

				#define LAST_PAGE 512 // 9TH bit (set when it is the end page in allocation)
				table[PTX(addr)] |= LAST_PAGE;
				lstAddr = addr + PAGE_SIZE;
				lstPageCt = numOfPages;
			}
		}

    	release_spinlock(&klock);
		return (void *)firstPageAddr;
	}
}

void kfree(void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	//  Write your code here, remove the panic and write your code
	// panic("kfree() is not implemented yet...!!");

	// you need to get the size of the given allocation using its address
	// refer to the project presentation and documentation for details

	if ((uint32)virtual_address < KERNEL_HEAP_START || (uint32)virtual_address >= KERNEL_HEAP_MAX)
	{
		panic("Invalid address to free, outside kernel heap bounds");
	}
	acquire_spinlock(&klock);
	if ((uint32)virtual_address < (uint32)limit)
	{

		if ((uint32)virtual_address < (uint32)brk)
			free_block(virtual_address);
		release_spinlock(&klock);
		return;
	}
	uint32 st = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);
	
	for (uint32 addr = st;; addr += PAGE_SIZE)
	{
		uint32 *table;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, addr, &table);

		if (ptr_frame_info == NULL)
		{
			lstPageCt = -1;

			panic("Cannot free an already freed page");
		}

		uint32 frameNum = to_physical_address(ptr_frame_info) / PAGE_SIZE;
		physToVirt[frameNum] = 0;

		if (table[PTX(addr)] & LAST_PAGE)
		{
			table[PTX(addr)] &= ~LAST_PAGE; // reset the bit

			free_frame(ptr_frame_info);
			unmap_frame(ptr_page_directory, addr);
			break;
		}

		free_frame(ptr_frame_info);
		unmap_frame(ptr_page_directory, addr);
	}

	if(st < lstAddr)
		lstPageCt = -1;
	release_spinlock(&klock);
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	//  Write your code here, remove the panic and write your code
	// panic("kheap_physical_address() is not implemented yet...!!");
	acquire_spinlock(&klock);
	uint32 offset = PGOFF(virtual_address);
	uint32 *pgTablePtr = NULL;
	get_page_table(ptr_page_directory, virtual_address, &pgTablePtr);
	if (pgTablePtr == NULL)
	{
		release_spinlock(&klock);
		return 0;
	}
	uint32 pgTableEntry = pgTablePtr[PTX(virtual_address)];
	if (!(pgTableEntry & PERM_PRESENT))
	{
		release_spinlock(&klock);
		return 0;
	}
	uint32 frameNum = pgTableEntry >> 12;
	uint32 pa = frameNum * PAGE_SIZE + offset;
	release_spinlock(&klock);
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
	uint32 frameNum = physical_address / PAGE_SIZE; // Extract frame number
	uint32 offset = physical_address % PAGE_SIZE;	// Extract offset within the page
	acquire_spinlock(&klock);
	uint32 virtualAddressBase = physToVirt[frameNum];
	release_spinlock(&klock);
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
		return kmalloc(new_size);
	}

	if (new_size == 0)
	{
		kfree(virtual_address);
		return NULL;
	}


	uint32 currsize;
	bool isBlock = 0;
	uint32 cntr = 0;

	if ((uint32)virtual_address <= (uint32)limit)
	{
		// it was block
		currsize = get_block_size(virtual_address) - sizeof(int) * 2;
		isBlock = 1;
	}
	else
	{
		// it was group of pages
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

	if (new_size > DYN_ALLOC_MAX_BLOCK_SIZE) /*it will be group pages*/
	{
		if(isBlock) /*if it was block*/
		{
			void *va = kmalloc(new_size);

			if (va != NULL)
			{
				memcpy(va, virtual_address, min_size);
				free_block(virtual_address);
			}
			return va;
		}
		else
		{
			uint32 newpages = ROUNDUP(new_size, PAGE_SIZE) / PAGE_SIZE;
			if(cntr > newpages) /*shrink*/
			{
				kfree(virtual_address + ROUNDUP(new_size, PAGE_SIZE));
				uint32 newendva = (uint32)virtual_address + (newpages - 1) * PAGE_SIZE;
				uint32 *table;
				get_page_table(ptr_page_directory, newendva, &table);
				table[PTX(newendva)] |= LAST_PAGE;
			}
			else if(cntr < newpages) /*enlarge*/
			{
				uint32 startva = (uint32)virtual_address + currsize;
				uint32 numOfPages = newpages - cntr;
				uint8 success = 1;
				for(uint32 addr = startva; addr < KERNEL_HEAP_MAX; addr += PAGE_SIZE)
				{
					uint32 *ptr_table;
					struct FrameInfo *frame_info = get_frame_info(ptr_page_directory, addr, &ptr_table);
					if (frame_info == NULL)
					{
						if(allocate_frame(&frame_info) || map_frame(ptr_page_directory,frame_info,addr, PERM_USER | PERM_WRITEABLE | PERM_PRESENT))
						{
							for(uint32 revaddr = addr; revaddr > startva; revaddr -= PAGE_SIZE)
							{
								free_frame(frame_info);
								unmap_frame(ptr_page_directory, revaddr);
							}
							success = 0;
							break;
						}
					}
					else
					{
						for(uint32 revaddr = addr; revaddr > startva; revaddr -= PAGE_SIZE)
						{
							free_frame(frame_info);
							unmap_frame(ptr_page_directory, revaddr);
						}
						success = 0;
						break;
					}

				}
				if(success)
				{
					uint32 newendva = (uint32)virtual_address + (newpages - 1) * PAGE_SIZE;
					uint32 *table2;
					get_page_table(ptr_page_directory, newendva, &table2);
					table2[PTX(newendva)] |= LAST_PAGE;

					uint32 prevpage = startva - PAGE_SIZE;
					uint32 *table;
					get_page_table(ptr_page_directory, prevpage, &table);
					table[PTX(prevpage)] &= ~LAST_PAGE;
					return virtual_address;
				}
				else
				{
					//reallocate
					void *va = kmalloc(new_size);
					if (va != NULL)
					{
						memcpy(va, virtual_address, min_size);
						kfree(virtual_address);
					}
					return va;
				}
			}
			else
			{
				return virtual_address;
			}
			return NULL;
		}
	}
	else /*it will be block*/
	{
		if(isBlock) /*if i was a block*/
		{
			return realloc_block_FF(virtual_address,new_size);
		}
		else /*if i was pages*/
		{
			void *new_block = alloc_block_FF(new_size);
			if (new_block != NULL)
			{
				memcpy(new_block, virtual_address, min_size);
				kfree(virtual_address);
			}
			return new_block;
		}
	}
}
