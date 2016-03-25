show_pipe(PIPE *p)
{
	int i, j;
	printf("------------ PIPE CONTENTS ------------\n");     
	// print pipe information
	printf("nreader=%d  nwriter=%d  data=%d  room=%d\n",
		p->nreader, p->nwriter, p->data, p->room);

	printf("contents=");

	for(i = 0, j = p->tail; i < p->data; i++, j++)
	{
        if(j == PSIZE) { 
			j = 0;
		}
        printf("%c", p->buf[j]);
	}

	printf("\n----------------------------------------\n");

	return 0;
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
	// print running process' opened file descriptors
	int i = 0;
	OFT *op;
	for(i = 0; i < NFD; i++){
		op = running->fd[i];
		if(op->refCount > 0){
			printf("fd = %d,  refCount = %d,  mode = %s\n",
				   	i, op->refCount, MODE[op->mode - 4]);
		}
	}

	return 0;
}
//============================================================


int read_pipe(int fd, char *buf, int n)
{
	int r = 0;	
	PIPE *p;

	if (n <= 0){
		return 0;
	}
	//validate fd; from fd, get OFT and pipe pointer p
	if (running->fd[fd] == 0){
		return -1;
	}
	if(running->fd[fd]->mode != READ_PIPE){
		  printf("Error: fd mode is not read\n");
		  return -1;
	}

	p = running->fd[fd]->pipe_ptr;

	printf("p = %d, p->buf", p, p->buf[0]);
	printf("Pipe before read\n");
	show_pipe(p);

	while(n){
		while(p->data){	
			put_word(p->buf[ p->head ], running->uss, buf+r);
			p->tail++;
			p->tail = p->tail % PSIZE;
			n--; r++; p->data--; p->room++;

			if (n==0){
			   	break;
			}
		}

		//pipe has data
		if (r || n == 0){		
			printf("Pipe after read\n");
			show_pipe(p);
			//kwakeup(&p->room);
			return r;
		}
		
		// pipe has no data
		if (p->nwriter){ // if pipe still has writer	
			printf("Pipe before sleep\n");
			show_pipe(p);

			kwakeup(&p->room); // wakeup ALL writers, if any.
			sleep(&p->data); // sleep for data
		}

		// pipe has no writer and no data
		return 0;	
	}
	
}

int write_pipe(int fd, char *buf, int n)
{
	// your code for write_pipe()
	int r = 0;
	PIPE *p;
	char c;

	if(n <= 0){
		return 0;
	}

	//validate fd; from fd, get OFT and pipe pointer p;
	if (running->fd[fd] == 0){
		return -1;
	}
	if(running->fd[fd]->mode != WRITE_PIPE){
		  printf("Error: fd mode is not write\n");
		  return -1;
	}

	p = running->fd[fd]->pipe_ptr;
	printf("Before WRITE\n");	
	show_pipe(p);

	while (n) {
		if (!p->nreader){ // no more readers
			//kexit(BROKEN_PIPE);		
			printf("BROKEN_PIPE\n");
			return BROKEN_PIPE;
		}
		while(p->room && n){
			//write a byte from buf to pipe;
			c = get_word(running->uss, buf+r);
			p->buf[p->head] = c;
			p->head++;
			p->head = p->head % PSIZE;
			r++; p->data++; p->room--; n--;
		}
		kwakeup(&p->data); // wakeup ALL readers, if any.

		if (n == 0){
			printf("Successful WRITE\n");
			show_pipe(p);
			return r;	//finished writing n bytes
		}
		//still has data to write but pipe has no room
		ksleep(&p->room); // sleep for room
	}

	//show_pipe(p);
	return 0;
}

int kpipe(int pd[2])
{
	//1.
	// create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
	int fd1, fd2;	

	int iPIPE;
	int iOFT, jOFT;
	int iFD, jFD;

	//2.
	//Allocate a PIPE object. Initialize the PIPE object
	//with head=tail=0; data=0; room=PSIZE; nreaders=nwriters=1;
	iPIPE=0;	
	while(pipe[iPIPE].busy != 0 && iPIPE < NPIPE){
		iPIPE++;										
	}

	//no pipe available	
	if(iPIPE == NPIPE){
		return -1;
	}

	pipe[iPIPE].head = 0;
	pipe[iPIPE].tail = 0;
	pipe[iPIPE].data = 0;
	pipe[iPIPE].room = PSIZE;
	pipe[iPIPE].nreader = 1;
	pipe[iPIPE].nwriter = 1;	
	pipe[iPIPE].busy = 1;	

	//3.
	//Allocate 2 OFTs. Initialize the OFTs as
	//readOFT.mode = READ_PIPE; writeOFT.mode = WRITE_PIPE;
	//both OFT's refcount=1 and pipe_ptr points to the same PIPE object.
	
	iOFT = 0;
	while(iOFT < NOFT && oft[iOFT].refCount != 0){		
		iOFT++;	
	}

	//no oft available
	if(iOFT == NOFT){
		return -1;
	}

	jOFT = iOFT + 1; //next available oft
	while(jOFT < NOFT && oft[jOFT].refCount != 0){
		jOFT++;	
	}

	//no second oft available
	if(jOFT == NOFT){
		return -1;
	}

	//first OFT is read, second is write
	oft[iOFT].mode = READ_PIPE;	
	oft[iOFT].refCount = 1;
	oft[iOFT].pipe_ptr = &(pipe[iPIPE]);
	
	oft[jOFT].mode = WRITE_PIPE;	
	oft[jOFT].refCount = 1;
	oft[jOFT].pipe_ptr = &(pipe[iPIPE]);	

	//4.
	//Allocate 2 free entries in the PROC.fd[] array, e.g. fd[i] and fd[j]
	//let fd[i] point to readOFT and fd[j] point to writeOFT.

	iFD = 0;	
	while(iFD < NFD && running->fd[iFD] != 0){
		iFD++;	
	}

	//no FD available
	if(iFD == NFD){
		return -1;
	}

	jFD = iFD + 1;
	while(jFD < NFD && running->fd[jFD] != 0){
		jFD++;	
	}

	//no second FD available
	if(jFD == NFD){
		return -1;
	}

	running->fd[iFD] = &(oft[iOFT]);
	running->fd[jFD] = &(oft[jOFT]);

	//5.
	//write index i to pd[0] and index j to pd[1]; both are in Uspace.	
	put_word(iOFT, running->uss, &pd[0]);
	put_word(jOFT, running->uss, &pd[1]);

	//6.
	//return 0 for OK
	return 0;
}

int close_pipe(int fd)
{
	OFT *op; PIPE *pp;

	printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

	op = running->fd[fd];
	running->fd[fd] = 0;                 // clear fd[fd] entry 

	if (op->mode == READ_PIPE){
		pp = op->pipe_ptr;
		pp->nreader--;                   // dec n reader by 1

		if (--op->refCount == 0){        // last reader
			if (pp->nwriter <= 0){         // no more writers
				pp->busy = 0;             // free the pipe   
					return;
			}
		}
		kwakeup(&pp->room);               // wakeup any WRITER on pipe 
		return;
	}

	// YOUR CODE for the WRITE_PIPE case:
	if (op->mode == WRITE_PIPE){
		pp = op->pipe_ptr;	
		pp->nwriter--;                   // dec n writer by 1

		if (--op->refCount == 0){        // last writer
			if (pp->nreader <= 0){         // no more readers
				pp->busy = 0;             // free the pipe   
				return;
			}
		}
		kwakeup(&pp->room);               // wakeup any READER on pipe 
		return;
	}

	return -1; //error case

}
