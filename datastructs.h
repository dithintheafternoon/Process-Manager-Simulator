//Header file for all strucutres for project

//================================================================================================
//DEFINES
#define SIZE 10
#define INFINITE 0
#define FIRSTFIT 1
#define PAGED 2
#define VIRTUAL 3

#define READY 0
#define RUNNING 1
#define FINISHED 2

#define NOTALLOCATED 0
#define NOADDRESS 1

#define NOTFINISHED -1
#define NFRAMES 512

#define NOPROCESS -1
#define NOFRAME -1
#define NOTFREE -1


//===============================================================================================
struct process{
    //process info. from file
    char *name;
    int size;
    int service_time;
    int arrival_time;

    int pq_i;
    //its index in the process queue
    int v_mem;
    
    //memory info
    int allocated;
    void *address;
        //will either be a pointer to a frame LIST or a pointer to a memory block

    //CPU info and stats
    int state;
    int time_left;
    int end_time;
    int last_exec;
    //will document the simtime that it last STARTED RUNNING

    //for ready queue
    //making it double linked so we can reuse the linkedlist functions from memory
    struct process *next;
    struct process *prev;
};

//contains info abt overall simulation/processes running
struct cpu_info{
    //saves where we are in the array that contains all processes
    int process_queue_i;
    //how long the simulation has been running for so far
    int simulation_time;
};

//will take processes from the process queue and add them here when they arrive
//linkedlist that can rearrange the processes.
struct ready_queue{
    struct process *head;
    struct process *tail;
    int n_processes;
    int empty;
};

//read all processes from the CSV into this datastructure
struct process_queue{
    //an array of all processes that will arrive
    //also stores information about the memory and quantum

    //an array of pointers to process structures
    struct process **queue;
    int q_size;

    //stats
    int n_processes;
    int quantum;
    char *mem_type;

};

//=====================================================================================================================
//MEMORY

    //blocks of memory to put into the memory
struct memory_block{
    struct memory_block *prev;
    struct memory_block *next;
        //info
    int id;
    int start;
    int end;
        //stats
    int size;
    int free;

    struct process *process_name;
};

struct block_list{
    struct memory_block *head;
    struct memory_block *tail;
    int remaining_memory;
    int n_blocks;
};



//===================================================

typedef struct frame * frame_pointer;
typedef frame_pointer frame_pointer_array[NFRAMES];

    //two different memory structs, one for paged memory, one for non-paged memory
//linkedlist to keep track of all available/occupied memory for non-paged
struct block_memory{
    //using a linked list for non-paged memory to merge sections
    struct block_list *block_list;
    //in KB
    int largest_free_block;
};


struct paged_memory{
    //a pointer to an array of pointers to frames
    int frames[NFRAMES];
    int remaining;
};

//====================================================================================================================

//reading
int read_file_new(char *filename, struct process_queue *pq);
struct process_queue *take_commands(int argc, char *argv[]);

//linkedlist function prototypes
    //processes
int move_to_tail(struct ready_queue *q);
int remove_from_ready_queue(struct ready_queue *q);
int add_to_rq(struct ready_queue *q, struct process *p, int quantum);

int remove_from_ready_queue(struct ready_queue *q);
int free_process_queue(struct process_queue *p_q, int mem);
int free_ready_queue(struct ready_queue *r_q);


//=====================================================================================================================