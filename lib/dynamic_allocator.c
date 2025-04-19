/*
 * dynamic_allocator.c
 *
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
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
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
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	//cprintf("The value of daStart is 0x%x \n", daStart);

	//heap boundaries
	uint32 *BEG_Block;
	uint32 *END_Block;
	//allocating boundaries (done)
	BEG_Block=(uint32*)daStart;
	*BEG_Block=1;
	uint32 end_sum=daStart+initSizeOfAllocatedSpace-sizeof(int);
	END_Block=(uint32*)end_sum;
	*END_Block=1;
//	cprintf("the address of the begin block is 0x%x \n",BEG_Block);
//	cprintf("the value of the begin block is %d \n",*BEG_Block);
//	cprintf("the allocated space is 0x%x \n",initSizeOfAllocatedSpace);
//	cprintf("the address of the end block is 0x%x \n",END_Block);
//	cprintf("the value of the end block is %d \n",*END_Block);
//	cprintf("the value of the end sum is 0x%x \n",end_sum);
	//header and footer
	uint32 *Header=(uint32*)daStart+1;
	//allocted -8 bytes due to the size(flag+size)
	uint32 SizeofHeader_Footer=initSizeOfAllocatedSpace-2*sizeof(int);
	uint32 footer_add=daStart+initSizeOfAllocatedSpace-2*sizeof(int);
	uint32 *Footer=(uint32*)footer_add;
	*Footer=SizeofHeader_Footer;
	*Header=SizeofHeader_Footer;
	//cprintf("the address of the header is 0x%x \n",Header);
	//cprintf("the address of the footer is 0x%x \n",Footer);
	//first free block
	struct BlockElement* first_Free_Block;
	first_Free_Block=(struct BlockElement*)daStart+1;
	first_Free_Block->prev_next_info.le_next=NULL;
	first_Free_Block->prev_next_info.le_prev=NULL;


	//cprintf("the address of the first free block is 0x%x \n",first_Free_Block);



	//setting the list
	LIST_INIT(&freeBlocksList);
	LIST_INSERT_HEAD(&freeBlocksList,first_Free_Block);
	//cprintf("initialized done\n");


}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	uint32 Calc_size;
	//cprintf("the total size is %d \n",totalSize);
	if(totalSize%2==0)//even size
	{
		if(isAllocated)//last bit should be 1
		{
			Calc_size=totalSize+1;
			//cprintf("the cal size(even allo) is %d \n",Calc_size);
		}
		else
		{
			Calc_size=totalSize;
			//cprintf("the cal size(even non allo) is %d \n",Calc_size);
		}
	}
	else //odd
	{
		totalSize++;
		if(isAllocated)//last bit should be 1
			{
				Calc_size=totalSize+1;
				//cprintf("the cal size is %d \n",Calc_size);
			}
		else
		{
			Calc_size=totalSize;
		}
	}
	uint32 *head_ptr=va-4; //head_ptr points to the header start
	*head_ptr=Calc_size;
	uint32 *footer_ptr=va+(totalSize-8);//footer_ptr points to footer start (remove the header)
	*footer_ptr=Calc_size;
//	cprintf("the address of the header is 0x%x \n",head_ptr);
//	cprintf("the address of the footer is 0x%x \n",footer_ptr);
//	cprintf("the value of the header is %d \n",*head_ptr);
//	cprintf("the value of the footer is %d \n",*footer_ptr);

	//cprintf("set block data done\n");

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
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
	//cprintf("allocate dynamiccccc\n");

//	struct BlockElement* blk ;
//	if(LIST_SIZE( &freeBlocksList) == 1)
//	{
//		LIST_FOREACH(blk,&freeBlocksList)
//			{
//			uint32 required_size;
//			uint32 da_break;
//			uint32 blk_start =(uint32) blk;
//			uint32 blk_end = blk_start + get_block_size(blk);
//		 if (blk_start == KERNEL_HEAP_START && blk_end == da_break) {
//			 da_break=blk_end;
//		 }
//		}
//	}
	if(size==0)
	{
		//cprintf("allocate size b zero\n");
		return NULL;
	}
	uint32 Totalsize=size+2*sizeof(int);//adding the header and footer
	struct BlockElement *iter_element;
	bool found=0;

	LIST_FOREACH(iter_element,&freeBlocksList)
	{
		//cprintf("the address of the iter_elemnt 0x%x \n",iter_element);
		uint32 blk_size=get_block_size(iter_element);
		if(blk_size>=16 && blk_size>=Totalsize)//law makan yekafy
		{
			if(blk_size==Totalsize)//law 3al 2ad bezabt
				{
					set_block_data(iter_element,Totalsize,1);
					//cprintf("set block data(equal blk sizes)\n");
					found=1;
					//removing it from the list
					LIST_REMOVE(&freeBlocksList,iter_element);
					return iter_element;
				}
			else if(blk_size-Totalsize<16)//internal frag
			{
				//cprintf("set block data(internal frag \n");
				//cprintf("the address of the iter_elemnt void 0x%x \n",(void*)iter_element);
				//cprintf("the address of the iter_elemnt 0x%x \n",iter_element);
				set_block_data(iter_element,blk_size,1);
				//removing it from the list
				LIST_REMOVE(&freeBlocksList,iter_element);
				found=1;
				return iter_element;
			}
			else if(blk_size-Totalsize>=16)//no internal frag (split)
			{
				struct BlockElement *new_element=(struct BlockElement*)((char*)iter_element + Totalsize);
				//pushing it it the free block list
				LIST_INSERT_AFTER(&freeBlocksList,iter_element,new_element);
				//set the block
				set_block_data(iter_element,Totalsize,1);
				//splitting(set the empty block)
				set_block_data(new_element,blk_size-Totalsize,0);
				//removing it from the list
				LIST_REMOVE(&freeBlocksList,iter_element);
				found=1;
				//cprintf("set block data(splitt) \n");
				return iter_element;
			}
		}
	}
	if(found==0)
	{
		//print_blocks_list(freeBlocksList);
		//panic("ii");
		//cprintf("allocate ml2a4\n");
		void*sbrk_blk =sbrk(ROUNDUP(Totalsize,PAGE_SIZE)/PAGE_SIZE);
		//cprintf("b3d ma ndh sbrk\n");
		//uint32 last_element_size = get_block_size(last_element);
		//bool merge_ou_non;
		//cprintf("brk size:%d,free:%d \n",get_block_size(sbrk_blk),is_free_block(sbrk_blk));
		//panic("ooo");
		if(sbrk_blk==(void*)-1)
		{
			//cprintf("awl if \n");
			return NULL;
		}
		else
		{
			uint32 * New_End=(uint32*)((char*)sbrk_blk +(ROUNDUP(Totalsize,PAGE_SIZE)/PAGE_SIZE )*PAGE_SIZE - sizeof(int));
		    *New_End=1;
			set_block_data(sbrk_blk,(ROUNDUP(Totalsize,PAGE_SIZE)/PAGE_SIZE)*PAGE_SIZE,0);
			uint32* footer_of_block=sbrk_blk-sizeof(int);
			int8 freee=is_free_block(footer_of_block);
			   if(freee==0){//allocated
				   if(LIST_SIZE(&freeBlocksList)==0){
					   LIST_INSERT_HEAD(&freeBlocksList,(struct BlockElement*)((char*)sbrk_blk));
				   }
				   else{
				   LIST_INSERT_TAIL(&freeBlocksList,(struct BlockElement*)((char*)sbrk_blk));
				   }
			   }else{//free
				   uint32 * pointer_to_free_header=(uint32*)((char*) footer_of_block-get_block_size(footer_of_block)+sizeof(int));
				   set_block_data(pointer_to_free_header,get_block_size(pointer_to_free_header)+get_block_size(sbrk_blk),0);
				   struct BlockElement * mergedblock= (struct BlockElement *)pointer_to_free_header;
				   LIST_INSERT_AFTER(&freeBlocksList,(struct BlockElement * )pointer_to_free_header,mergedblock);
				   LIST_REMOVE(&freeBlocksList,(struct BlockElement*)pointer_to_free_header);
			   }
			 void* p=alloc_block_FF(size);
			 return p;
		}
	}
	return NULL;
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
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

	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...
	if(size==0)
		{
			return NULL;
		}
		uint32 Totalsize=size+2*sizeof(int);//adding the header and footer
		struct BlockElement *iter_element;
		struct BlockElement *bestFitBlock = NULL;
		uint32 bestFitBlockSize = 4294967295U; //variable keber fel awel aashan nqaren bih
		uint32 blk_size;

		bool found=0;
		//coalesce then search or search then coalesce??
		LIST_FOREACH(iter_element,&freeBlocksList)
		{
			blk_size=get_block_size(iter_element);

			if((blk_size>=Totalsize)&&(blk_size<=bestFitBlockSize)){
				bestFitBlock=iter_element;
				bestFitBlockSize=blk_size;
			}
		}

		if (bestFitBlock==NULL)
			found=0;
		else{
			if(bestFitBlockSize==Totalsize)//law 3al 2ad bezabt
			{
				set_block_data(bestFitBlock,Totalsize,1);
				cprintf("set block data(equal blk sizes bf)\n");
				found=1;
				//removing it from the list

				LIST_REMOVE(&freeBlocksList,bestFitBlock);
				return bestFitBlock;
			}
			else if(bestFitBlockSize-Totalsize<16)//internal frag
			{
				cprintf("set block data(internal frag) bf\n");
				//cprintf("the address of the iter_elemnt void 0x%x \n",(void*)iter_element);
				//cprintf("the address of the iter_elemnt 0x%x \n",iter_element);
				set_block_data(bestFitBlock,bestFitBlockSize,1);
				//removing it from the list
				LIST_REMOVE(&freeBlocksList,bestFitBlock);
				found=1;

				return bestFitBlock;
			}
			else if(bestFitBlockSize-Totalsize>=16)//no internal frag (split)
			{
				struct BlockElement *new_element=(struct BlockElement*)((char*)bestFitBlock + Totalsize);
				//pushing it it the free block list
				LIST_INSERT_AFTER(&freeBlocksList,bestFitBlock,new_element);
				//set the block
				set_block_data(bestFitBlock,Totalsize,1);
				//splitting(set the empty block)
				set_block_data(new_element,bestFitBlockSize-Totalsize,0);
				//removing it from the list
				LIST_REMOVE(&freeBlocksList,bestFitBlock);
				found=1;
				//cprintf("set block data(splitt) \n");
				return bestFitBlock;
			}
			else
				found=0;
		}

		if(found==0)
			{
				void*sbrk_blk = sbrk(Totalsize);
				if(sbrk_blk==(void*)-1)
				{
					return NULL;
				}
				else
				{
					alloc_block_FF(size);
					cprintf("set block data(sbrk) \n");
					return sbrk_blk;
				}
			}

		return NULL;
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
//	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
//	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	//panic("free_block is not implemented yet");
//	//Your Code is Here...
	if(va == NULL){
		return;
	}
	//handling if the va isn't allocated "masking to get least significant bit to know if it is allocated or not"
	if(is_free_block(va) == 1){
		cprintf("the address you provided is free");
		return;
		//print_blocks_list(freeBlocksList);
	}

	//we will see if the before and after blocks is allocated or not by footer or header"check if the size 0 and alloc 1"heap boundaries"

	uint32 *prev_blockptr_footer = (uint32 *)((char *)va-4);
	int8 notallocatedprev = is_free_block(prev_blockptr_footer);
	uint32 size_of_prev_block = get_block_size(prev_blockptr_footer);
	uint32 *prev_blockptr_header = (uint32 *)((char *)prev_blockptr_footer - (char *)size_of_prev_block+4);
	//////////////////////////////////////////////////////////////
	uint32 size_of_current_block = get_block_size(va);
	void *next_blockptr_header = (va + size_of_current_block);
	uint32 size_of_next_block = get_block_size(next_blockptr_header);
	int8 notallocatednext = is_free_block(next_blockptr_header);

	if(prev_blockptr_header!= NULL && next_blockptr_header!=NULL){

	if((notallocatedprev == 0) && (notallocatednext == 0))
		{
		set_block_data(va,size_of_current_block,0);
		struct BlockElement* block_va = ( struct BlockElement *)va;
		struct BlockElement* block;
		bool flag = 0;
		if(LIST_SIZE(&freeBlocksList) == 0)
		{
			LIST_INSERT_HEAD(&freeBlocksList,block_va);
		}
		else
		{
			LIST_FOREACH(block,&freeBlocksList)
			{
				if(block > block_va )
				{
					LIST_INSERT_BEFORE(&freeBlocksList,block,block_va);
					flag = 1;
					break;
				}
			}
			if(flag == 0)
				LIST_INSERT_TAIL(&freeBlocksList,block_va);
		}
		//print_blocks_list(freeBlocksList);
		}
	else if((notallocatedprev == 0) && (notallocatednext != 0))
	{

		uint32 newsize = size_of_current_block + size_of_next_block ;
		set_block_data(va,newsize,0);
		struct BlockElement* mergedblock = ( struct BlockElement *)va;
		struct BlockElement* nextblock = ( struct BlockElement *)next_blockptr_header;
		LIST_INSERT_BEFORE(&freeBlocksList,nextblock,mergedblock);
		LIST_REMOVE(&freeBlocksList,nextblock);
		//print_blocks_list(freeBlocksList);
	}
	else if((notallocatedprev != 0) && (notallocatednext == 0))
		{

			uint32 newsize = size_of_current_block + size_of_prev_block ;
			set_block_data(prev_blockptr_header,newsize,0);
			struct BlockElement* prevblock = ( struct BlockElement *)prev_blockptr_header;//to be removed from the freeblocklist
			LIST_REMOVE(&freeBlocksList,prevblock);
			struct BlockElement* block;
			struct BlockElement* mergedblock = ( struct BlockElement *)prev_blockptr_header;//to be added in the freeblocklist
			bool flag = 0;
			if(LIST_SIZE(&freeBlocksList) == 0)
					{
						LIST_INSERT_HEAD(&freeBlocksList,mergedblock);
					}
			else
			{
					LIST_FOREACH(block,&freeBlocksList)
					{
					if(block > mergedblock )
						{
						LIST_INSERT_BEFORE(&freeBlocksList,block,mergedblock);
						flag = 1;
						break;
						}
					}
						if(flag == 0){
							LIST_INSERT_TAIL(&freeBlocksList,mergedblock);
						}
			}
			//print_blocks_list(freeBlocksList);
		}
	else if((notallocatedprev != 0) && (notallocatednext != 0))
		{

			uint32 newsize = size_of_current_block + size_of_next_block + size_of_prev_block;
			set_block_data(prev_blockptr_header,newsize,0);
			struct BlockElement* mergedblock = ( struct BlockElement *)prev_blockptr_header;
			struct BlockElement* prevblock = ( struct BlockElement *)prev_blockptr_header;
			struct BlockElement* nextblock = ( struct BlockElement *)next_blockptr_header;
			LIST_REMOVE(&freeBlocksList,prevblock);
			LIST_INSERT_BEFORE(&freeBlocksList,nextblock,mergedblock);
			LIST_REMOVE(&freeBlocksList,nextblock);
			//print_blocks_list(freeBlocksList);
		}
	}
	else{
		cprintf("NULL!");
	}

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
	if(va != NULL && new_size==0){
		free_block(va);
		return NULL;
	}
	else if(va == NULL && new_size!=0)
		return alloc_block_FF(new_size);
	else if(va == NULL && new_size==0)
		return NULL;
	bool nxtIsEnough=0;
	struct BlockElement *listVA;
	struct BlockElement *listNewElement;
    uint32 TotalNewSize= new_size + 2*sizeof(uint32) ;
	uint32	current_block_size = get_block_size(va);
	uint32 new_block_size =  new_size;
	int8 isFreeCurrentBlock = is_free_block(va);
	void* new_Adrress = va;
	uint32 *prev_blockptr_footer = (uint32 *)((char *)va-4);
	int8 isFreePrev = is_free_block(prev_blockptr_footer);
	uint32 size_of_prev_block = get_block_size(prev_blockptr_footer);
    uint32 *prev_blockptr_header = (uint32 *)((char *)prev_blockptr_footer - (char *)size_of_prev_block+4);
    void *next_blockptr_header = (va + current_block_size);
    uint32 size_of_next_block = get_block_size(next_blockptr_header);
    int8 isAllocatedNext = is_free_block(next_blockptr_header);
    if(isFreeCurrentBlock)
    	return NULL;
    struct BlockElement *blockIterator;
	if (TotalNewSize % 2 != 0) TotalNewSize++;
    if(TotalNewSize<4*sizeof(int))
    	TotalNewSize = 4*sizeof(int);
	if(TotalNewSize <= current_block_size ){
		uint32 freeSpace = current_block_size - TotalNewSize;
		char *address_new_free_element=va + TotalNewSize;
		if(freeSpace >= 16){
			listVA= (struct BlockElement *)va;
			listNewElement= (struct BlockElement *)address_new_free_element;
			set_block_data(va,TotalNewSize,1);
			set_block_data(address_new_free_element,freeSpace,0);
			LIST_FOREACH(blockIterator,&freeBlocksList)
						{
							if(blockIterator > listNewElement )
							{
								LIST_INSERT_BEFORE(&freeBlocksList,blockIterator,listNewElement);
								break;
							}
						}
		}else{
			if(is_free_block(next_blockptr_header)){
			char *address_new_free_element=va + TotalNewSize;
			set_block_data(va,TotalNewSize,1);
			set_block_data(address_new_free_element,size_of_next_block+(current_block_size)-TotalNewSize,0);
			LIST_INSERT_BEFORE(&freeBlocksList,(struct BlockElement *)next_blockptr_header,(struct BlockElement *)address_new_free_element);
			LIST_REMOVE(&freeBlocksList,(struct BlockElement *)next_blockptr_header);
			}
			else
			set_block_data(va,current_block_size,1);
		}
		return va;
	}
	else{
		if(is_free_block(next_blockptr_header)){
			if(size_of_next_block+current_block_size >= TotalNewSize){
				nxtIsEnough=1;
				if(size_of_next_block+current_block_size-(TotalNewSize)<16){
			new_block_size=size_of_next_block+current_block_size;
			set_block_data(va,new_block_size,1);
			LIST_REMOVE(&freeBlocksList,(struct BlockElement *)next_blockptr_header);
				return va;
				}else{
					set_block_data(va,TotalNewSize,1);
					uint32 diffrence = size_of_next_block+current_block_size - TotalNewSize;
					char * address_new_free_element = va + TotalNewSize;
					set_block_data(address_new_free_element,diffrence,0);
					listNewElement=(struct BlockElement *)address_new_free_element;
					LIST_INSERT_BEFORE(&freeBlocksList,(struct BlockElement *)next_blockptr_header,(struct BlockElement *)address_new_free_element);
					LIST_REMOVE(&freeBlocksList , (struct BlockElement *)next_blockptr_header);
					return va;
				}
			}
		}
		else if(!is_free_block(next_blockptr_header)){
			uint32* newBlock = alloc_block_FF(TotalNewSize);
			free_block(va);
			return newBlock;
		}
		else if(!nxtIsEnough){
			uint32* newBlock = alloc_block_FF(TotalNewSize);
			free_block(va);
			return newBlock;
		}
		else{
			void*sbrk_blk = sbrk(TotalNewSize);
			if(sbrk_blk==(void*)-1)
			{
				return NULL;
			}
			else
			{
				set_block_data(sbrk_blk,TotalNewSize,1);
				return sbrk_blk;
			}
		}
}
	return va;
}

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
