void put_proc(PROC **list, PROC *p){ // e.g. put_proc(&freeList, p);
	p->next = *list;
	*list = p;	
}

PROC *get_proc(PROC **list){ // e.g. get_proc(&freeList);
	PROC *p;

	if(*list == 0){ //null address
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

int do_tswitch() { 
	tswitch(); 
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

		default: printf("ERROR: invalid key\n"); break;

	 }
   }
}


PROC *kfork(){
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
	
	nproc++; //number of procs has increased **

	for (i = 1; i < 10; ++i){ //SAVED CPU registers
		p->kstack[SSIZE - i] = 0; //all 0's
	}

	p->kstack[SSIZE - 1] = (int) body; //resume point = address of body()
	p->ksp = &p->kstack[SSIZE - 9]; //prock saved sp
	enqueue(&readyQueue, p); // enter p into readyQueue by priority

	return p; //return child PROC pointer
	
}

int stoi(char *s){
	char *temp = s;
	int multiplier = 1;
	int cur = 0, number = 0;

	if((int)*s == 48){ //0123 is NaN
		return 0;
	}
	
	while(*temp){
		temp++;	
	};	//get to '\0'
	
	temp--; //get back to last character

	while(temp != s){ //first get length
		cur = (int) *temp - 48;
		printf("cur = %d\n", cur);
		if(cur > 9){ //not a number
			return 0;
		}
		temp--; //go back one byte 
		
		number = multiplier * cur + number;
		multiplier = multiplier * 10; //move to next tens
	}

	if(temp == s){
		cur = (int) *temp - 48;
		printf("cur = %d\n", cur);
		if(cur > 9){ //not a number
			return 0;
		}
		temp--; //go back one byte 
		
		number = multiplier * cur + number;
		multiplier = multiplier * 10; //move to next tens
	}
	printf("number = %d\n", number);
	return number;
}

int do_exit(){
	int exitValue;
	char buf[32];

	printf("please enter an exitValue: ");
	gets(buf);

	exitValue = stoi(buf);

	printf("\nExiting with %d...\n", exitValue);
	kexit(exitValue);	
	
}

int do_sleep(){
	int sleepValue;
	char buf[32];

	printf("please enter an sleepValue: ");
	gets(buf);
	sleepValue = stoi(buf);

	printf("\nSleeping with %d\n", sleepValue);
	ksleep(sleepValue);	
}

int do_wakeup(){	
	int wakeupValue;
	char buf[32];

	printf("please enter an wakeupValue: ");
	gets(buf);

	wakeupValue = stoi(buf);

	printf("\nWaking up with %d\n", wakeupValue);
	kwakeup(wakeupValue);	
}

int do_wait(){
	int pid, status;
	pid = kwait(&status);
	printf("pid = %d, status = %d\n", pid, status);
}
