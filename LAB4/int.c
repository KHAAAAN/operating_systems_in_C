
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

#define AX 8
#define PA 13
/****************** syscall handler in C ***************************/
int kcinth()
{
   int a,b,c,d, r;

//==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
	
   u16 segment, offset;

   segment = running->uss; offset = running->usp;

   /*get syscall parameters from ustack */
   a = get_word(segment, offset + 2*PA);
   b = get_word(segment, offset + 2*(PA +1));
   c = get_word(segment, offset + 2*(PA+2));
   d = get_word(segment, offset + 2*(PA+3));

   /*route syscall call to kernel functions by call# a */

   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kkfork();         break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;
	  case 7: r = kkgetc(); break;
	  case 8:  kkputc(b); break;

       case 99: kkexit(b);            break;
       default: printf("invalid syscall # : %d\n", a); 
   }

//==> WRITE CODE to let r be the return value to Umode
	put_word(r, segment, offset + 2*AX); //return value in uax register	
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
	return running->pid;
}

int kps()
{
	PROC *pPROC; int i;

	printf("proc[i]:       pid       status       ppid       priority       name\n\n");
			
	for(i = 0; i < NPROC; ++i){
		printf("proc[%d]:      %d        %d           %d        %d           %s\n\n", i, proc[i].pid, proc[i].status, proc[i].ppid, proc[i].priority, proc[i].name);
	}
}

int kchname(char *name)
{
    //WRITE C CODE to change running's name string;
	int i = 0;

	char buf[32];
	char *cp = buf;
	int count = 0;

	while(count < 32){
		*cp = get_byte(running->uss, name);
		if (*cp == 0){
			break;
		}

		cp++;
		name++;
		count++;
	}

	buf[31] = 0;
	strcpy(running->name, buf);
}

int kkfork()
{
  /*use you kfork() in kernel;
  return child pid or -1 to Umode!!!*/

  return kfork();
}

int ktswitch()
{
    return tswitch();
}

int kkwait(int *status)
{

  /*use YOUR kwait() in LAB3;
  return values to Umode!!!*/

	return kwait(status);
}

int kkexit(int value)
{
    /*use your kexit() in LAB3
    do NOT let P1 die*/

	if(running->pid == 1){
		printf("Cannot let P1 die.\n");
		return 0;
	}

	return kexit(value);
}
