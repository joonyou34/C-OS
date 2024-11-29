#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share *get_share(int32 ownerID, char *name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
// Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list);
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char *shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS

	struct Share *ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
struct FrameInfo **create_frames_storage(int numOfFrames)
{
	// TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	// Your Code is Here...
	// TODO: DONT FORGET TO ADD THE INLINE BACK
	cprintf("meow i entered the create frames storage\n");
    uint32 frameSize = numOfFrames * sizeof(struct FrameInfo*);
    struct FrameInfo** FI = (struct FrameInfo **) kmalloc(frameSize);
	if(FI == NULL)
	{
		cprintf("meow i couldnt allocate in create frames storage\n");
		return NULL;
	}
	memset(FI,0, frameSize);
	cprintf("meow i left the create frames storage\n");
	return FI;
}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
// Allocates a new shared object and initialize its member
// It dynamically creates the "framesStorage"
// Return: allocatedObject (pointer to struct Share) passed by reference
struct Share *create_share(int32 ownerID, char *shareName, uint32 size, uint8 isWritable)
{
	// TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
	
	//ID(va), ownerID (have), name(have), size(have), writable(have), framestorage (call create_frame_storage)
	cprintf("meow i entered the create share\n");
	void* va = kmalloc(size);
	if(va == NULL)
	{
		cprintf("meow i couldnt allocate a sobject in the create share\n");
		return NULL;
	}
	struct Share* Sobject = (struct Share *) va;
	Sobject->ID = (int32)((uint32)va & 0x7FFFFFFF); // mask MSB
	Sobject->ownerID = ownerID;
	strcpy(Sobject->name, shareName);
	Sobject->references = 1;
	Sobject->size = size;
	Sobject->isWritable = isWritable;

	uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	struct FrameInfo ** FI = create_frames_storage(numOfPages);
	if(FI == NULL)
	{
		cprintf("meow i couldnt allocate a frame storage in the create share\n");
		kfree(va);	
		return NULL;
	}
	Sobject->framesStorage = FI;
	cprintf("meow i left the create share\n");
	return Sobject;
}

//=============================
// [3] Search for Share Object:
//=============================
// Search for the given shared object in the "shares_list"
// Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share *get_share(int32 ownerID, char *name)
{
	// TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("get_share is not implemented yet");
	// Your Code is Here...
	acquire_spinlock(&AllShares.shareslock);
	struct Share *SobjectSearch;
	LIST_FOREACH(SobjectSearch, &AllShares.shares_list)
	{
		if (SobjectSearch->ownerID == ownerID && strcmp(SobjectSearch->name, name) == 0)
		{
			release_spinlock(&AllShares.shareslock);
			return SobjectSearch;
		}
	}
	release_spinlock(&AllShares.shareslock);
	return NULL;
}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char *shareName, uint32 size, uint8 isWritable, void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	// Your Code is Here...
	
	struct Env* myenv = get_cpu_proc(); //The calling environment
	//shared object already exists
	if(get_share(ownerID, shareName) != NULL)
	{
		return E_SHARED_MEM_EXISTS;
	}

	#define USER_ALLOCATED 512  // 9TH bit (unused) set when the user allocated this page
	
	uint32 va = (uint32)virtual_address;
	struct Share* Sobject = create_share(ownerID, shareName, size, isWritable);
	if (Sobject == NULL)
    {
        return E_NO_SHARE;
    }

	acquire_spinlock(&AllShares.shareslock);
	LIST_INSERT_TAIL(&AllShares.shares_list, Sobject);

	uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	//index for adding the frames in the frame storage
	uint32 ind = 0;
	for (uint32 addr = va; addr < (va + numOfPages * PAGE_SIZE); addr += PAGE_SIZE)
	{
		struct FrameInfo *frame_info = NULL;
		if (allocate_frame(&frame_info) || map_frame(ptr_page_directory, frame_info, addr, PERM_WRITEABLE))
		{
			// Roll back if allocation fails
			for (uint32 rollback_addr = va; rollback_addr < addr; rollback_addr += PAGE_SIZE)
			{
				unmap_frame(ptr_page_directory, rollback_addr);
			}
			// Remove the added share
			LIST_REMOVE(&AllShares.shares_list, AllShares.shares_list.lh_last);
			memset(Sobject->framesStorage, 0, sizeof(struct FrameInfo*) * numOfPages);
			//Failed
			release_spinlock(&AllShares.shareslock);
			return E_NO_SHARE;
		}


		uint32* table;
		get_page_table(ptr_page_directory,addr,&table);// 9TH bit (set when it is the end page in allocation) 
		table[PTX(addr)] |= USER_ALLOCATED;//set a unused bit
		Sobject->framesStorage[ind] = frame_info;
		ind++;
	}
	release_spinlock(&AllShares.shareslock);
	return Sobject->ID;
}

//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char *shareName, void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	// Your Code is Here...

	struct Env *myenv = get_cpu_proc(); // The calling environment
	
	struct Share *shareObj = get_share(ownerID,shareName);

	if(shareObj==NULL){
		return E_SHARED_MEM_NOT_EXISTS;
	}

	struct FrameInfo *ptr;
	uint32 va = (uint32)virtual_address;

	struct FrameInfo** framesList = shareObj->framesStorage;
	int finish = shareObj->size/PAGE_SIZE + (shareObj->size%PAGE_SIZE != 0);
	for (int i = 0; i < finish; i++)
	{
		struct FrameInfo* current_frame = framesList[i];
		map_frame(ptr_page_directory, current_frame, va, shareObj->isWritable);
		current_frame->references++;
		va += PAGE_SIZE;
	}
	
	shareObj->references++;
	return shareObj->ID;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
// delete the given shared object from the "shares_list"
// it should free its framesStorage and the share object itself
void free_share(struct Share *ptrShare)
{
	// TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	// Your Code is Here...
	
}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	// TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	// Your Code is Here...
}
