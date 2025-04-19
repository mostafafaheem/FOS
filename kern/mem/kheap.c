#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC

//struct{
//	struct spinlock lk;
//	uint32 frame_arr[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];
//}Frame_Arr;

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
	start_heap=daStart;
	rlimit_heap=daLimit;
	uint32 size=ROUNDUP(initSizeToAllocate,PAGE_SIZE)+start_heap;
	brk=size;//check eno yekon less than daLimit
	if(brk>daLimit)
	{
		panic("the brk is greater than the hard limit\n");
		//return 0;
	}
	else
	{
		uint32 range_size=brk-daStart;
		range_size=ROUNDUP(range_size,PAGE_SIZE);
		uint32 num_pages=range_size/PAGE_SIZE;

		struct FrameInfo *ptr_frame_info;
		uint32 va=daStart;

		for(int i=0;i<num_pages;i++)
		{
			//acquire_spinlock(&MemFrameLists.mfllock);
			int ret=allocate_frame(&ptr_frame_info);
			//release_spinlock(&MemFrameLists.mfllock);
			if(ret!=E_NO_MEM)
			{
				map_frame(ptr_page_directory,ptr_frame_info,(uint32)va,PERM_PRESENT|PERM_WRITEABLE);
				uint32 corres_pa=to_physical_address(ptr_frame_info);
				uint32 frame_no=corres_pa/PAGE_SIZE;
//				acquire_spinlock(&Frame_Arr.lk);
				frames_info[frame_no].mappedVA= va&0xFFFFF000;
//				release_spinlock(&Frame_Arr.lk);
			}
			else
			{
				break;
				panic("E_NO_MEM!!\n");
			}
			va+=PAGE_SIZE;

		}

		initialize_dynamic_allocator(daStart,initSizeToAllocate);
		return 0;
	}
}


