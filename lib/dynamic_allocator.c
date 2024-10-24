/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...

	LIST_INIT(&freeBlocksList);
	
	uint32 actualSize = initSizeOfAllocatedSpace-2*sizeof(int);
	uint32* it = (uint32 *)daStart;
	//begin block
	*it = 1;
	it++;
	//header
	*it = actualSize;

	it++;
	LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement *)it);

	it = (uint32 *)(daStart + initSizeOfAllocatedSpace - sizeof(int));
	//end block
	*it = 1;
	it--;
	//footer
	*it = actualSize;
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("set_block_data is not implemented yet");
	//Your Code is Here...

	char* address = (char *)va;
	//modify header
	*((uint32*)(address-sizeof(int))) = ((totalSize) | isAllocated);
	
	//modify footer
	*((uint32 *)(address + totalSize - 2*sizeof(int))) = ((totalSize) | isAllocated);
}

void* sbrkBytes(uint32 sz) {
	return (void *)-1;
	// return sbrk(ROUNDUP(sz, PAGE_SIZE)/PAGE_SIZE);
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...

	if(size == 0)
		return NULL;
	size += 2*sizeof(int);
	struct BlockElement* it;
	LIST_FOREACH(it, &freeBlocksList) {
		uint32 blockSz = get_block_size(it);
		//first block found
		if(blockSz >= size) {
			char* address = (char*) (it);
			//see if can split
			if(blockSz-size >= 4*sizeof(int)) {
				blockSz -= size;
				struct BlockElement* newBlockPtr = (struct BlockElement*) (address + size);
				set_block_data(newBlockPtr, blockSz, 0);
				LIST_INSERT_AFTER(&freeBlocksList, it, newBlockPtr);
			}
			else
				size = blockSz;
			set_block_data(it, size, 1);
			LIST_REMOVE(&freeBlocksList, it);
			return it;
		}
	}
	//REVISE AFTER SBRK
	if(sbrkBytes(size) == (void*) -1)
		return NULL;
		
	return alloc_block_FF(size - 2*sizeof(int));
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}

	if(size == 0)
		return NULL;
	size += 2*sizeof(int);
	struct BlockElement* it;
	struct BlockElement* best = NULL;
	uint32 mnSz = -1;
	LIST_FOREACH(it, &freeBlocksList) {
		uint32 blockSz = get_block_size(it);
		//candidate block
		if(blockSz >= size) {
			if(best == NULL || blockSz < mnSz) {
				best = it;
				mnSz = blockSz;
			}
		}
	}

	if(best != NULL) {
		char* address = (char*) (best);
			//see if can split
			if(mnSz-size >= 4*sizeof(int)) {
				mnSz -= size;
				struct BlockElement* newBlockPtr = (struct BlockElement*) (address + size);
				set_block_data(newBlockPtr, mnSz, 0);
				LIST_INSERT_AFTER(&freeBlocksList, best, newBlockPtr);
			}
			else
				size = mnSz;
			set_block_data(best, size, 1);
			LIST_REMOVE(&freeBlocksList, best);
			return best;
	}

	//REVISE AFTER SBRK
	if(sbrkBytes(size) == (void*) -1)
		return NULL;
	return alloc_block_FF(size - 2*sizeof(int));
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("free_block is not implemented yet");
	//Your Code is Here...

	if(va == NULL || is_free_block(va)) return;

	uint32 size = get_block_size(va);
	set_block_data(va, size, 0);

	char* address = (char *)va;
	uint32 metadataIt = *((uint32 *)(address - 2*sizeof(int)));
	uint32 prevSize = 0;

	char* mergedAddress = NULL;
	if(((metadataIt)&1) == 0) {
		prevSize = (metadataIt&(~0x1));
		mergedAddress = address - prevSize;
		set_block_data(mergedAddress, size + prevSize, 0);
	}

	char* nxtAddress = ((address + size));
	if(is_free_block(nxtAddress)) {
		if(mergedAddress == NULL)  {
			LIST_INSERT_BEFORE(&freeBlocksList, (struct BlockElement *) nxtAddress, (struct BlockElement *)address);
			mergedAddress = address;
		}
		LIST_REMOVE(&freeBlocksList, (struct BlockElement *)nxtAddress);
		set_block_data(mergedAddress, size + get_block_size(nxtAddress) + prevSize, 0);
	}
	if(mergedAddress != NULL)
		return;

	struct BlockElement* it;
	LIST_FOREACH(it, &freeBlocksList) {
		if((char *)it > address) {
			LIST_INSERT_BEFORE(&freeBlocksList, it, (struct BlockElement *)address);
			return;
		}
	}
	LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement *)address);
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...

	if(new_size==0){
		free_block(va);
		return NULL;
	}
	if(va == NULL){
		return alloc_block_FF(new_size);
	}
	
	uint32 current_size = get_block_size(va);

	if(current_size > new_size){
		set_block_data(va,new_size,1);

		uint32 rem_size = current_size - new_size;

		if(rem_size >= DYN_ALLOC_MIN_BLOCK_SIZE)
		{	
			void* new_free_block = ((char*)va + new_size);
        	set_block_data(new_free_block, rem_size, 0);
        	LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement*)new_free_block);
		}
		return va;
	}
	else if(current_size < new_size)
	{
		char* nxtAddress = (((char*)va + current_size));
		if(is_free_block(nxtAddress)){
			uint32 nxt_size = get_block_size(nxtAddress);
			uint32 total_available_size = current_size+nxt_size;
			if(total_available_size >= new_size)
			{
				set_block_data(va,new_size,1);

				uint32 rem_size = total_available_size - new_size;
	
				if(rem_size >= DYN_ALLOC_MIN_BLOCK_SIZE)
				{
					void* new_free_block = ((char*)va + new_size);
					set_block_data(new_free_block, rem_size, 0);
					LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement*)new_free_block);
				}

				return va;
			}
		}
	}
	else
	{
		return va;
	}

	void* new_block = alloc_block_FF(new_size);
	if(new_block==NULL){
		return NULL;
	}
	
	free_block(va);
	
	return new_block;
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}