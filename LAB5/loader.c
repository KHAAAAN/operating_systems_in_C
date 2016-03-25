#define EXT2_NAME_LEN 255
#define BLKSIZE 1024
char names[64][64];

//page 34 mtx
typedef struct ext2_group_desc
{
	u32	bg_block_bitmap;		// Bmap block number
	u32	bg_inode_bitmap;		// Imap block number
	u32	bg_inode_table;			// Inodes begin block number
	u16	bg_free_blocks_count;	// THESE are OBVIOUS
	u16	bg_free_inodes_count;	
	u16	bg_used_dirs_count;
	u16	bg_pad;
	u32	bg_reserved[3];			// ignore these
} GD;

//page 35 mtx
typedef struct ext2_inode {
	u16 i_mode;		// 16 bits = |tttt|ugs|rwx|rwx|rwx|
	u16 i_uid;		// owner uid	
	u32 i_size;     // file size in bytes
	u32 i_atime;	// time fields in seconds
	u32 i_ctime;    // since 0:00:00,1-1-1970
	u32 i_mtime;
	u32 i_dtime;
	u16 i_gid;		// group ID u16 i_links_count; // hard-link count
	u16 i_links_count;
	u32 i_blocks;	// number of 512-byte sectors
	u32 i_flags;	//IGNORE
	u32 i_reserved1; //IGNORE
	u32 i_block[15]; // See details below
	u32 i_pad[7];
} INODE;

//page 36 mtx
typedef struct ext2_dir_entry_2 {
	u32 inode;
	u16 rec_len;
	u8 name_len;
	u8 file_type;
	char name[EXT2_NAME_LEN];	// name: 1 -255 chars, no NULL byte
} DIR;

//To load a disk block number, blk, into char buf[1024]: call get_block():

/*NOTE: block numbers in INODEs are u32 values. You must Call the get_block() 
      function with blk of the right size.*/

typedef struct header{
     u32 ID_space;        // 0x04100301:combined I&D or 0x04200301:separate I&D
     u32 magic_number;     // 0x00000020
     u32 tsize;            // code section size in bytes
     u32 dsize;            // initialized data section size in bytes
     u32 bsize;            // bss section size in bytes
     u32 zero;             // 0
     u32 total_size;       // total memory size, including heap
     u32 symbolTable_size; // only if symbol table is present
} HEADER;
 