void* sbrk(int numOfPages)
{
	uint32 old_brk =brk;
	uint32 new_brk =ROUNDUP((uint32)brk+(numOfPages*PAGE_SIZE),PAGE_SIZE);

	if (numOfPages== 0)
	{
		return (void*)brk;
	}
	else if(new_brk<= (uint32)rlimit_heap)
	{
		struct FrameInfo *ptr_frame_info;
		uint32 va= (uint32)brk;

		brk+= (numOfPages*PAGE_SIZE);


		for (int i = 0; i < numOfPages; i++)
		{
			//acquire_spinlock(&MemFrameLists.mfllock);
			int ret = allocate_frame(&ptr_frame_info);
			//release_spinlock(&MemFrameLists.mfllock);
			if (ret != E_NO_MEM)
			{
				map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
				uint32 corres_pa=to_physical_address(ptr_frame_info);
				uint32 frame_no=corres_pa/PAGE_SIZE;
//				acquire_spinlock(&Frame_Arr.lk);
				frames_info[frame_no].mappedVA= va&0xFFFFF000;
//				release_spinlock(&Frame_Arr.lk);
			}
			else
			{
				return (void*)-1;
				panic("E_NO_MEM!!\n");
			}
			va += PAGE_SIZE;
		}

		return (void*) old_brk;
	}
	else
	{
		return (void*)-1;
	}
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{

	struct FrameInfo *framePointer;
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	if(isKHeapPlacementStrategyFIRSTFIT()){   //make sure its first fit

		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE && size>0){
			uint32 virt_block=(uint32)alloc_block_FF(size);
//			if(holding_spinlock(&MemFrameLists.mfllock))
//				release_spinlock(&MemFrameLists.mfllock);
			return (void*)virt_block;
		}else if(size > DYN_ALLOC_MAX_BLOCK_SIZE){
//			if(!holding_spinlock(&MemFrameLists.mfllock))
//							acquire_spinlock(&MemFrameLists.mfllock);

			uint32 num_of_frames = ROUNDUP(size,PAGE_SIZE)/(PAGE_SIZE);
			uint32 pageCounter = rlimit_heap+PAGE_SIZE;
			uint32 startPage=pageCounter;
			uint32 increment=0;
			uint32 prevPageCounter;

			uint32* fisrtFitPageTable = NULL;
			struct FrameInfo *ptr_frame_info=NULL;
			get_page_table(ptr_page_directory,pageCounter,&fisrtFitPageTable);
			while(increment<num_of_frames){

				if(PDX(pageCounter) != PDX(prevPageCounter)){
					get_page_table(ptr_page_directory,(uint32)pageCounter,&fisrtFitPageTable);
				}

				ptr_frame_info = get_frame_info(ptr_page_directory,pageCounter,&fisrtFitPageTable);

				if(ptr_frame_info==0){
					increment++;
					prevPageCounter = pageCounter;
					pageCounter+=PAGE_SIZE;

				}
				else{
					increment=0;
					prevPageCounter = pageCounter;
					pageCounter+= (ptr_frame_info->NumofFrames)*PAGE_SIZE;
					startPage = pageCounter;
				}

				if(pageCounter>=KERNEL_HEAP_MAX)
//					if(holding_spinlock(&MemFrameLists.mfllock))
//						release_spinlock(&MemFrameLists.mfllock);
					return NULL;

			}

			uint32 mapCounter = startPage;

			for(int i = 0;i<num_of_frames;i++){
				//cprintf("4 HARRYYYYYYYYYYYYYYYYYYYYYYYY\n");
				//acquire_spinlock(&MemFrameLists.mfllock);
				int ret = allocate_frame(&framePointer);
				//release_spinlock(&MemFrameLists.mfllock);
				if(ret != E_NO_MEM){
				framePointer->NumofFrames=num_of_frames;
				map_frame(ptr_page_directory,framePointer,mapCounter,PERM_WRITEABLE);
				uint32 physical_address = to_physical_address(framePointer);
				uint32 frame_no=physical_address/PAGE_SIZE;
//				acquire_spinlock(&Frame_Arr.lk);
				frames_info[frame_no].mappedVA=mapCounter;
//				release_spinlock(&Frame_Arr.lk);

				mapCounter+= PAGE_SIZE;
				}else{
					//cprintf("NOOOO MEM FOR FRAMESSSS\n");
				}
			}
//			if(holding_spinlock(&MemFrameLists.mfllock))
//				release_spinlock(&MemFrameLists.mfllock);
			return (void*)startPage; //ELMAFROOD A RETURN EHH??
		}else
//			if(holding_spinlock(&MemFrameLists.mfllock))
//					release_spinlock(&MemFrameLists.mfllock);
			return NULL;

	}
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
//	if(holding_spinlock(&MemFrameLists.mfllock))
//			release_spinlock(&MemFrameLists.mfllock);
return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
//	cprintf("free hereee\n");
//	if(!holding_spinlock(&MemFrameLists.mfllock))
//		acquire_spinlock(&MemFrameLists.mfllock);
	if((uint32)virtual_address < KERNEL_HEAP_START || (uint32)virtual_address > KERNEL_HEAP_MAX)
		panic(" Invalid Address..!!");

	else if((uint32)virtual_address <=rlimit_heap)
        free_block(virtual_address);

	else if( (uint32)virtual_address>=(rlimit_heap+PAGE_SIZE)){

		uint32* PageTable;
        uint32  ptr_per;
        char* ptr_to_va=virtual_address;
		struct FrameInfo* ptr_frame_info=get_frame_info(ptr_page_directory, (uint32)virtual_address,&PageTable);
		int frames=ptr_frame_info->NumofFrames;
		for(int i=0;i<frames;i++)
		{

			ptr_frame_info=get_frame_info(ptr_page_directory, (uint32)(ptr_to_va+(i*PAGE_SIZE)),&PageTable);
			ptr_frame_info->NumofFrames=0;

			//acquire_spinlock(&MemFrameLists.mfllock);
			unmap_frame(ptr_page_directory, (uint32)(ptr_to_va+(i*PAGE_SIZE)));
			//release_spinlock(&MemFrameLists.mfllock);
		}
//		if(holding_spinlock(&MemFrameLists.mfllock))
//			release_spinlock(&MemFrameLists.mfllock);
		return;
   }else
//	   if(holding_spinlock(&MemFrameLists.mfllock))
//	   		release_spinlock(&MemFrameLists.mfllock);
	   return;

}


unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32 pageDirindex = PDX(virtual_address);
	uint32 pagetableindex = PTX(virtual_address);
	uint32* pagetable = NULL;
	struct FrameInfo* frame_inf = NULL;
	if(get_page_table(ptr_page_directory,virtual_address,&pagetable) == TABLE_NOT_EXIST){
		return 0;
	}
	else if(pagetable == NULL){
		return 0;
	}
	else{
		uint32 pagetableentry = pagetable[pagetableindex];
		frame_inf = get_frame_info(ptr_page_directory,virtual_address,&pagetable);
		if((!(pagetableentry & PERM_PRESENT)) || (frame_inf == NULL) || (frame_inf->references == 0 ))
			return 0;
		uint32 framenumber = pagetableentry & 0xFFFFF000;
		uint32 offset_of_framenumber = virtual_address & 0x00000FFF;
		uint32 pa = framenumber + offset_of_framenumber;
		return pa;
	}


	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}


unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
		uint32 frame_no=physical_address/PAGE_SIZE;
		//cprintf("frame NO: %d\n",frame_no);
		unsigned int offset=physical_address& 0x00000FFF;
		//cprintf("OFFSET: %d\n",offset);
		if(frames_info[frame_no].references==0)//not mapped
		{
			//cprintf("not mapped\n");
			return 0;
		}
		else
		{
			uint32 ret = frames_info[frame_no].mappedVA+offset;
				return ret;
		}


	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
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
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//panic("krealloc() is not implemented yet...!!");
	struct FrameInfo* ptr_page_info = NULL;
	uint32* ptr_page_table = NULL;
	get_page_table(ptr_page_directory,(uint32)virtual_address,&ptr_page_table);
	ptr_page_info = get_frame_info(ptr_page_directory,(uint32)virtual_address,&ptr_page_table);

	if (virtual_address==NULL&&new_size==0)
		return NULL;

	else if (new_size==0){
		kfree(virtual_address);
		return NULL;
	}

	else if (virtual_address==NULL)
		return kmalloc(new_size);

	else if((uint32)virtual_address>=KERNEL_HEAP_START&&(uint32)virtual_address<rlimit_heap&&(new_size+2*sizeof(uint32))==get_block_size(virtual_address))
		return virtual_address;

	else if ((uint32)virtual_address>=KERNEL_HEAP_START&&(uint32)virtual_address<rlimit_heap&&new_size<=DYN_ALLOC_MAX_BLOCK_SIZE&&new_size>0)
		return realloc_block_FF((void*)virtual_address, new_size);


	else if((uint32)virtual_address>=KERNEL_HEAP_START&&(uint32)virtual_address<rlimit_heap&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE){
		void* ret = kmalloc(new_size);
		kfree(virtual_address);
		return ret;
	}

	else if((uint32)virtual_address>=(rlimit_heap+PAGE_SIZE)&&(uint32)virtual_address<KERNEL_HEAP_MAX&&ROUNDUP(new_size,PAGE_SIZE)==(ptr_page_info->NumofFrames*PAGE_SIZE))
		return virtual_address;

	else if((uint32)virtual_address>=(rlimit_heap+PAGE_SIZE)&&(uint32)virtual_address<KERNEL_HEAP_MAX&&new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){
		void* ret = kmalloc(new_size);
		kfree(virtual_address);
		return ret;
	}
	else if((uint32)virtual_address>=(rlimit_heap+PAGE_SIZE)&&(uint32)virtual_address<KERNEL_HEAP_MAX&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE&&ROUNDUP(new_size,PAGE_SIZE)<(ptr_page_info->NumofFrames*PAGE_SIZE)){
		uint32 currentSize=ptr_page_info->NumofFrames*PAGE_SIZE;
		uint32 sizeDifference=currentSize-ROUNDUP(new_size,PAGE_SIZE);
		uint32 extraFrames=sizeDifference/PAGE_SIZE;
		uint32 newNumOfFrames=ptr_page_info->NumofFrames-extraFrames;

		for(int i=0;i<newNumOfFrames;i++){
			if(PDX((uint32)virtual_address+i*PAGE_SIZE) != PDX((uint32)virtual_address+(i-1)*PAGE_SIZE))
				get_page_table(ptr_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);

			ptr_page_info = get_frame_info(ptr_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);
			ptr_page_info->NumofFrames=newNumOfFrames;
		}

		for (int i=0;i<extraFrames;i++){
			if(PDX((uint32)virtual_address+newNumOfFrames+i*PAGE_SIZE) != PDX((uint32)virtual_address+newNumOfFrames+(i-1)*PAGE_SIZE))
				get_page_table(ptr_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);

			ptr_page_info = get_frame_info(ptr_page_directory,(uint32)virtual_address+newNumOfFrames+i*PAGE_SIZE,&ptr_page_table);
			ptr_page_info->NumofFrames=0;
			//acquire_spinlock(&MemFrameLists.mfllock);
			unmap_frame(ptr_page_directory, (uint32)virtual_address+newNumOfFrames+(i*PAGE_SIZE));
			//release_spinlock(&MemFrameLists.mfllock);
		}
		return virtual_address;
	}
	else if((uint32)virtual_address>=(rlimit_heap+PAGE_SIZE)&&(uint32)virtual_address<KERNEL_HEAP_MAX&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE&&ROUNDUP(new_size,PAGE_SIZE)>(ptr_page_info->NumofFrames*PAGE_SIZE)){
		get_page_table(ptr_page_directory,(uint32)virtual_address,&ptr_page_table);
		ptr_page_info = get_frame_info(ptr_page_directory,(uint32)virtual_address,&ptr_page_table);

		uint32 currentNumOfFrames=ptr_page_info->NumofFrames;
		uint32 currentSize=currentNumOfFrames*PAGE_SIZE;
		uint32 sizeDifference=ROUNDUP(new_size,PAGE_SIZE)-currentSize;
		uint32 extraFrames=sizeDifference/PAGE_SIZE;
		uint32 iterations=0;
		uint32 pageCounter=(uint32)virtual_address+currentSize;
		uint32 newNumOfFrames=currentNumOfFrames+extraFrames;
//		bool freed;

		while(iterations<extraFrames){

			if(PDX(pageCounter) != PDX(pageCounter-PAGE_SIZE))
				get_page_table(ptr_page_directory,(uint32)pageCounter,&ptr_page_table);

			ptr_page_info = get_frame_info(ptr_page_directory,pageCounter,&ptr_page_table);

			if(ptr_page_info==0){
				iterations++;
				pageCounter+=PAGE_SIZE;
			}

			else{
				void* ret = kmalloc(new_size);
				kfree(virtual_address);
				return ret;
			}

			if(pageCounter>=KERNEL_HEAP_MAX)
				return NULL;
		}

		for(int i=0;i<newNumOfFrames;i++){
			if(PDX((uint32)virtual_address+i*PAGE_SIZE) != PDX((uint32)virtual_address+(i-1)*PAGE_SIZE))
				get_page_table(ptr_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);

			ptr_page_info = get_frame_info(ptr_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);
			ptr_page_info->NumofFrames=newNumOfFrames;
		}

		struct FrameInfo* framePointer;
		uint32 mapCounter = (uint32)virtual_address+currentSize;
		uint32 physical_address;
		for(int i = 0;i<extraFrames;i++){
			//acquire_spinlock(&MemFrameLists.mfllock);
			int ret = allocate_frame(&framePointer);
			//release_spinlock(&MemFrameLists.mfllock);
			if(ret != E_NO_MEM){
				framePointer->NumofFrames=newNumOfFrames;
				map_frame(ptr_page_directory,framePointer,mapCounter,PERM_WRITEABLE);
				physical_address = to_physical_address(framePointer);
//				acquire_spinlock(&Frame_Arr.lk);
				frames_info[physical_address/PAGE_SIZE].mappedVA=mapCounter;
//				release_spinlock(&Frame_Arr.lk);
				mapCounter+=PAGE_SIZE;
			}
			else return NULL;
		}
		return virtual_address;
	}
	else return NULL;

	return NULL;
}
