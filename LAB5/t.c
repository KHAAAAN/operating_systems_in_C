#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "queue.c"
#include "io.c"
#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
#include "loader.c"
#include "fork_exec.c"

int color;

int init()
{
    PROC *p; int i;
    color = 0x0C;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{
    if (running->status == READY)
        enqueue(&readyQueue, running);
     running = dequeue(&readyQueue);
     color = running->pid + 0x0A;
}

int int80h(); 
int set_vector(u16 vector, u16 handler)
{
     // put_word(word, segment, offset)
     put_word(handler, 0, vector<<2);
     put_word(0x1000,  0,(vector<<2) + 2);
}
            
main()
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vector(80, int80h);

    kfork("/bin/u1");     // P0 kfork() P1, P1 will have a Umode image /bin/u1 loaded in the segment 0x2000.

    while(1){
      printf("P0 running\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();         // P0 switch to run P1
   }
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
		
		case 's' : do_tswitch(); break;
		
		case 'q':
		do_exit();
		break;


		case 'z':
		do_sleep();
		break;

		case 'a':
		do_wakeup();
		break;

		case 'w':
		do_wait();
		break;
   				
		case 'p':	  
		printf("*****readyQueue: ");
		printQueue(readyQueue);
		printf("\n");
		
		printf("*****freeList: ");
		printQueue(freeList);
		printf("\n");

		printf("*****sleepList: ");
		printSleepList(sleepList);
		printf("\n");

		break;
	
		case 'u': 
		goUmode();
		break;

		case 'o':
		kps();
		break;

		default: printf("ERROR: invalid key\n"); break;

	 }
   }
}
