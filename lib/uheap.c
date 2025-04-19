#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

struct Page_Info
{
	uint32 virtual_address;
	uint32 size;
	bool marked;
	int32 id;
	//uint32 va;
	//LIST_ENTRY(Page_Info) prev_next_info;	/* linked list links */
};// __attribute__((packed))
//struct id_search
//{
//	int32 id;
//	uint32 va;
//};
//LIST_HEAD(PageList, Page_Info);
//struct PageList page_marked_List;
struct Page_Info arrPageInfo[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE] = { 0 };
//struct id_search id_search_array[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE]={ 0 };

int arrPageInfo_index = 0;
//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
		if(size<=DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			uint32 blk_alloc=(uint32)alloc_block_FF(size);
			return (void*)blk_alloc;
		}
		else if(size>DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			uint32 startVA=myEnv->uLimit+PAGE_SIZE;
			uint32 no_of_pages=size/PAGE_SIZE;
			uint32 remainder=size%PAGE_SIZE;
			uint32 startIndex=(startVA-USER_HEAP_START)/PAGE_SIZE;
			uint32 increment=0;
			bool found = 0;
			if(remainder!=0)
			{
				no_of_pages++;
			}
			//cprintf("no of pages:%d\n",no_of_pages);

			uint32 index=(startVA-USER_HEAP_START)/PAGE_SIZE;

			while(increment<no_of_pages){
				//cprintf("startva1: %d\n",startVA);

				//pageCounter = pageCounter+PAGE_SIZE;
				//cprintf("pagecounter2: %d\n",pageCounter);

				if(arrPageInfo[index].marked==0){
					increment++;

//					if(increment%10000>9998)
//					cprintf("INcrement: %d\n",increment);
					index++;
				}
				else{
					increment=0;
					startIndex = index+((ROUNDUP(arrPageInfo[index].size,PAGE_SIZE))/PAGE_SIZE);
					index=startIndex;
					//cprintf("INDEXXXXXXXXXXXXXXXXXXXXXX: %d\n",index);
				}
				//cprintf("pagecounter3: %d\n",pageCounter);

				if((index*PAGE_SIZE)+USER_HEAP_START>=USER_HEAP_MAX){

					//cprintf("nulllll\n");
				return NULL;
				}
			}
			uint32 markingCounter = startIndex;
			for(int i=0;i<no_of_pages;i++){

				arrPageInfo[markingCounter].marked=1;
				arrPageInfo[markingCounter].virtual_address=(markingCounter*PAGE_SIZE)+USER_HEAP_START;
				arrPageInfo[markingCounter].size=size;


				//cprintf("SIZE: %d\n",arrPageInfo[markingCounter].size);
				markingCounter++;
			}
			//cprintf("khalasna 3ala kheir w hanendah harry \n");
			startVA= (startIndex*PAGE_SIZE)+USER_HEAP_START;
			sys_allocate_user_mem(startVA,size);
			return (void*)startVA;
		}
}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	if((uint32)virtual_address < USER_HEAP_START || (uint32)virtual_address > USER_HEAP_MAX){
//			cprintf("first");
			panic(" Invalid Address..!!");
	}else if((uint32)virtual_address <=(myEnv->uLimit)){
//			cprintf("blockkkkk\n");
	        free_block(virtual_address);
    }else if( (uint32)virtual_address>=((myEnv->uLimit)+PAGE_SIZE)){
			//cprintf("pageeee\n");
			uint32 size_allocated =arrPageInfo[((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE].size;
			//cprintf("1\n");

            uint32 no_of_page_allocated =size_allocated/PAGE_SIZE;
			//cprintf("2\n");

            uint32 reminder=size_allocated%PAGE_SIZE;
			//cprintf("3\n");

             if(reminder!=0)
	            no_of_page_allocated++;
			uint32 start_index=((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE;
			//cprintf("4\n");

		    while(no_of_page_allocated!=0){
				//cprintf("5\n");

		    	arrPageInfo[start_index].virtual_address=0;
				//cprintf("6\n");

		    	arrPageInfo[start_index].marked=0;
				//cprintf("7\n");

		    	arrPageInfo[start_index].size=0;
				//cprintf("8\n");

		    	start_index++;
		    	no_of_page_allocated--;
	     	}
			//cprintf("9\n");

		    sys_free_user_mem((uint32)virtual_address,size_allocated);
	}
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//cprintf("ana fel smalloc\n");
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
	  uint32 startVA=myEnv->uLimit+PAGE_SIZE;
	  uint32 no_of_pages=size/PAGE_SIZE;
	  uint32 remainder=size%PAGE_SIZE;
	  uint32 startIndex=(startVA-USER_HEAP_START)/PAGE_SIZE;
      uint32 increment=0;
      bool found = 0;
	  if(remainder!=0)
	  {
		no_of_pages++;
	  }
	  uint32 index=(startVA-USER_HEAP_START)/PAGE_SIZE;
      while(increment<no_of_pages){
		  if(arrPageInfo[index].marked==0){
			 increment++;
			 index++;
		  }
		  else{
			 increment=0;
			 startIndex = index+((ROUNDUP(arrPageInfo[index].size,PAGE_SIZE))/PAGE_SIZE);
			 index=startIndex;
		  }
		  if((index*PAGE_SIZE)+USER_HEAP_START>=USER_HEAP_MAX){
	    	return NULL;
		  }
 	 }

	 startVA= (startIndex*PAGE_SIZE)+USER_HEAP_START;
    // cprintf("ablhaaa sys create\n");
	 int ret=sys_createSharedObject(sharedVarName,size,isWritable,(void*)startVA);
	 //cprintf("b3d sys createee\n");
		struct Page_Info idVAR;
		idVAR.virtual_address=(uint32)startVA;
//		int mask=0x7FFFFFFF;
//		int virtual_address_id=(int)startVA&mask;
		idVAR.id=ret;

	 if(ret==E_SHARED_MEM_EXISTS){
			return NULL;
	 }
	 else if(ret!=E_SHARED_MEM_EXISTS || ret!=E_NO_SHARE)
	 {
		//cprintf("gwaa\n");
		uint32 markingCounter = startIndex;
		for(int i=0;i<no_of_pages;i++){
				arrPageInfo[markingCounter].marked=1;
				arrPageInfo[markingCounter].virtual_address=(markingCounter*PAGE_SIZE)+USER_HEAP_START;
				arrPageInfo[markingCounter].size=size;
				arrPageInfo[markingCounter].id=idVAR.id;
				arrPageInfo[markingCounter].virtual_address=idVAR.virtual_address;

				markingCounter++;
		}
		struct Page_Info idVAR;
		idVAR.virtual_address=(uint32)startVA;
//		int mask=0x7FFFFFFF;
//		int virtual_address_id=(int)startVA&mask;
		idVAR.id=ret;
		//cprintf("id = %d \n",idVAR.id);
//		for(int i=0;i<(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;i++)
//			{
//
//				if(id_search_array[i].id==0 && id_search_array[i].va==0)
//				{
//					id_search_array[i].id=idVAR.id;
//					id_search_array[i].va=idVAR.va;
//				}
//			}

		//sys_allocate_user_mem(startVA,size);
        return (void*)startVA;
	 }

   }
    return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	//Write your code here, remove the panic and write your code
    //panic("sget() is not implemented yet...!!");
	uint32 size=sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	//cprintf("sgettttttt\n");
	if(size==0)
	{
		return NULL;//mal2ash 7aga
	}
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
		uint32 startVA=myEnv->uLimit+PAGE_SIZE;
		uint32 no_of_pages=size/PAGE_SIZE;
		uint32 remainder=size%PAGE_SIZE;
		uint32 startIndex=(startVA-USER_HEAP_START)/PAGE_SIZE;
		uint32 increment=0;
		if(remainder!=0)
		{
			no_of_pages++;
		}
		uint32 index=(startVA-USER_HEAP_START)/PAGE_SIZE;
		while(increment<no_of_pages){
			if(arrPageInfo[index].marked ==0){
				increment++;
				index++;
			}
			else{
			    increment=0;
				startIndex = index+((ROUNDUP(arrPageInfo[index].size,PAGE_SIZE))/PAGE_SIZE);
				index=startIndex;
			}
			if((index*PAGE_SIZE)+USER_HEAP_START>=USER_HEAP_MAX){
				return NULL;
			}
		}

		//cprintf("khalasna 3ala kheir w hanendah harry \n");
		startVA= (startIndex*PAGE_SIZE)+USER_HEAP_START;
		int x=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)startVA);
		struct Page_Info idVAR;
				idVAR.virtual_address=(uint32)startVA;
		//		int mask=0x7FFFFFFF;
		//		int virtual_address_id=(int)startVA&mask;
				idVAR.id=x;
		if(x == E_SHARED_MEM_NOT_EXISTS)
			return NULL;
		//cprintf("abl for f sget\n");
		uint32 markingCounter = startIndex;
					for(int i=0;i<no_of_pages;i++){
						arrPageInfo[markingCounter].marked=1;
						arrPageInfo[markingCounter].virtual_address=(markingCounter*PAGE_SIZE)+USER_HEAP_START;
						arrPageInfo[markingCounter].size=size;
						arrPageInfo[markingCounter].id=idVAR.id;
						arrPageInfo[markingCounter].virtual_address=idVAR.virtual_address;
						markingCounter++;
					}
		//cprintf("a5r sget\n");
	    return (void *)	startVA;
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

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");
    //sys_freeSharedObject();
//	cprintf("address in sfree%p/n",virtual_address);

	if((uint32)virtual_address>=((myEnv->uLimit)+PAGE_SIZE)){
//				cprintf("pageeee\n");
				uint32 size_allocated =arrPageInfo[((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE].size;
				//cprintf("1\n");

	            uint32 no_of_page_allocated =size_allocated/PAGE_SIZE;
				//cprintf("2\n");

	            uint32 reminder=size_allocated%PAGE_SIZE;
				//cprintf("3\n");

	             if(reminder!=0)
		            no_of_page_allocated++;
				uint32 start_index=((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE;
				//cprintf("4\n");

			    while(no_of_page_allocated!=0){
					//cprintf("5\n");

			    	arrPageInfo[start_index].virtual_address=0;
					//cprintf("6\n");

			    	arrPageInfo[start_index].marked=0;
					//cprintf("7\n");

			    	arrPageInfo[start_index].size=0;
					//cprintf("8\n");

			    	start_index++;
			    	no_of_page_allocated--;
		     	}
	}
				//cprintf("9\n");
//	       int mask=0x7FFFFFFF;
//	       int virtual_address_id=(int)virtual_address&mask;
		int final_id;
//		for(int i=0;i<(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;i++)
//		{
//			if(id_search_array[i].va==(uint32)virtual_address)
//			{
//				final_id=id_search_array[i].id;
//				break;
//			}
//		}
	       sys_freeSharedObject(arrPageInfo[((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE].id,virtual_address);
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
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
