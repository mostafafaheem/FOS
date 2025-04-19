/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"
#include <inc/queue.h>

//extern void inctst();

/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va on the given page_directory
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, cut-paste the number of pages and return 0
//	ALL 12 permission bits of the destination should be TYPICAL to those of the source
//	The given addresses may be not aligned on 4 KB
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	panic("cut_paste_pages() is not implemented yet...!!");
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	If ANY of the destination pages exists with READ ONLY permission, deny the entire process and return -1.
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages doesn't exist, create it with the following permissions then copy.
//	Otherwise, just copy!
//		1. WRITABLE permission
//		2. USER/SUPERVISOR permission must be SAME as the one of the source
//	The given range(s) may be not aligned on 4 KB
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the //panic and write your code
	panic("copy_paste_chunk() is not implemented yet...!!");
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	It should set the permissions of the second range by the given perms
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, share the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	The given range(s) may be not aligned on 4 KB
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the //panic and write your code
	panic("share_chunk() is not implemented yet...!!");
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate the given virtual range [<va>, <va> + <size>) in the given address space  <page_directory> with the given permissions <perms>.
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, allocate the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	Allocation should be aligned on page boundary. However, the given range may be not aligned.
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the //panic and write your code
	panic("allocate_chunk() is not implemented yet...!!");
}

//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	panic("calculate_allocated_space() is not implemented yet...!!");
}

//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
//This function should calculate the required number of pages for allocating and mapping the given range [start va, start va + size) (either for the pages themselves or for the page tables required for mapping)
//	Pages and/or page tables that are already exist in the range SHOULD NOT be counted.
//	The given range(s) may be not aligned on 4 KB
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
/* DYNAMIC ALLOCATOR SYSTEM CALLS */
//=====================================
void* sys_sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the current user program to increase the size of its heap
	 * 				by the given number of pages. You should allocate NOTHING,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) As in real OS, allocate pages lazily. While sbrk moves the segment break, pages are not allocated
	 * 		until the user program actually tries to access data in its heap (i.e. will be allocated via the fault handler).
	 * 	2) Allocating additional pages for a process’ heap will fail if, for example, the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sys_sbrk fails, the net effect should
	 * 		be that sys_sbrk returns (void*) -1 and that the segment break and the process heap are unaffected.
	 * 		You might have to undo any operations you have done so far in this case.
	 */

	//TODO: [PROJECT'24.MS2 - #11] [3] USER HEAP - sys_sbrk
	struct Env* env = get_cpu_proc(); //the current running Environment to adjust its break limit
	uint32 old_brk =env->uBrk;
	uint32 new_brk =ROUNDUP(env->uBrk+(numOfPages*PAGE_SIZE),PAGE_SIZE);
	//uint32 new_brk =ROUNDUP(env->uBrk+(numOfPages*PAGE_SIZE),PAGE_SIZE);
	//uint32 new_brk =env->uBrk+(numOfPages*PAGE_SIZE);
	if (numOfPages== 0)
	{
		return (void*)env->uBrk;
	}
	else if(new_brk <= env->uLimit)
	{
		uint32 *ptr_page_table;
		uint32 va= env->uBrk;
		env->uBrk+=(numOfPages*PAGE_SIZE);
		for (int i = 0; i < numOfPages; i++)
		{
			get_page_table(env->env_page_directory,va,&ptr_page_table);
			if(ptr_page_table==NULL){
				create_page_table(env->env_page_directory,va);
				get_page_table(env->env_page_directory,va,&ptr_page_table);
			}
			//ptr_page_table[PTX(va)] = ptr_page_table[PTX(va)] | PERM_MARKED;
			pt_set_page_permissions(env->env_page_directory,(uint32)va,PERM_MARKED,0);

			//allocate_user_mem()
			va += PAGE_SIZE;
			//va=va+sizeof(int);
		}

		return (void*) old_brk;

	}
	else
	{
		return (void*)-1;
	}
}

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
//	return;
	/*====================================*/

	//TODO: [PROJECT'24.MS2 - #13] [3] USER HEAP [KERNEL SIDE] - allocate_user_mem()
	// Write your code here, remove the panic and write your code
	//panic("allocate_user_mem() is not implemented yet...!!");
	uint32 num_of_frames = size/(PAGE_SIZE); //
	uint32 remainder = size%PAGE_SIZE;//
	virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE);
	if (remainder>0){
		num_of_frames++;
		}
	uint32* ptr_page_table=NULL;
	//cprintf("ehna abl el for 3and harry \n");
	for(int i=0;i<num_of_frames;i++){
		get_page_table(e->env_page_directory,virtual_address,&ptr_page_table);
		if(ptr_page_table==NULL){
			create_page_table(e->env_page_directory,virtual_address);
			get_page_table(e->env_page_directory,virtual_address,&ptr_page_table);
		}
		//cprintf("ehna abl el perm marked \n");
		//ptr_page_table[PTX(virtual_address)]|=PERM_MARKED;
		pt_set_page_permissions(e->env_page_directory,(uint32)virtual_address,PERM_MARKED,0);
		virtual_address=virtual_address+PAGE_SIZE;
	}
	return;
}


