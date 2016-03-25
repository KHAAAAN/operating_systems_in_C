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

	return p->pid;
}

int do_tswitch() { 
	tswitch(); 
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
                    
/*kernel.c file:

             YOUR kernel.c file as in LAB3

	     MODIFY kfork() to*/
 
PROC *kfork(char *filename){
	int i;
	PROC *p;
	u16 segment;
	
	p = get_proc(&freeList);
	if(p == 0){
		printf("no more PROC, kfork() failed\n");
		return 0; //FAIL
	}

	p->status = READY;
	p->priority = 1;
	p->ppid = running->pid;
	p->parent = running;

	segment = (p->pid + 1) * 0x1000;
	//printf("segment = %d\n", segment);

	nproc++; //number of procs has increased **

	for (i = 1; i < 10; ++i){ //SAVED CPU registers
		p->kstack[SSIZE - i] = 0; //all 0's
	}
	
	/*p->kstack[SSIZE - 1] = (int) body; //resume point = address of body()
	p->ksp = &p->kstack[SSIZE - 9]; //prock saved sp
	enqueue(&readyQueue, p); // enter p into readyQueue by priority*/


	//instead of going to address of body, we set that resume point to Umode
	p->kstack[SSIZE - 1] = (int)goUmode; //resume point = address of goUmode
	//p->kstack[SSIZE - 1] = (int)body; //resume point = address of body
	p->ksp = &(p->kstack[SSIZE - 9]); //prock saved sp
	
	/*if(filename == 0){
		load("/bin/u1", segment);
		//printf("if: loaded /bin/u1 successfully\n");
	}
	else{
		load(filename, segment);
		//printf("else: loaded %s successfully\n", filename);
	}*/

	if(filename){
		load(filename, segment);
		printf("loaded %s successfully\n", filename);

		for(i = 1; i < 13; i++) {
			put_word(0, segment, -i*2);
		}

		//for uflag uCS uES uDS
		put_word(0x0200, segment, -2);	
		put_word(segment, segment, -4);
		put_word(segment, segment, -22);
		put_word(segment, segment, -24);

		p->uss = segment;
		p->usp = -24;
	}

			
	enqueue(&readyQueue, p); // enter p into readyQueue by priority*/
	
	//printf("PROC %d kforked a child with pid %d\n", running->pid, p->pid);
	
	return p; //return pointer to new PROC
	//}
	
}

char kkgetc(){
	char c;
	printf("kkgetc(): "); c = getc();
}

void kkputc(char c){
	printf("kkputc c: "); 
	putc(c);	
	printf("\n\n");
}


