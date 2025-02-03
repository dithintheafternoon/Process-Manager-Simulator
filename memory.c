#include <stdio.h>
#include <stdlib.h>
#include "datastructs.h"
#include <string.h>
#include "memory.h"
#include <math.h>

struct block_memory *initialise_block_mem(){
    struct block_list *bl = (struct block_list *)malloc(sizeof(struct block_list));

    struct memory_block *b = (struct memory_block *)malloc(sizeof(struct memory_block));

    struct block_memory *bm = (struct block_memory *)malloc(sizeof(struct block_memory));

    b->next = NULL;
    b->prev = NULL;
    b->start = 0;
    b->end = MEMMAX;
    b->size = MEMMAX;
    b->free = 1;
    b->process_name = NULL;
    b->id = 1;

    bl->head = b;
    bl->tail = b;
    bl->remaining_memory = MEMMAX;
    bl->n_blocks = 1;

    bm->block_list = bl;
    bm->largest_free_block = MEMMAX;

    return bm;
}

int merge_block_hole(struct block_list *q, struct memory_block *block){

    //the block given to the function will become the only block
    if(q->head != block){
        //don't need to merge with prev if this block is the head
        if(block->prev->free == 1){
            if(block->start != (block->prev->end)){
                printf("this block starts at %d, prev block ends at %d\n", block->start, block->next->end);
                printf("the prev memory block is not contiguous!\n");
                printf("    will not merge\n");
            } else{
                //the previous block is free. we will merge with prev block
                block->size = block->end - block->prev->start;
                block->start = block->prev->start;

                //we remove the previous block from the linkedlist
                if(q->head == block->prev){
                    //block shouls replace prev as head
                    q->head = block;
                } else{
                    //the prev of the prev should now connect to the block
                    (block->prev->prev)->next = block;
                }
                block->prev = block->prev->prev;
                q->n_blocks -=1;
            }
        } 
    }
    if(q->tail != block){
        if(block->next->free == 1){
            if(block->end != (block->next->start)){
                
            } else{
                block->size = block->next->end - block->start;
                block->end = block->next->end;

                if(q->tail == block->next){
                    q->tail = block;
                } else{
                    (block->next->next)->prev = block;
                }
                block->next = block->next->next;
                q->n_blocks -= 1; 
            }
        } 
    }

    return 1;
}

int free_process_block_memory(struct block_list *bl, struct process *p){
    ((struct memory_block *)(p->address))->free = 1;
    merge_block_hole(bl, (struct memory_block *)(p->address));
    p->allocated = 0;
    p->address = NULL;
    bl->remaining_memory = bl->remaining_memory + p->size;
    return 1;
}



struct memory_block *split_block(struct block_list *bl, struct memory_block *block, int p_size){
    //splits a block of memory and returns the block that should be allocated to the process
    //which is just the modified original block
    struct memory_block *process_block = (struct memory_block *)malloc(sizeof(struct memory_block));

    //adding attributes to the new block that will be occupied by the process
        //SPLIT BLOCK WILL NOT ASSIGN THE PROCESS TO THE BLOCK. NEED TO DO THIS OUTSIDE IN ALLOCATE MEMORY
    process_block->start = block->start;
    process_block->end = process_block->start + p_size;
    process_block->size = p_size;
    process_block->free = 0;
    
    //modifying the pre-existing block to become the hole
    block->start = block->start + p_size;
    block->size = block->size - p_size;
    block->free = 1;
    

    //adding the new block to the linkedlist
    process_block->next = block;
    process_block->prev = block->prev;
    block->prev = process_block;
    if(bl->head == block){
        bl->head = process_block;
        
    } 

    bl->n_blocks += 1;
    process_block->id = bl->n_blocks;
    return process_block;
}



int allocate_block_memory(struct block_list *bq, struct process *p){
    int size = p->size;
    struct memory_block *this_block = bq->head;
    if(bq->remaining_memory >= size){
        //there is enough space in memory
        while(this_block != NULL){
            //checking if this memory is contiguous
            if((this_block->free == 1) && (this_block->size >= size)){
                
                if(this_block->size != size){
                    this_block = split_block(bq, this_block, size);\
                }
                this_block->free = 0;
                this_block->process_name = p;
                p->allocated = 1;
                p->address = this_block;
                
                bq->remaining_memory = bq->remaining_memory - p->size;
                return 1;
            }
            this_block = this_block->next;
        }
        return 0;
    }
    return 0;
}

