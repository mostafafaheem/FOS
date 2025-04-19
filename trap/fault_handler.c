/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/kheap.h>

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//==================
// [1] MAIN HANDLER:
//==================
/*2022*/
uint32 last_eip = 0;
uint32 before_last_eip = 0;
uint32 last_fault_va = 0;
uint32 before_last_fault_va = 0;
int8 num_repeated_fault  = 0;

struct Env* last_faulted_env = NULL;
void fault_handler(struct Trapframe *tf)
{
	/******************************************************/
	// Read processor's CR2 register to find the faulting address
	uint32 fault_va = rcr2();
	//cprintf("\n************Faulted VA = %x************\n", fault_va);
		//print_trapframe(tf);
	/******************************************************/

	//If same fault va for 3 times, then panic
	//UPDATE: 3 FAULTS MUST come from the same environment (or the kernel)
	struct Env* cur_env = get_cpu_proc();
	if (last_fault_va == fault_va && last_faulted_env == cur_env)
	{
		num_repeated_fault++ ;
		if (num_repeated_fault == 3)
		{
			print_trapframe(tf);
			panic("Failed to handle fault! fault @ at va = %x from eip = %x causes va (%x) to be faulted for 3 successive times\n", before_last_fault_va, before_last_eip, fault_va);
		}
	}
	else
	{
		before_last_fault_va = last_fault_va;
		before_last_eip = last_eip;
		num_repeated_fault = 0;
	}
	last_eip = (uint32)tf->tf_eip;
	last_fault_va = fault_va ;
	last_faulted_env = cur_env;
	/******************************************************/
	//2017: Check stack overflow for Kernel
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	if (!userTrap)
	{
		struct cpu* c = mycpu();
		//cprintf("trap from KERNEL\n");
		if (cur_env && fault_va >= (uint32)cur_env->kstack && fault_va < (uint32)cur_env->kstack + PAGE_SIZE)
			panic("User Kernel Stack: overflow exception!");
		else if (fault_va >= (uint32)c->stack && fault_va < (uint32)c->stack + PAGE_SIZE)
			panic("Sched Kernel Stack of CPU #%d: overflow exception!", c - CPUS);
#if USE_KHEAP
		if (fault_va >= KERNEL_HEAP_MAX)
			panic("Kernel: heap overflow exception!");
#endif
	}
	//2017: Check stack underflow for User
	else
	{
		//cprintf("trap from USER\n");
		if (fault_va >= USTACKTOP && fault_va < USER_TOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	//cprintf("curenv = %x\n", curenv);
	struct Env* faulted_env = cur_env;
	if (faulted_env == NULL)
	{
		print_trapframe(tf);
		panic("faulted env == NULL!");
	}
	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ( (faulted_env->env_page_directory[PDX(fault_va)] & PERM_PRESENT) != PERM_PRESENT)
	{
		// we have a table fault =============================================================
		//		cprintf("[%s] user TABLE fault va %08x\n", curenv->prog_name, fault_va);
		//		print_trapframe(tf);

		faulted_env->tableFaultsCounter ++ ;

		table_fault_handler(faulted_env, fault_va);
	}
	else
	{
		if (userTrap)
						{
							//============================================================================================/
							//TODO: [PROJECT'24.MS2 - #08] [2] FAULT HANDLER I - Check for invalid pointers
							//(e.g. pointing to unmarked user heap page, kernel or wrong access rights),
							//your code is here
							//cprintf("CHECK INVALID POINTERS\n");
							//cprintf("virtual address in pointers %p\n",fault_va);
							unsigned int page_perm = pt_get_page_permissions(faulted_env->env_page_directory,fault_va);

							if(fault_va >= USER_LIMIT  && fault_va <= KERNEL_HEAP_MAX)
							{
								//cprintf("CHECK INVALID POINTERS 1st if\n");
								env_exit();
							}

							else if((page_perm & PERM_PRESENT)==PERM_PRESENT){
								//cprintf("CHECK INVALID POINTERS 2nd if\n");
								//env_exit();
								if((page_perm & PERM_WRITEABLE)!=PERM_WRITEABLE){
								   //cprintf("CHECK INVALID POINTERS 3rd if\n");
								   env_exit();
								}
							}

							else if(fault_va <= USER_HEAP_MAX && fault_va >= USER_HEAP_START)
							{
									//cprintf("CHECK INVALID POINTERS 4th if\n");
									if((page_perm & PERM_MARKED)!=PERM_MARKED)
									{
										//cprintf("CHECK INVALID POINTERS 5th if\n");
												env_exit();
									}
													//env_exit();
							}
							else
							{
								//cprintf("BENMOOOOTTTTT!!!!!!!\n");
							}

							//============================================================================================/
						}
		/*2022: Check if fault due to Access Rights */
		int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
		if (perms & PERM_PRESENT)
			panic("Page @va=%x is exist! page fault due to violation of ACCESS RIGHTS\n", fault_va) ;
		/*============================================================================================*/


		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter ++ ;

		//		cprintf("[%08s] user PAGE fault va %08x\n", curenv->prog_name, fault_va);
		//		cprintf("\nPage working set BEFORE fault handler...\n");
		//		env_page_ws_print(curenv);

		if(isBufferingEnabled())
		{

			__page_fault_handler_with_buffering(faulted_env, fault_va);
		}
		else
		{
			//page_fault_handler(faulted_env, fault_va);
			page_fault_handler(faulted_env, fault_va);
		}
		//		cprintf("\nPage working set AFTER fault handler...\n");
		//		env_page_ws_print(curenv);


	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/
}

//=========================
// [2] TABLE FAULT HANDLER:
//=========================
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//=========================
// [3] PAGE FAULT HANDLER:
//=========================
void page_fault_handler(struct Env * faulted_env, uint32 fault_va)
{

#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(faulted_env->page_WS_list));
#else
		int iWS =faulted_env->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(faulted_env);
#endif
		if(isPageReplacmentAlgorithmNchanceCLOCK()){

		if(wsSize < (faulted_env->page_WS_max_size))
		{
			//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
			//TODO: [PROJECT'24.MS2 - #09] [2] FAULT HANDLER I - Placement
			fault_va= ROUNDDOWN(fault_va,PAGE_SIZE);
			struct FrameInfo* frame_info;
			acquire_spinlock(&MemFrameLists.mfllock);
			int ret=allocate_frame(&frame_info);
			release_spinlock(&MemFrameLists.mfllock);
			if(ret!=E_NO_MEM)
				{
					map_frame(faulted_env->env_page_directory,frame_info,(uint32)fault_va,PERM_WRITEABLE|PERM_USER);
				}
			int read_disk= pf_read_env_page(faulted_env,(uint32*)fault_va);
			if(read_disk==E_PAGE_NOT_EXIST_IN_PF)
					{
						if((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)||(fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
						{

						}
						else
						{

							env_exit();
						}
					}
					uint32* ptr_page_table = NULL;
					get_page_table(faulted_env->env_page_directory,(uint32)fault_va,&ptr_page_table);
					struct FrameInfo *ptr_frame_info=get_frame_info(faulted_env->env_page_directory,(uint32)fault_va,&ptr_page_table);

					struct WorkingSetElement* new_element=env_page_ws_list_create_element(faulted_env,(uint32)fault_va);


					LIST_INSERT_TAIL(&(faulted_env->page_WS_list),new_element);
					ptr_frame_info->wsElement=LIST_LAST(&(faulted_env->page_WS_list));
					ptr_frame_info->inWS=1;

					wsSize++;

					if(wsSize < (faulted_env->page_WS_max_size))
					{
						//cprintf("dakhal m4 full \n");

						//faulted_env->page_last_WS_element=faulted_env->page_last_WS_element->prev_next_info.le_next;
						faulted_env->page_last_WS_element=NULL;
					}
					else
					{
						//cprintf("dakhal full \n");
						faulted_env->page_last_WS_element=LIST_FIRST(&(faulted_env->page_WS_list));
					}
					//env_page_ws_print(faulted_env);



			//refer to the project presentation and documentation for details
		}
//		else if(1==1)
//			{
//				cprintf("gwa");
//			}
		else{

					//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
					//refer to the project presentation and documentation for details
					//TODO: [PROJECT'24.MS3] [2] FAULT HANDLER II - Replacement
					// Write your code here, remove the panic and write your code
					//env_page_ws_print(faulted_env);
//					cprintf("dakhal replacement\n");
					struct WorkingSetElement *first_element = LIST_FIRST(&faulted_env->page_WS_list);
					struct WorkingSetElement *last_element = LIST_LAST(&faulted_env->page_WS_list);
					struct WorkingSetElement *iterElement = faulted_env->page_last_WS_element; //bade2 mn foo2 a3taked
					struct WorkingSetElement *yahyoski;
					int maxSweeps=page_WS_max_sweeps;
					if(maxSweeps<0){
						maxSweeps=maxSweeps*-1;
					}
					while (1){
						//env_page_ws_print(faulted_env);
						uint32 iterElement_va = ROUNDDOWN(iterElement->virtual_address, PAGE_SIZE);
						if(page_WS_max_sweeps > 0){
							uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory,iterElement_va);
							if(iterElement->sweeps_counter >= maxSweeps && (permissions & PERM_USED)!= PERM_USED)
							{
								victimWSElement=iterElement;
								break;
							}
							else
							{
								uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory,iterElement_va);
								if((permissions & PERM_USED)!= PERM_USED) //PERM USED = 0
								{
									iterElement->sweeps_counter++;
								}
								else //PERM_USED = 1
								{
									iterElement->sweeps_counter=0;
									pt_set_page_permissions(faulted_env->env_page_directory, iterElement_va, 0,PERM_USED);
								}
							}
						}
							else // page_WS_max_sweeps < 0
							{

								uint32 modified_permissions = pt_get_page_permissions(faulted_env->env_page_directory,iterElement_va);
								if((modified_permissions & PERM_MODIFIED) == PERM_MODIFIED) //PERM_MODIFIED = 1
								{
									uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory, iterElement_va);
									if(iterElement->sweeps_counter >= maxSweeps+1 && (permissions & PERM_USED) != PERM_USED)
									{
										victimWSElement=iterElement;
										break;
									}
									else
									{
										//uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory, iterElement_va);
										if((permissions & PERM_USED) != PERM_USED) //PERM_USED = 0
										{
											iterElement->sweeps_counter++;
										}
										else
										{
											iterElement->sweeps_counter=0;
											pt_set_page_permissions(faulted_env->env_page_directory, iterElement_va, 0,PERM_USED);
										}
									}
								} //IF NOT MODIFIED
								else
								{
									uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory, iterElement_va);
									if(iterElement->sweeps_counter >= maxSweeps && (permissions & PERM_USED)!= PERM_USED)
									{
										victimWSElement=iterElement;
										break;
									}
									else
									{
										uint32 permissions = pt_get_page_permissions(faulted_env->env_page_directory, iterElement_va);
										if((permissions & PERM_USED) != PERM_USED)
										{
											iterElement->sweeps_counter++;
										}
										else
										{
											iterElement->sweeps_counter=0;
											pt_set_page_permissions(faulted_env->env_page_directory, iterElement_va, 0,PERM_USED);
										}
									}
								}
							}

							if (iterElement == LIST_LAST(&faulted_env->page_WS_list))
							{
								iterElement = LIST_FIRST(&faulted_env->page_WS_list);
							}
							else
								{
								//cprintf("%x %x\n",iterElement, LIST_NEXT(iterElement));
								iterElement = LIST_NEXT(iterElement);
								}

					}
					//remove the victim and place the frame

					uint32* ptr_page_table = NULL;
					//int ret = get_page_table(faulted_env->env_page_directory, victimWSElement->virtual_address, &ptr_page_table);
					uint32 victim_va= ROUNDDOWN(victimWSElement->virtual_address, PAGE_SIZE);
					uint32 victim_permissions = pt_get_page_permissions(faulted_env->env_page_directory,victimWSElement->virtual_address );
					struct FrameInfo* frame_info= get_frame_info(faulted_env->env_page_directory,victimWSElement->virtual_address,&ptr_page_table);
					//lawo howa modified benrmy 3al disk
					if((victim_permissions & PERM_MODIFIED)== PERM_MODIFIED)
					{
						pf_update_env_page(faulted_env,victimWSElement->virtual_address, frame_info);
					}
					//
					faulted_env->page_last_WS_element = LIST_NEXT(victimWSElement);
					if(faulted_env->page_last_WS_element==NULL){
						faulted_env->page_last_WS_element=faulted_env->page_WS_list.lh_first;
					}
					unmap_frame(faulted_env->env_page_directory,victim_va);

	                //3amalna free w han3ml allocate
					struct FrameInfo* new_frame_info=NULL;
					int ret=allocate_frame(&new_frame_info);

					if(ret!=E_NO_MEM)
						{
							map_frame(faulted_env->env_page_directory,new_frame_info,(uint32)fault_va,PERM_WRITEABLE|PERM_USER | PERM_USED);
						}
					int read_disk= pf_read_env_page(faulted_env,(uint32*)fault_va);
					if(read_disk==E_PAGE_NOT_EXIST_IN_PF)
					{
						if((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)||(fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
						{

						}
						else
						{
							env_exit();
						}
					}

					struct WorkingSetElement* new_element=env_page_ws_list_create_element(faulted_env,(uint32)fault_va);

					LIST_INSERT_AFTER(&(faulted_env->page_WS_list),victimWSElement,new_element);

					LIST_REMOVE(&(faulted_env->page_WS_list), victimWSElement);
					//kfree(victimWSElement);
					//env_page_ws_print(faulted_env);
//					cprintf("dakhal final replacement\n");
				}
}
}




void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	//[PROJECT] PAGE FAULT HANDLER WITH BUFFERING
	// your code is here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");
}

