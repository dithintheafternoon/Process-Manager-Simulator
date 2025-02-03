#include <stdio.h>
#include <stdlib.h>
#include "datastructs.h"
#include <string.h>
#include "memory.h"
#include <math.h>


int main(int argc, char* argv[]){

    //make a 'cpu' to store information
    struct cpu_info ci = {0, 0};
    int mem;

    //this is our memory object. We don't know what kind of memory it is going to be yet
    void *m;

    //receive commands and save them in a process_queue
    struct process_queue *pq = take_commands(argc, argv);

    int turnaround = 0;
    double overhead_max = 0;
    double overhead_av = 0;
    int makespan = 0;

    //making a ready queue
    struct ready_queue *rq = (struct ready_queue *)malloc(sizeof(struct ready_queue));
    if(rq == NULL){
        printf("rq malloc failed\n");
    }

    rq->head = NULL;
    rq->tail = NULL;
    rq->n_processes = 0;
    rq->empty = 1;

    //making appropriate memory data structures
    if(strcmp(pq->mem_type, "first-fit") == 0){
        mem = FIRSTFIT;
        m = (struct block_memory *)initialise_block_mem();
        //printf("block memory created. we cast mem object to block_memory\n");
    } else if(strcmp(pq->mem_type, "paged") == 0){
        mem = PAGED;
        m = (struct paged_memory *)initialise_paged_mem();
    } else if(strcmp(pq->mem_type, "virtual") == 0){
        //printf("virtual. initialise paged mem\n");
        mem = VIRTUAL;
        m = (struct paged_memory *)initialise_paged_mem();
    }else{
        mem = INFINITE;
        //printf("no memory required\n");
        m = NULL;
    }


    
    
    //while the process queue still has unready processes, or while there are still unfinished processes in ready queue
    while(ci.process_queue_i < pq->n_processes || rq->empty != 1){


        int dont_increment = 0;
        //int virtual_mem = 0;
        
        //still unready processes in pq
        if(ci.process_queue_i < pq->n_processes){
            //add all ready processes to ready queue

            //check if next process in the process queue is ready yet, make sure not to go past pq index
                //the queue_i will point to the next process in the queue that has not yet been added to ready queue
            while((ci.process_queue_i < pq->n_processes) && (pq->queue[ci.process_queue_i]->arrival_time <= ci.simulation_time)){
                //printf("add process: %s\n", pq->queue[ci.process_queue_i]->name);
                
                pq->queue[ci.process_queue_i]->state = READY;
                add_to_rq(rq, pq->queue[ci.process_queue_i], pq->quantum);
                

                if(rq->empty == 1){
                    //rq is no longer empty
                    rq->empty = 0;
                }

                ci.process_queue_i++; 
            }

        }
        
        if(rq->empty != 1){
            struct process *curr_process = rq->head;

            double this_overhead;
            int this_turnaround;

            //allocating memory
            if(curr_process->allocated == 0){
                int n_pages = (int)ceil((double)curr_process->size / PFSIZE);
                if(mem == INFINITE){
                    curr_process->allocated = 1;
                    curr_process->address = NULL;
                } else if(mem == FIRSTFIT){
                    if(allocate_block_memory(((struct block_memory *)m)->block_list, curr_process) == 1){
                        curr_process->allocated = 1;
                    }
                } else if(mem == PAGED){
                    
                    allocate_paged_memory(((struct paged_memory *)m), curr_process, n_pages, rq, ci.simulation_time);

                    //ALLOCATE FRAMES
                } else if(mem == VIRTUAL){
                    curr_process->v_mem = allocate_paged_virtual_memory(((struct paged_memory *)m), curr_process, n_pages, rq, ci.simulation_time);

                }else{
                    //ALLOCATE BASED ON VIRTUAL
                }
            }

            //running process
            if(curr_process->allocated == 1){
                if(curr_process->state == READY){
                   
                    curr_process->state = RUNNING;
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d", ci.simulation_time, curr_process->name, curr_process->time_left);

                    //awful print statements//////////////////////
                    if(mem == FIRSTFIT){
                        printf(",mem-usage=%d%%,allocated-at=%d\n", (int)ceil(((double)(MEMMAX - ((struct block_memory *)m)->block_list->remaining_memory)/MEMMAX) * CONV),
                            ((struct memory_block *)(curr_process->address))->start);
                    } else if(mem == PAGED || mem == VIRTUAL){
                        
                        int frames_print = (int)ceil((double)curr_process->size / PFSIZE);
                        if(mem == VIRTUAL){
                            frames_print = curr_process->v_mem;
                        }
                        printf(",mem-usage=%d%%", (int)ceil(((double)(MEMMAX - (((struct paged_memory *)m)->remaining * PFSIZE))/MEMMAX) * CONV));
                        printf(",mem-frames=[");
                        for(int i = 0;i< frames_print;i++){
                            printf("%d", ((int *)(curr_process->address))[i]);
                            if(i != frames_print - 1){
                                printf(",");
                            }
                        }
                        printf("]\n");
                    } 
                     else{
                        printf("\n");
                    }
                    /////////////////////////////////////////////

                }
                curr_process->time_left = curr_process->time_left - pq->quantum;

                //if a process finished
                if(curr_process->time_left <= 0){
                    curr_process->state = FINISHED;
                    curr_process->allocated = 0;

                    //if need to evict memory
                    if(mem != INFINITE){
                        if(mem != FIRSTFIT){
                            printf("%d,EVICTED,evicted-frames=", ci.simulation_time+pq->quantum);
                            int process_pages = (int)ceil((double)(curr_process->size)/PFSIZE); 
                            if(mem == VIRTUAL){
                                process_pages = curr_process->v_mem;
                            }
                            printf("[");
                            for(int i = 0;i< process_pages;i++){
                                printf("%d", ((int *)curr_process->address)[i]);
                                if(i != process_pages - 1){
                                    printf(",");
                                }
                            }
                            printf("]\n");
                            //DEALLOCATE FOR PAGED AND VIRTUAL 
                            evict_finished((struct paged_memory *)m, curr_process, mem);
                            //printf("SUCCESS\n");
                            //return 0; 
                        } else{
                            //DEALLOCATE FOR FIRSTFIT
                            free_process_block_memory(((struct block_memory *)m)->block_list, curr_process);
                        }
                    }

                    //function does not change process state
                    remove_from_ready_queue(rq);
                    
                    //NEED TO LOOP TO CHECK IF THERE ARE ANY PROCESSES THAT ARRIVED AT THIS EXACT TIME
                        //THAT HAVENT BEEN ACCOUNTED FOR YET
                    int x = 0;
                    int new_procs = 0;
                    while((x < pq->n_processes) && (pq->queue[x]->arrival_time <= (ci.simulation_time + pq->quantum))){
                        
                        if(pq->queue[x]->arrival_time == (ci.simulation_time + pq->quantum)){
                            new_procs++;
                        }
                        x++;
                    }

                    printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", ci.simulation_time+pq->quantum, curr_process->name, rq->n_processes+new_procs);

                    curr_process->end_time = ci.simulation_time + pq->quantum;

                    //stat making
                    this_turnaround = curr_process->end_time - curr_process->arrival_time;
                    this_overhead = (double)this_turnaround/curr_process->service_time;
                    turnaround += this_turnaround;
                    overhead_av += this_overhead;
                    if(overhead_max < this_overhead){
                        overhead_max = this_overhead;
                    }

                } else{
                    move_to_tail(rq);
                }
            } else if(mem == FIRSTFIT){

                dont_increment = 1;

                move_to_tail(rq);
                
            } else{

                //evict pages for paged and virtual
            }
            if(dont_increment != 1){
                curr_process->last_exec = ci.simulation_time;
            }
        }
        if(dont_increment != 1){
            
            ci.simulation_time+=pq->quantum;
        }
    }

    makespan = ci.simulation_time;


    turnaround = (int)ceil((double)turnaround/(pq->n_processes));

    overhead_av = overhead_av/(pq->n_processes);

    printf("Turnaround time %d\n",turnaround);
    printf("Time overhead %.2f %.2f\n", overhead_max, overhead_av);
    printf("Makespan %d\n", makespan);

    free_process_queue(pq, mem);
    free_ready_queue(rq);

    //freeing all memory
    if(mem != INFINITE){
        free_mem(mem, m);
    } 
   
    return 0; 
}