//int free_block_mem(struct block_list *q, struct process *p){}

int free_block_list(struct block_list *q){
    struct memory_block *curr_block = q->head;
    while(q->head != NULL){
        free(curr_block);
        curr_block = curr_block->next;
    }
    return 1;

}

int free_block_memory(struct block_memory *bm){

    struct memory_block *curr_block = bm->block_list->head;

    while(curr_block != NULL){
        struct memory_block *next_block = curr_block->next;
        free(curr_block);
        curr_block = next_block;
    }

    free(bm->block_list);

    free(bm);
    return 1;
}

int free_paged_memory(struct paged_memory *pm){

    free(pm);
    return 1;
}



//====================================================================================

//PAGED MEMORY
struct paged_memory *initialise_paged_mem(){
    struct paged_memory *pm = (struct paged_memory *)malloc(sizeof(struct paged_memory));
    //struct process *frames_array[NFRAMES];
        //each element in the fram array is a pointer to the process currently in the frame

    if(pm == NULL){
        printf("pm malloc unsuccessful\n");
    }

    for(int i = 0;i<NFRAMES;i++){
        pm->frames[i] = NOPROCESS;
    }

    pm->remaining = NFRAMES;

    return pm;
}

int evict_finished(struct paged_memory *pm, struct process *p, int memtype){
    int p_pages = (int)ceil((double)(p->size)/PFSIZE);
    if(memtype == VIRTUAL){
        p_pages = p->v_mem;
    }

    for(int i = 0;i< p_pages;i++){
        pm->frames[((int *)p->address)[i]] = NOPROCESS;
    }

    pm->remaining += p_pages;

    return 1;
}

int evict(struct paged_memory *pm, struct process *p, struct ready_queue *rq, int simtime){
    struct process *this_p;
    if(rq->tail == rq->head){
        this_p = rq->head;
    }else{
        this_p = rq->head->next;
    }

    int min = this_p->last_exec;
    struct process *last_exec_p = p;

    while(this_p != NULL){
        if((this_p->last_exec <= min) && (this_p->last_exec != -1) && (this_p->allocated == 1)){
            min = this_p->last_exec;
            last_exec_p = this_p;
            
        }
        this_p=this_p->next;
    }

    int npages = (int)ceil(((double)(last_exec_p->size)) / PFSIZE);
    
    for(int i = 0;i<npages;i++){
        pm->frames[((int *)last_exec_p->address)[i]] = NOPROCESS;
        pm->remaining ++;
    }
    last_exec_p->allocated = 0;

    //print evict statement for the process that was swapped
    printf("%d,EVICTED,evicted-frames=", simtime);
    printf("[");
    for(int i=0;i<npages;i++){
        printf("%d", ((int *)last_exec_p->address)[i]);
        if(i != npages - 1){
            printf(",");
        }
    }
    printf("]\n");
    return 1;
}




int allocate_paged_memory(struct paged_memory *pm, struct process *p, int n_pages, struct ready_queue *rq, int simtime){
    
    
    //casting address appropriately
        //P->ADDRESS IS NOW AN INT POINTER
    if(p->last_exec == -1){
        //don't want to be mallocing new space for a p->address that mgiht already exist
        p->address = (int *)malloc(sizeof(int) * n_pages);
    }
    
    int pages_allocated = 0;

    //if there's enough memory without evicting

    if(pm->remaining < n_pages){
        //NOT enough pages. evict
        while(pm->remaining < n_pages){
            evict(pm, p, rq, simtime);
        }
    } 

    if(pm->remaining >= n_pages){
        for(int i = 0;i<NFRAMES;i++){
            
            if(pm->frames[i] == NOPROCESS){
                pm->frames[i] = p->pq_i;
                ((int *)p->address)[pages_allocated] = i;
                pages_allocated ++;
                pm->remaining -- ;
                

                //in the event that this loop makes process take up entire memory and this is also the
                    //exact size of the process
                if(pages_allocated == n_pages){
                    for(int i = 0; i< pages_allocated;i++){
                    }
                    p->allocated = 1;
                    return 1;
                }
            }
            
        }
        
    }

    return 0;
}



