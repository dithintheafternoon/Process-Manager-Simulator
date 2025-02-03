//==memory consts==//
#define MEMMAX 2048
#define CONV 100
#define NFRAMES 512
#define PFSIZE 4
#define VIRTMIN 4

#define LASTEXECMAX 1000

//==memory functions==//
int free_memory(int mem);

//blocks
struct block_memory *initialise_block_mem();
        //merge holes with other holes
int merge_block_hole(struct block_list *q, struct memory_block *block);
int free_process_block_memory(struct block_list *bl, struct process *);
        //will return the first half of the split block portion
struct memory_block *split_block(struct block_list *bq, struct memory_block *block, int p_size);   
int allocate_block_memory(struct block_list *q, struct process *p);
//int free_block_memory(struct block_list *q, struct process *p);
int free_block_list(struct block_list *q);
int free_mem(int mem, void *m);

//pages
struct paged_memory *initialise_paged_mem();
int free_paged_memory();

//=============================================================================

//pages
int allocate_paged_memory(struct paged_memory *pm, struct process *p, int n_pages, struct ready_queue *rq, int simtime);
int evict(struct paged_memory *pm, struct process *p, struct ready_queue *rq, int simtime);

int allocate_paged_virtual_memory(struct paged_memory *pm, struct process *p, int n_pages, struct ready_queue *rq, int simtime);


//=====================================

int evict_finished(struct paged_memory *pm, struct process *p, int memtype);