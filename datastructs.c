#include "datastructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//=================================================================================================
int read_file_new(char *filename, struct process_queue *pq){
    char *line = NULL;
    size_t line_length = 0;
    ssize_t read;

    int i;
    int n_processes = 0;

    FILE *file = fopen(filename, "r");

    if(file == NULL){
        printf("invalid file\n");
        return 0;
    }
    while((read = getline(&line, &line_length, file)) != -1){
        struct process *p = (struct process *)malloc(sizeof(struct process));

        p->allocated = 0;
        p->address = NULL;
        p->end_time = 0;
        p->v_mem = 0;
        
        //DON'T MAKE p->state = READY YET. DO THIS WHEN ADDED OT RQ

        i = 0;
        char *attribute = strtok(line, " ");
        while (attribute != NULL){
            if(i == 0){
                p->arrival_time = atoi(attribute);
            } else if(i == 1){
                p->name = (char *)malloc(sizeof(char)*(strlen(attribute)+1));
                strcpy(p->name, attribute);
            } else if(i == 2){
                p->service_time = atoi(attribute);
                p->time_left = atoi(attribute);
            } else if(i == 3){
                p->size = atoi(attribute);
            }
            attribute = strtok(NULL, " ");
            i++;
        }

        n_processes++;

        if(n_processes == pq->q_size){
            pq->q_size = (pq->q_size)*2;
            pq->queue = (struct process **)realloc(pq->queue, pq->q_size * sizeof(struct process *));
        }
        pq->queue[n_processes-1] = p;
        p->pq_i = n_processes-1;
    }

    pq->n_processes = n_processes;

    fclose(file);
    free(line);
    return 1;
}

struct process_queue *take_commands(int argc, char *argv[]){
    struct process_queue *pq = (struct process_queue *)malloc(sizeof(struct process_queue));
    if(pq == NULL){
        printf("pq alloc failed\n");
    }

    struct process **q = (struct process **)malloc(sizeof(struct process *)*SIZE);
    if(q == NULL){
        printf("q alloc failed\n");
    }

    pq->queue = q;
    pq->q_size = SIZE;

    for (int i = 1; i < argc-1; i+=2){

        //first element in argv is the program name
        if(strcmp(argv[i], "-f") == 0){
            read_file_new(argv[i+1], pq);
        } else if(strcmp(argv[i], "-m") == 0){
            pq->mem_type = argv[i+1];
            //printf("the mem type is: %s\n", pq->mem_type);
        } else if(strcmp(argv[i], "-q") == 0){
            pq->quantum = atoi(argv[i+1]);
            //printf("the quantum is: %d\n", pq->quantum);
        }
    }

    //successfully read the file
    return pq;
}

//=================================================================================================
    //processes
int move_to_tail(struct ready_queue *q){
    if(q->head == NULL || q->head == q->tail){
        //state will stay as running

        return 1;
    }

    struct process *old_head = q->head;

    if(q->head->next == q->tail){
        //if there are only two processes in the ready queue

        q->tail->next = old_head;
        old_head->prev = q->tail;
        old_head->next = NULL;
        q->tail->prev = NULL;
        q->head = q->tail;
        q->tail = q->head->next;
    } else{
        q->tail->next = old_head;
        old_head->prev = q->tail;
        q->head = old_head->next;
        old_head->next = NULL;
        q->tail = old_head;
        q->head->prev = NULL;
    }
    q->tail->state = READY;
    return 1;
}



int remove_from_ready_queue(struct ready_queue *q){
    if(q->head == NULL && q->tail == NULL){
        return 0;
    }
    if(q->head == q->tail){
        //there's only one element
        q->empty = 1;  
        q->head = NULL;
        q->tail = NULL;
    } else if(q->head->next == q->tail){
        //only 2 elems in linkedlist
        q->head = q->tail;
        q->head->prev = NULL;
    } else{
        q->head = q->head->next;
        q->head->prev = NULL;
    }

    q->n_processes = q->n_processes - 1;

    return 1;
}



int add_to_rq(struct ready_queue *q, struct process *p, int quantum){
    if(p == NULL){
        printf("error: can't add null process to ready q\n");
        return 0;
    }
    
    if(q->head == NULL){
        q->head = p;
        q->tail = p;
        p->prev = NULL;
        p->next = NULL;
    } else if(q->tail->last_exec >= (p->arrival_time - quantum) && (q->tail->arrival_time != p->arrival_time)){
        //printf("point b\n");

        //if there's only one process
        if(q->head == q->tail){
            p->prev = NULL;
            p->next = q->tail;
            q->head = p;
            q->tail->prev = p;
        } else{
            q->tail->prev->next = p;
            p->prev = q->tail->prev;
            p->next = q->tail;
            q->tail->prev = p;
        }

    }else{

        q->tail->next = p;
        p->prev = q->tail;
        p->next = NULL;
        q->tail = p;

    } 

    if(q->head->next == q->tail){
     
        if(p==q->head){
            q->tail->state = READY;
        }
        q->head->state = READY;

    }

    q->n_processes++;

    p->last_exec = -1;
    return 1;
}

int free_process_queue(struct process_queue *p_q, int mem){
    for(int i = 0;i< p_q->n_processes;i++){

        free(p_q->queue[i]->name);
        if(mem == PAGED || mem == VIRTUAL){
            free(p_q->queue[i]->address);
        }
        free(p_q->queue[i]);
    }
    
    free(p_q->queue);
    free(p_q);
    return 1;
}

int free_ready_queue(struct ready_queue *r_q){

    free(r_q);
    return 1;
}

