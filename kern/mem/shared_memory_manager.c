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
	cprintf("haha im here\n");
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
    uint32 frameSize = numOfFrames * sizeof(struct FrameInfo *);
    struct FrameInfo** FI = (struct FrameInfo **) kmalloc(frameSize);
	if(FI == NULL)
	{
		return NULL;
	}
	memset(FI,0, frameSize);
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
	cprintf("Creating and Initializing Shared Object...\n");
	struct Share *Sobject = (struct Share *)kmalloc(sizeof(struct Share));
    if (Sobject == NULL)
    {
        cprintf("couldn't allocate a Share object in create_share!\n");
        return NULL;
    }

	Sobject->ID = (int32)((uint32)Sobject & 0x7FFFFFFF); // mask MSB
	Sobject->ownerID = ownerID;
	strcpy(Sobject->name, shareName);
	Sobject->references = 1;
	Sobject->size = size;
	Sobject->isWritable = isWritable;

	uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	struct FrameInfo ** FI = create_frames_storage(numOfPages);
	if(FI == NULL)
	{
		cprintf("couldnt allocate a frame storage in the create share!\n");
		kfree(Sobject);	
		return NULL;
	}
	Sobject->framesStorage = FI;
	cprintf("Shared Object Created Successfully with values:\nID: %d\nOwnerID: %d\nName: %s\nSize: %u\nIsWritable: %d\nFramesStorage: %p\n", 
    Sobject->ID, 
    Sobject->ownerID, 
    Sobject->name, 
    Sobject->size, 
    Sobject->isWritable, 
    Sobject->framesStorage);

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
	if(!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);
	struct Share *SobjectSearch = NULL;
	struct Share *res = NULL;
	cprintf("i am now going to search the list\n");
	LIST_FOREACH(SobjectSearch, &AllShares.shares_list)
	{
		cprintf("Share Name: %s, Owner ID: %d\n", SobjectSearch->name, SobjectSearch->ownerID);
		if (SobjectSearch->ownerID == ownerID && strcmp(SobjectSearch->name, name) == 0)
		{
			cprintf("FOUND IT\n");
			res = SobjectSearch;
			break;
		}
		cprintf("ok next one\n");
	}

	if(res == NULL)
	{
		cprintf("I couldnt get the object needed!\n");
	}
	else
	{
		cprintf("Object retrieved!\n");
	}
	
	if(holding_spinlock(&AllShares.shareslock))
		release_spinlock(&AllShares.shareslock);
	
	return res;
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
	
	uint32 va = (uint32)virtual_address;
	struct Share* Sobject = create_share(ownerID, shareName, size, isWritable);
	cprintf("Checking if the created Object is null...\n");
	if (Sobject == NULL)
    {
        return E_NO_SHARE;
    }
	cprintf("Object not NULL!\n");

	uint32 numOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	//index for adding the frames in the frame storage
	uint32 ind = 0;
	uint8 success = 1;
	for (uint32 addr = va; addr < (va + numOfPages * PAGE_SIZE); addr += PAGE_SIZE)
	{
		struct FrameInfo *frame_info = NULL;
		if (allocate_frame(&frame_info) || map_frame(myenv->env_page_directory, frame_info, addr, PERM_WRITEABLE | PERM_USER))
		{
			success = 0;
			// Roll back if allocation fails
			for (uint32 rollback_addr = va; rollback_addr < addr; rollback_addr += PAGE_SIZE)
			{
				unmap_frame(myenv->env_page_directory, rollback_addr);
			}
			// Remove the added share
			memset(Sobject->framesStorage, 0, sizeof(struct FrameInfo*) * numOfPages);
			//Failed
			break;
		}
		Sobject->framesStorage[ind] = frame_info;
		ind++;
	}

	if(!success)
	{
		return E_NO_SHARE;
	}

	if(!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);

	LIST_INSERT_TAIL(&AllShares.shares_list, Sobject);

	if(holding_spinlock(&AllShares.shareslock))
		release_spinlock(&AllShares.shareslock);
	
	return Sobject->ID;
}

//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char *shareName, void *virtual_address)
{
	// TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	// panic("getSharedObject is not implemented yet");

	struct Env *myenv = get_cpu_proc(); // The calling environment
	struct Share *shareObj = get_share(ownerID, shareName);
	cprintf("i am getting\n");

	if (shareObj == NULL)
	{
		return E_SHARED_MEM_NOT_EXISTS;
	}

	uint32 va = (uint32)virtual_address;
	struct FrameInfo **framesList = shareObj->framesStorage;
	int numPages = shareObj->size / PAGE_SIZE + (shareObj->size % PAGE_SIZE != 0);

	for (int i = 0; i < numPages; i++)
	{
		struct FrameInfo *current_frame = framesList[i];
		uint32 perms = PERM_USER | (shareObj->isWritable ? PERM_WRITEABLE : 0);

		// Map the frame with error handling to prevent page faults
		map_frame(myenv->env_page_directory, current_frame, va, perms);
		va += PAGE_SIZE;
	}

	if (!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);

	shareObj->references++;

	if (holding_spinlock(&AllShares.shareslock))
		release_spinlock(&AllShares.shareslock);

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
