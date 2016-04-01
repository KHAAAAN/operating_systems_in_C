#include "uio.c"
#include "ucode.c"

int color;

main()
{ 
  char name[64]; int pid, cmd;
	char c;

  while(1){
    pid = getpid();
    color = 0x0C;
       
    printf("----------------------------------------------\n");
    printf("USER: I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
			case 0 : getpid();   break;
			case 1 : ps();       break;
			case 2 : chname();   break;
			case 3 : kfork();    break;
			case 4 : kswitch();  break;
			case 5 : wait();     break;
			case 6 : exit();     break;
			case 7: c =  kgetc(); putc(c); putc('\n'); break;
			case 8: kputc(); break;
			case 9: ufork();	break;
			case 10: uexec();	break;
			case 11 : pipe();     break;
			case 12: pfd();      break;
			case 13: close_pipe(); break;
			case 14: read_pipe(); break;
			case 15: write_pipe(); break;
			case 16: sleep(); break;

			default: invalid(name); break;
    }
  }
}