//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
//	inctst();
//	return;
	/*====================================*/

	//TODO: [PROJECT'24.MS2 - #15] [3] USER HEAP [KERNEL SIDE] - free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");
//cprintf("free user mem\n");
	uint32 num_of_frames = ROUNDUP(size,PAGE_SIZE)/(PAGE_SIZE); //
	//PERM_MARKED=1;
	virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE);
    uint32 va=virtual_address;
	uint32* ptr_page_table=NULL;
	struct FrameInfo *ptr_frame_info=NULL;

	get_page_table(e->env_page_directory,virtual_address,&ptr_page_table);
	bool sizeMax = LIST_SIZE(&(e->page_WS_list)) == (e->page_WS_max_size);
//	cprintf("FREEUSERMEM BEFORE FREEING\n");
//	env_page_ws_print(e);

	for(int i=0;i<num_of_frames;i++){

		if(ptr_page_table==NULL){
			create_page_table(e->env_page_directory,virtual_address);
		}
		get_page_table(e->env_page_directory,virtual_address,&ptr_page_table);
		ptr_frame_info = get_frame_info(e->env_page_directory,(uint32)virtual_address,&ptr_page_table);

		//pt_clear_page_table_entry(e->env_page_directory, virtual_address);

		pt_set_page_permissions(e->env_page_directory,(uint32)virtual_address,0,PERM_MARKED);
		pf_remove_env_page(e, virtual_address);
		if (ptr_frame_info!=0){
			env_page_ws_invalidate(e, virtual_address);
		}
//		if (ptr_frame_info!=0){
//			//env_page_ws_print(e);
//
//			if (ptr_frame_info->inWS){
//
//				unmap_frame(e->env_page_directory, ptr_frame_info->wsElement->virtual_address);
//
//				if (e->page_last_WS_element == ptr_frame_info->wsElement)
//				{
//					e->page_last_WS_element = LIST_NEXT(ptr_frame_info->wsElement);
//				}
//				LIST_REMOVE(&(e->page_WS_list), ptr_frame_info->wsElement);
//
////				kfree(ptr_frame_info->wsElement);
//
//
//				ptr_frame_info->inWS=0;
//				ptr_frame_info->wsElement=NULL;
//			}
//		}
		virtual_address=virtual_address+PAGE_SIZE;
	}
//	cprintf("FREEUSERMEM BEFORE SORT\n");
//	env_page_ws_print(e);
	struct WorkingSetElement* fajita;
//	cprintf("LIST SIZE: %d\n", LIST_SIZE(&(e->page_WS_list)));
	if(sizeMax==0)
	{
	}
	else
	{
		LIST_FOREACH(fajita, &(e->page_WS_list))
		{
//			cprintf("FUM");
			if(fajita==e->page_last_WS_element){
				break;
			}
			LIST_REMOVE(&(e->page_WS_list), fajita);
			LIST_INSERT_TAIL(&(e->page_WS_list),fajita);
		}
	}

//	cprintf("FREEUSERMEM AFTER SORT\n");
//	env_page_ws_print(e);



return;
	//TODO: [PROJECT'24.MS2 - BONUS#3] [3] USER HEAP [KERNEL SIDE] - O(1) free_user_mem
}



//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//[PROJECT] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

