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

//struct id_search
//{
//	int32 id;
//	uint32 va;
//};
//struct id_search id_search_array[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE]={0};
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
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
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//

	struct Share* ptr_share = get_share(ownerID, shareName);

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
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
	uint32 allocatedSize=numOfFrames*sizeof(struct FrameInfo*);
//	if(!holding_spinlock(&MemFrameLists.mfllock))
//			acquire_spinlock(&MemFrameLists.mfllock);
	struct FrameInfo** frames_storage=(struct FrameInfo**)kmalloc(allocatedSize);
	//release_spinlock(&MemFrameLists.mfllock);
	for(int i=0;i<numOfFrames;i++)
	{
		frames_storage[i]=NULL;
	}
	if(frames_storage!=NULL)
	{
		return frames_storage;
	}
	else
	{
		return NULL;
	}
	//cprintf("CFS!!!!!!!!!!!!!!!!!!\n");
}


//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
	//cprintf("da5alt create share w hast5dem kmalloc ahu");
//	if(isKHeapPlacementStrategyFIRSTFIT()){
//		struct FrameInfo *framePointer;
//		struct Share* create_new_share;
//		uint32 num_of_frames = sizeof(struct Share *)/(PAGE_SIZE); //
//		uint32 remainder = sizeof(struct Share *)%PAGE_SIZE;//
//			if (remainder>0){
//				num_of_frames++;
//			}
//
//		uint32 pageCounter = rlimit_heap+PAGE_SIZE;
//		uint32 startPage=pageCounter;
//		uint32 increment=0;
//        uint32* fisrtFitPageTable = NULL;
//		struct FrameInfo *ptr_frame_info=NULL;
//			while(increment<num_of_frames){
//				get_page_table(ptr_page_directory,pageCounter,&fisrtFitPageTable);
//				ptr_frame_info = get_frame_info(ptr_page_directory,pageCounter,&fisrtFitPageTable);
//				pageCounter= pageCounter+PAGE_SIZE;
//                if(ptr_frame_info==0){ //pageTable[x]==0?
//					increment++;
//                }
//				else{
//					increment=0;
//					startPage = pageCounter;
//				}
//                if(pageCounter>=KERNEL_HEAP_MAX){
//						return NULL;
//				}
//			}
//              uint32 mapCounter = startPage;
//              for(int i = 0;i<num_of_frames;i++){
//						//cprintf("4 HARRYYYYYYYYYYYYYYYYYYYYYYYY\n");
//						int ret = allocate_frame(&framePointer);
//						if(ret != E_NO_MEM){
//						framePointer->NumofFrames=num_of_frames;
//						map_frame(ptr_page_directory,framePointer,mapCounter,PERM_WRITEABLE);
//						uint32 physical_address = to_physical_address(framePointer);
//						uint32 frame_no=physical_address/PAGE_SIZE;
//						frame_arr[frame_no]=mapCounter;
//						//cprintf("frame NO: %d\n",frame_no);
//						//cprintf("VA page: %x\n",frame_arr[frame_no]);
//						mapCounter=mapCounter+ PAGE_SIZE;
//						}else{
//							//cprintf("NOOOO MEM FOR FRAMESSSS\n");
//						}
//			 }
//	 //make sure its first fit
//		create_new_share=(struct Share*)startPage;
//	 //mynf3sh n3ml dynamic allocation
//	    uint32 va=(uint32) create_new_share;
//	if(create_new_share==NULL)
//	{
//		return NULL;
//	}
//	else
//	{
//  int numOfFrames=ROUNDUP(size,PAGE_SIZE);
//	if(!holding_spinlock(&MemFrameLists.mfllock))
//		acquire_spinlock(&MemFrameLists.mfllock);
        uint32* t=(uint32 *)kmalloc(sizeof(struct Share ));
        //release_spinlock(&MemFrameLists.mfllock);
        if(t==NULL){
	       return NULL;
        }
//     struct id_search get_id;
//     get_id.va=(uint32)t;

    struct Share * create_new_share=(struct Share * )t;
	create_new_share->ownerID=ownerID;
	create_new_share->isWritable=isWritable;
	create_new_share->size=size;
	create_new_share->references=1;
	 char * y=strcpy(create_new_share->name, shareName);
