//enter p into queue by priority
void enqueue(PROC **queue, PROC *p){
	PROC *prev, *cur;

	if(*queue == 0){ // empty case
		*queue = p;
		(*queue)->next = 0;
	}

	//root case
	else if(p->priority > (*queue)->priority){ // case where p's priority is greater than or equal to root
		p->next = *queue;
		*queue = p;	
	}

	//middle, last case	
	else{
		cur = *queue;

		while(cur  && p->priority <= cur->priority){
			prev = cur;
			cur = cur->next;
		}

		//last case
		if (cur == 0){
			prev->next = p;
			p->next = 0;
		}	
		//middle case
		else{
			p->next = cur;
			prev->next = p;
		}
	}

	return;
}

PROC *dequeue(PROC **queue){
	PROC *p;
	p = *queue; //pop first one

	if(p != 0){
		*queue = (*queue)->next;
	}


	return p;
}

void deqSleepList(PROC **queue, PROC *p){
	PROC *prev, *cur;

	if(*queue){

		//root case
		if(p == *queue){ //if p is pointing same address of PROC as queue, get rid of that
			*queue = (*queue)->next;
			return;
		}
	
		cur = *queue;

		while(cur  && p != cur){
			prev = cur;
			cur = cur->next;
		}

		prev->next = cur->next;
			
	}

	return;	
}

void printSleepList(PROC *queue){
	if(queue == 0){
		printf("NULL\n");
		return;
	}

	printf("[pid = %d, priority = %d, event = %d] -> ", queue->pid, queue->priority, queue->event);

	printQueue(queue->next);
}

void printQueue(PROC *queue){
	if(queue == 0){
		printf("NULL\n");
		return;
	}

	printf("[pid = %d, priority = %d] -> ", queue->pid, queue->priority);

	printQueue(queue->next);
}

