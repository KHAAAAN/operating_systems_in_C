typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

#define BROKEN_PIPE -1
#define READ_PIPE  4
#define WRITE_PIPE 5

#define NOFT      20
#define NFD       10
#define PSIZE 10
#define NPIPE 10

typedef struct Oft{
  int   mode;
  int   refCount;
  struct pipe *pipe_ptr;
} OFT;

typedef struct pipe{
  char  buf[PSIZE];
  int   head, tail, data, room;
  int   nreader, nwriter;
  int   busy;
}PIPE;

typedef struct proc{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];           // name string of PROC

	OFT *fd[NFD]; // ADD: open file descriptors
    int    kstack[SSIZE];      // per proc stack area

}PROC;

PROC *kfork(char *filename);

void put_proc(PROC **list, PROC *p);

void goUmode();

//pipe.c
int showPipe(PIPE *p);
int pfd();
int read_pipe(int pd, char *buf, int n);
int write_pipe(int pd, char *buf, int n);
int kpipe(int pd[2]);
int close_pipe(int fd);