//	 if(strncmp(y,shareName,strlen(shareName))==0)
//		 cprintf("yessssssssss\n");
//	 else
//		 cprintf("nooooooooooo\n");
	 		uint32 num_of_frames = size/(PAGE_SIZE); //
	 		uint32 remainder = size%PAGE_SIZE;//
	 			if (remainder>0){
	 				num_of_frames++;
	 			}
	create_new_share->framesStorage=create_frames_storage(num_of_frames);
	if(create_new_share->framesStorage==NULL)
	{
		kfree(t);
		return NULL;
	}
	int mask=0x7FFFFFFF;
	create_new_share->ID=(int)t&mask;
//	get_id.id=create_new_share->ID;
//	for(int i=0;i<(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;i++)
//	{
//		if(id_search_array[i].id==0 && id_search_array[i].va==0)
//		{
//			id_search_array[i]=get_id;
//		}
//	}

	//cprintf("CS!!!!!!!!!!!!!!!!!!\n");

	return create_new_share;
}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("get_share is not implemented yet");
	//Your Code is Here...
	//cprintf("d5l get share\n");
	struct Share *iter_element;


		LIST_FOREACH(iter_element,&AllShares.shares_list)
		{
            int ret=strcmp(iter_element->name,name);
            if(iter_element->ownerID == ownerID && ret == 0)
			{
            	if (holding_spinlock(&AllShares.shareslock))
            		release_spinlock(&AllShares.shareslock);
				return iter_element;
			}

		}




	// Traverse the list using the macro LIST_FOREACH
		return NULL;
}
//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...
    //cprintf("create shared object\n");
	if (!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);

	struct Env* myenv = get_cpu_proc(); //The calling environment
	//cprintf("abl get share\n");
	struct Share* temp=get_share(ownerID, shareName);
	//cprintf("abl if\n");
	if(temp!=NULL){
		//cprintf("d5lt if\n");
    	if (holding_spinlock(&AllShares.shareslock))
    		release_spinlock(&AllShares.shareslock);

		return E_SHARED_MEM_EXISTS;
	}
	//cprintf("b3d if\n");
	struct Share* new_obj = create_share(ownerID,shareName,size,isWritable);
	if(new_obj != NULL)
	{
		LIST_INSERT_TAIL(&AllShares.shares_list, new_obj);
		//cprintf("CSO!!!!!!!!!!!!!!!!!!\n");
		uint32 num_of_frames = size/(PAGE_SIZE);
		uint32 remainder = size%PAGE_SIZE;
		if (remainder>0)
		{
		   num_of_frames++;
		}
		struct FrameInfo *ptr_frame_info;
		uint32 va= (uint32)virtual_address;
		for (int i = 0; i < num_of_frames; i++)
		{
			ptr_frame_info=NULL;
			//acquire_spinlock(&MemFrameLists.mfllock);
			int ret = allocate_frame(&ptr_frame_info);
			//release_spinlock(&MemFrameLists.mfllock);
			if (ret != E_NO_MEM)
			{
			   map_frame(myenv->env_page_directory, ptr_frame_info,va,PERM_WRITEABLE|PERM_USER);
			   new_obj->framesStorage[i]=(ptr_frame_info);
			}
			else
			{
				for (int j = 0; j < i; j++)
				{
					     unmap_frame(myenv->env_page_directory,va);
						 va-= PAGE_SIZE;
				}
//            	if (holding_spinlock(&AllShares.shareslock))
//            		release_spinlock(&AllShares.shareslock);
				return E_NO_SHARE;
			}
			va += PAGE_SIZE;
		}
    	if (holding_spinlock(&AllShares.shareslock))
    		release_spinlock(&AllShares.shareslock);
		return new_obj->ID;
	}
	else
	{
    	if (holding_spinlock(&AllShares.shareslock))
    		release_spinlock(&AllShares.shareslock);
		return E_NO_SHARE;
	}
}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("getSharedObject is not implemented yet");
//Your Code is Here...
    //cprintf("d5l get shared objectttt\n");

	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* iter = get_share(ownerID,shareName);
	//cprintf("b3d get shared f get shared object\n");
	if (!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);
	uint32 va = (uint32) virtual_address;
	if(iter == NULL){
    	if (holding_spinlock(&AllShares.shareslock))
    		release_spinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
		}
	    uint32 numofframes=(iter->size)/PAGE_SIZE;
		uint32 remainder=(iter->size)%PAGE_SIZE;
		if(remainder!=0){
			numofframes++;
		}
		for(int i=0;i<numofframes;i++){
			//cprintf("awl for\n");
	       if(iter->isWritable == 1)
			{
			//cprintf("if f for\n");
			map_frame(myenv->env_page_directory,(iter->framesStorage[i]),va,PERM_WRITEABLE|PERM_USER|PERM_PRESENT);
			//cprintf("b3d map\n");
			}
			else
			{
				//cprintf("else f  for\n");
				map_frame(myenv->env_page_directory,(iter->framesStorage[i]),va,PERM_USER|PERM_PRESENT);
			}
			//cprintf("a5r al for\n");
			va+=PAGE_SIZE;
		}
		iter->references++;