int evict_virtual(struct paged_memory *pm, struct process *p, struct ready_queue *rq, int simtime){
    
    
    //process that causes an eviction will only receive 4 pages at most. Just needs to have 4 pages in total. 
    int extras_req = (VIRTMIN - p->v_mem) - pm->remaining;
    p->v_mem = VIRTMIN;

    //printf("%d extra pages requires\n", extras_req);
    
    //printf("looking for space ofr %s\n",p->name);

    struct process *this_p = (struct process *)malloc(sizeof(struct process));
    //traverse the ready queue. 
    if(rq->tail == rq->head){
        this_p = rq->head;
    }else{
        this_p = rq->head->next;
    }

    int min = LASTEXECMAX;
    struct process *last_exec_p = this_p;

    while(this_p != NULL){
        if((this_p->last_exec <= min) && (this_p->last_exec != -1) && (this_p->allocated == 1)){
            min = this_p->last_exec;
            last_exec_p = this_p;
        }
        this_p=this_p->next;
    }

    //evicting the frames and recording this in memory
    for(int i = 0;i<extras_req;i++){
        pm->frames[((int *)last_exec_p->address)[i]] = NOPROCESS;
        pm->remaining ++;
    }


    //print evict statement for the process that was swapped
    printf("%d,EVICTED,evicted-frames=", simtime);
    printf("[");
    for(int i=0;i<extras_req;i++){
        printf("%d", ((int *)last_exec_p->address)[i]);
        if(i != extras_req - 1){
            printf(",");
        }
    }
    printf("]\n");
    
    
    //we need to free the old memory address array and make a smaller one. We remove the smallest frame numbers
    last_exec_p->v_mem = last_exec_p->v_mem - extras_req;
    int *newaddress = (int *)malloc(sizeof(int) * last_exec_p->v_mem);

    //now copy all the kept frames to the new array
    int new_i = 0;
    //printf("the new fram array for process %s is [", last_exec_p->name);
    for(int i = extras_req;i<(extras_req+last_exec_p->v_mem);i++){
        newaddress[new_i] = ((int *)last_exec_p->address)[i];
        new_i++;
    }

    //now we free the old array and assign this new one
    free(last_exec_p->address);
    last_exec_p->address = newaddress;


    if(last_exec_p->v_mem < VIRTMIN){
        
        last_exec_p->allocated = 0;
       
    }

    free(this_p);
    return 1;
}

int allocate_paged_virtual_memory(struct paged_memory *pm, struct process *p, int n_pages, struct ready_queue *rq, int simtime){
    int n_alloced;
    int pages_allocated = 0;
    

    if((pm->remaining + p->v_mem) < VIRTMIN){
        //NOT enough pages. evict
       
        
        while((pm->remaining + p->v_mem) < VIRTMIN){
            //printf("need to evict\n");
            evict_virtual(pm, p, rq, simtime);
        }
        
    }

    
    //we malloc memory again for processes that have already run, and create new arrays for processes that havemt run\n");

    if(p->last_exec != -1){
        //has been allocated before, therefore has a preexisting malloced array
        free(p->address);
    }

    if(n_pages >= pm->remaining){
        p->address = (int *)malloc(sizeof(int) * (pm->remaining));
        n_alloced = pm->remaining;
    }else{
        p->address = (int *)malloc(sizeof(int) * n_pages);
        n_alloced = n_pages;
    }

    if(pm->remaining >= VIRTMIN){
        p->allocated = 1;
        for(int i = 0;i<NFRAMES;i++){
            if(pages_allocated == n_alloced){
                
                return n_alloced;
            }
                
            if(pm->frames[i] == NOPROCESS){
                
                pm->frames[i] = p->pq_i;
                ((int *)p->address)[pages_allocated] = i;
                pm->remaining -=1 ;
                
                pages_allocated ++;
                
            }
           
        }

        return n_alloced;
        
    }
    return n_alloced;
}

//====================================================================================
//GENERAL MEM

int free_mem(int mem, void *m){
    if(mem == FIRSTFIT){
        struct block_memory *bm = (struct block_memory *)m;
        free_block_memory(bm);
    } else if(mem == VIRTUAL || mem == PAGED){
        struct paged_memory *pm = (struct paged_memory *)m;
        free_paged_memory(pm);
    } else{
        
        return 0;
    }
    return 1;
}
