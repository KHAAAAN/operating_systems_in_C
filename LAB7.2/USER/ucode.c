// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kfork", "switch",
   	"wait", "exit", "kgetc", "kputc", "fork", "exec", 
	 "pipe", "pfd", "close", "read", "write", "sleep", 0};

#define LEN 64

int show_menu()
{
   printf("***************** Menu *******************\n");
   printf("*  ps    chname  kfork  switch  wait   exit  kgetc  kputc  fork  exec  *\n");
   printf("*  pipe  pfd     close  read    write  sleep  *\n");
   printf("******************************************\n");
}

int sleep(){
  char length[16];
  int t;
  printf("Enter time you want PROC to sleep for: ");
  gets(length);
  t = atoi(length);

  printf("PROC going to kernel to sleep for %d seconds\n", t);
  syscall(16, t, 0);
  printf("PROC has just waken up\n");
}

int find_cmd(char *name)
{
  // return command index
  int i = 0;
  char *c = cmd[0];

  while(cmd[i] != 0){
		if(strcmp(cmd[i], name) == 0){
			return i;
		}
		i++;
  } 
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    printf("input new name : ");
    gets(s);
    
	return syscall(2, s, 0);
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(3, 0, 0);
  printf("proc %d kforked a child w/ address %d\n", pid, child);
}    

int kswitch()
{
    return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
}

int geti()
{
  // return an input integer
}

int exit()
{
   int exitValue;
   printf("enter an exitValue: ");
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

char kgetc(){
	return syscall(7, 0, 0);	
}

int putc(char c){	
	return syscall(8, c, 0);
}

int kputc(){
	char c;
	//printf("Enter character: "); 
	//c = getc();
	return syscall(8, c, 0);
}

int fork(){
	return syscall(9, 0, 0, 0); 
}

int exec(char *s) {
	return syscall(10, s, 0, 0);
}

int ufork(){
	int child = fork();
	(child) ? printf("parent ") : printf("child ");
	printf("%d return from fork, child_pid=%d\n", getpid(), child);
}

// user exec command
int uexec(){
	int r;
	char filename[64];
	printf("enter exec filename : ");
	gets(filename);
	r = exec(filename);

	//if not forked properly
	printf("exec failed\n");
}

int pd[2];

int pipe()
{
    printf("pipe syscall\n");
    syscall(11, pd, 0);
    printf("PROC %d created a pipe with fd = %d %d\n",
		   	getpid(), pd[0], pd[1]);
}

int pfd()
{
    syscall(12,0,0,0);
}

int close_pipe()
{
    int fd; 
    char s[32];

    printf("Enter fd to close : ");
    gets(s);

    fd = atoi(s);
    syscall(13, fd);
}


int read_pipe()
{
    char buf[1024], fdBuf[16];

	int n;
	int nbytes;
    int fd;

    printf("Enter fd to read from: ");
    fd = atoi(gets(fdBuf));

	printf("Enter nbytes to read: ");
    nbytes = atoi(gets(fdBuf));

    printf("fd=%d nbytes=%d\n", fd, nbytes);
    n = syscall(14, fd, buf, nbytes);

    if(n >= 0) {
        printf("PROC %d back to Umode, read %d bytes from pipe: ", getpid(), n);
        buf[n] = 0;
        printf("%s\n", buf);
    }
    else { 
		printf("READ_PIPE failed\n"); 
	}
}

int write_pipe()
{
    char fdBuf[16], buf[1024];
	
	int n;
	int nbytes;
    int fd;

    printf("Enter fd to write to: ");
    fd = atoi(gets(fdBuf));

    printf("Enter string: ");
    gets(buf);
    nbytes = strlen(buf);

    printf("fd=%d nbytes=%d\n", fd, nbytes);

    n = syscall(15, fd, buf, nbytes);
    
    if (n >= 0 ) {
        printf("\nPROC %d back to Umode, wrote %d bytes to pipe\n", getpid(), n);
    }
    else { 
		printf("WRITE_PIPE failed\n"); 
	}
}