u16 get_block(u16 blk, char *buf)  // load disk block blk to char buf[1024]
{
    // Convert blk into (C,H,S) format by Mailman to suit disk geometry
    //      CYL         HEAD            SECTOR         
    diskr( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
}

//from my 360 code
u16 search(INODE *ip, char *name){
	u16 i;
	char tempName[256];
	char buf[1024];
	
	char *cp;
	DIR *dp;

	i = 0;
	while(i < 12){
		//first check if i_block is even initiated.
		if(ip->i_block[i] == 0){
			printf("i = %d\n", i);
			return 0;
		}

		//we get whatever i_block we are on
		get_block((u16) ip->i_block[i], buf);
		
		dp = (DIR *) buf;
		cp = (char *) dp;
		
		do{
			strcpy(tempName, ""); //reset string	
			strncpy(tempName, dp->name, dp->name_len);	
			tempName[dp->name_len] = '\0';
			printf("tempName = %s\n", tempName);
	
			//printf("tempName = %s\n", tempName);
			if(strcmp(tempName, name) == 0){
				return (u16) dp->inode;//return inode number
			}

			cp += dp->rec_len;
			dp = (DIR *) cp;

		}while(cp < buf + BLKSIZE);
		
		i++;
	} 
	//default return 0, hasn't found name
	return 0;
}

//from 360
/*************
 *tokenize breaks up pathname
  and puts each string into names[64][64] 
 *************/
u16 tokenize(char *pathname){
	int i;
	char *s;
	char tempBuf[256];
	
	//first reset all strings.
	for(i = 0; i < 64; ++i){
		strcpy(names[i], "");
	}


	if(strcmp(pathname, "/") == 0){ //this say
		return;
	}
	
	if(strcmp(pathname, "") == 0){
		pathname = ".";
	}

	strcpy(tempBuf, pathname);

	i = 0;
	
	s = strtok(tempBuf, "/");
	strcpy(names[i++], s);

	while(s = strtok(0, "/")){
		strcpy(names[i++], s);	
	}
	
	return (u16) i;
}

char bufOne[BLKSIZE];
char bufHeader[BLKSIZE];	
char bufInd[BLKSIZE]; //indirects blocks buffer

/*1. find the inode of filename; return 0 if fails;
2. read file header to get tsize, dsize and bsize;
3. load [code|data] sections of filename to memory segment;
4. clear bss section of loaded image to 0;
5. return 1 for success;*/
int load(char *filename, u16 segment)
{
	char   *cp;

	u16 i, n, size, word, bsize;
	u16 found, blk, inode_table;

	u32    *ind;
	
	INODE  *ip;
	GD     *gp;
	HEADER *hp;

	
	/*******1. find the inode of filename; return 0 if fails******/

	//first we tokenize the filename
	n = tokenize(filename);

	//now we have to get the group descriptor
	get_block(2, bufOne);
	gp = (GD *) bufOne;

	//get the inode_table start point.
	inode_table = (u16)gp->bg_inode_table;	

	//get the first inode block from inode_table
	get_block(inode_table, bufOne);

	//point our ip to the first inode
	ip = (INODE *)bufOne + 1;
		
	//make sure the inode for the filename path exists
	for(i = 0; i < n; ++i){

		//search the inode's meta data for if names[i] exists
		found = search(ip, names[i]);
		
		//return -1 if it doesn't exist
		if(found == 0){
			printf("load: Error, %s not found\n", names[i]);
			return -1;
		}

		//this reads that inode block into bufOne
		get_block(inode_table + ( (found - 1) / 8), bufOne );

		//mail man's algorithm to get specific inode
		ip = (INODE *) bufOne + ( (found - 1)  % 8);
	}	


	/*****2. read file header to get tsize, dsize and bsize*********/

	// To get header we want to use bufHeader so we don't mess up the data ip is pointing to
	// header information is located in i_block[0]
	get_block((u16)ip->i_block[0], bufHeader);

	hp = (HEADER *) bufHeader;

	/*****3. load [code|data] sections of filename to memory segment*****/
	
	//load the segment, aka set the es register to our segment 
	setes(segment);

	//first direct blocks
	//if the i block is NULL, you are done
	i = 0;
	while(i < 12 && ip->i_block[i] != 0){
		get_block((u16)ip->i_block[i], 0);	
		inces(); //increment ES by 1K
		i++;
	}

	//now take care of indirect blocks
	if ( (u16)ip->i_block[12] != 0 ){
		printf("*********INDIRECT***************\n\n");
		get_block((u16)ip->i_block[12], bufInd); //first get the indirect block into our buffer

		ind = (u32 *)bufInd; // have a pointer to the address of our indirect block

		while((u16) *ind != 0){ //when dereferenced our indirect block will be a direct block
			get_block((u16) *ind, 0);  //read that block into the address, 0
			inces(); //increment ES by 1k aka 0x40
			ind++; //onto the next direct block of our indirect block
		}
	}

	size = (u16)hp->tsize + (u16)hp->dsize; //code section size + data section
	bsize = (u16) hp->bsize;

	printf("tsize = %d, dsize = %d, bsize = %d\n", (u16)hp->tsize, (u16)hp->dsize, (u16)hp->bsize);

	//we have to move shift 32 bytes because of header and then shift everything down
	i = 0;	
	while(i <= size){
		word = get_word(segment + 2, i); //get word from segment + 32 bit with offset of 16 bit * i, put into word
		put_word(word, segment, i); //from word put into segment with offset of 16bit * i

		i += 2; //because that's a u16 we have to do += 2 for 32bit (4 byte)
	}


	/*****4. clear bss section of loaded image to 0******/

	//this might be wrong, haven't tested yet
	for(i = size; i < bsize + size; ++i){
		put_byte(0, segment, i);
	}

	//setes sets es register back to 0x1000 which is where MTX started at proc0	
	setes(0x1000); 


	/*****5. return 1 for success*******/
	return 1; 
}

