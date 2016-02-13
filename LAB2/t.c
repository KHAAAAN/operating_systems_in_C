#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 

#define FREE 0
#define READY 1
#define SLEEP 2
#define BLOCK 3
#define ZOMBIE 4


typedef struct proc{
    struct proc *next;
    int    *ksp;               // saved ksp when not running
                           //   0    1     2     3     4
	int    status;        // FREE|READY|SLEEP|BLOCK|ZOMBIE
	int    priority;      // priority
	int    pid;           // process pid; Add pid for identify the proc
	int    ppid;          // parent pid 
	struct proc *parent;
    
	int    kstack[SSIZE];      // proc stack area

}PROC;

PROC *kfork();

int  procSize = sizeof(PROC);

PROC proc[NPROC], *running, *freeList, *readyQueue;    // define NPROC procs
extern int color;

#include "io.c"
#include "queue.c"

void put_proc(PROC **list, PROC *p){ // e.g. put_proc(&freeList, p);
	p->next = *list;
	*list = p;	
}

PROC *get_proc(PROC **list){ // e.g. get_proc(&freeList);
	PROC *p;

	if(*list == 0){ //null address
		printf("get_proc, null\n");
		return 0;
	}

	p = *list;
    *list = (*list)->next; //update root
	
	return p;	

}


int do_kfork(){
	PROC *p = kfork();
	if (p == 0){
		printf("kfork failed\n");
		return -1;
	}

	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int body()
{ 
   char c;
   printf("proc %d resumes to body()\n", running->pid);
   while(1){
     color = running->pid + 7;
     printf("proc %d running : enter a key : ", running->pid);
     c = getc(); 
     switch(c){
		case 'f' : do_kfork(); break;
		
		case 's' : tswitch(); break;
		
		case 'q':
		running->status = ZOMBIE; 
		running->priority = 0; 
		put_proc(&freeList, running); 
		tswitch();
		break;
	 
		case 'p':	  
		printf("*****readyQueue: ");
		printQueue(readyQueue);
		printf("\n");
		
		printf("*****freeList: ");
		printQueue(freeList);
		printf("\n");

		break;

		default: printf("ERROR: invalid key\n"); break;

	 }
   }
}

int init()
{
   PROC *p;
   int i, j;

   /* initialize all proc's */
   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->pid = i;                        // pid = 0,1,2,..NPROC-1
       p->next = &proc[i+1];              // point to next proc
	   p->priority = 0;
	   p->status = FREE;
       if (i){                            // not for P0
          p->kstack[SSIZE-1] = (int)body; // entry address of body()
          for (j=2; j<10; j++)            // kstack[ ] high end entries = 0
               p->kstack[SSIZE-j] = 0;
          p->ksp = &(p->kstack[SSIZE-9]);
       }
   }
   proc[NPROC-1].next = 0;         // all procs form a circular link list
	freeList = &proc[0];
	readyQueue = 0;
	
	
	p = get_proc(&freeList);
	p->ppid = 0;
	p->status = READY;
   running = p;                    // P0 is running 

   printQueue(freeList);
   printf("\n");
   printf("init complete\n");
 }
 
int scheduler()
{
	if (running->status == READY){
		enqueue(&readyQueue, running);
	}

	running = dequeue(&readyQueue);	
}

main()
{
  printf("MTX starts in main()\n");
  init();
  kfork();
  tswitch();
  body();
  while(1){
    printf("proc 0  running : enter a key : \n");
    getc();
	if (readyQueue){
    	tswitch();
	}
	else{
		printf("proc 0 running: enter a key: \n");
	}
  }
}

PROC *kfork(){
	//PROC *p = get_proc(&freeList); //to get a FREE PROC from freeList; if none, return 0 for FAIL;
	int i;
	PROC *p;
	
	p = get_proc(&freeList);
	if(p == 0){
		printf("no more PROC, kfork() failed\n");
		return 0; //FAIL
	}


	p->status = READY;
	p->priority = 1;
	p->ppid = running->pid;
	p->parent = running;

	for (i = 1; i < 10; ++i){ //SAVED CPU registers
		p->kstack[SSIZE - i] = 0; //all 0's
	}

	p->kstack[SSIZE - 1] = (int) body; //resume point = address of body()
	p->ksp = &p->kstack[SSIZE - 9]; //prock saved sp
	enqueue(&readyQueue, p); // enter p into readyQueue by priority

	return p; //return child PROC pointer
	
}