//		struct id_search get_id;
//		get_id.va=(uint32)virtual_address;
//		get_id.id=iter->ID;
//		for(int i=0;i<(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;i++)
//			{
//				if(id_search_array[i].id==0 && id_search_array[i].va==0)
//				{
//					id_search_array[i]=get_id;
//				}
//			}
	if (holding_spinlock(&AllShares.shareslock))
		release_spinlock(&AllShares.shareslock);
	return iter->ID;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	if (!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);

	//cprintf("free_share \n");
	if(ptrShare==NULL)
	{
    	if (holding_spinlock(&AllShares.shareslock))
    		release_spinlock(&AllShares.shareslock);
		return;
	}
	LIST_REMOVE(&AllShares.shares_list,ptrShare);
	//cprintf("FS!!!!!!!!!!!!!!!!!!\n");


	//ptrShare->framesStorage=create_frames_storage(num_of_frames);
	kfree(ptrShare->framesStorage);
	kfree(ptrShare);
	//cprintf("end of free_share \n");
	if (holding_spinlock(&AllShares.shareslock))
		release_spinlock(&AllShares.shareslock);

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	if (!holding_spinlock(&AllShares.shareslock))
		acquire_spinlock(&AllShares.shareslock);
	struct Share* iter_element;
	struct Share* shared_to_be_removed;
	struct Env* myenv = get_cpu_proc();
	uint32 va = (uint32) startVA;
	//cprintf("sharedObjectID:%d \n",sharedObjectID);
	LIST_FOREACH(iter_element,&AllShares.shares_list)
	{
		//cprintf("badawar aho f el list \n");
		if(iter_element->ID == sharedObjectID)
		{
			shared_to_be_removed =iter_element;
			//cprintf("la2et el obj \n");
			break;
		}
	}
	//cprintf("FSO!!!!!!!!!!!!!!!!!!\n");

		//cprintf("shared_to_beremoved:%d \n",shared_to_be_removed->ID);
		if (shared_to_be_removed == NULL){
        	if (holding_spinlock(&AllShares.shareslock))
        		release_spinlock(&AllShares.shareslock);
			return E_SHARED_MEM_NOT_EXISTS;
		}

			int size =  getSizeOfSharedObject(shared_to_be_removed->ownerID, shared_to_be_removed->name);
			uint32 num_of_frames = (ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE);
			for(int i = 0; i < num_of_frames; i++)
			{
				//cprintf("size:%d",shared_to_be_removed->size);
				//cprintf("unmappp \n");
				 unmap_frame(myenv->env_page_directory,va);
				 //env_page_ws_invalidate(myenv,i);
				 //shared_to_be_removed->framesStorage[i]->references--;
				 va+= PAGE_SIZE;
			}

			va = (uint32) startVA;
			//if pagetable(s) become empty remove them
			uint32 *ptr_page_table;
			struct FrameInfo* ptr_frame_info;
			bool empty = 1;
			for(int i=0;i<num_of_frames;i++)
			{
				empty = 1;
				get_page_table(myenv->env_page_directory,va,&ptr_page_table);
				if(ptr_page_table == NULL)
				{
					//cprintf("mala2tsh el page_table \n");
					va+=PAGE_SIZE;
					continue;
				}
				for(int j =0; j<1024;j++)
				{
					//cprintf("checking law msh empty \n");
					if(ptr_page_table[j] & PERM_PRESENT)
					{
						empty=0;
						break;
					}
				}
				if(empty==1)
				{
					//unmap_frame(myenv->env_page_directory,(uint32)ptr_page_table);
					pd_clear_page_dir_entry(myenv->env_page_directory,va);
					kfree((uint32*)ptr_page_table);

				}

				va+=PAGE_SIZE;

			}
			shared_to_be_removed->references--;
			if(shared_to_be_removed->references == 0)//this is the last share
			{
				//cprintf("refrences 0 \n");
				free_share(shared_to_be_removed);
			}

			tlbflush();
			//tlb_invalidate((uint32 *)myenv->env_page_directory,(void*)va);
			//cprintf("end of free shared obj \n");
        	if (holding_spinlock(&AllShares.shareslock))
        		release_spinlock(&AllShares.shareslock);
			return 0;
	//Your Code is Here...

}

