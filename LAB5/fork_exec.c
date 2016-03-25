int copyImage(u16 pseg, u16 cseg, u16 size){
	u16 i;
	for (i = 0; i < size; i++){
		put_word(get_word(pseg, 2*i), cseg, 2*i); //get word from pseg with offset 2bytes * i
	}
}

int fork(){
	int pid; 
	u16 segment;

	PROC *p = kfork(0);		// kfork() a child, do not load image file

	// kfork failed
	if (p == 0){
		return -1;
	}

	segment = (p->pid + 1) * 0x1000; // child segment
	copyImage(running->uss, segment, 32*1024);	// copy 32k Words

	p->uss = segment; //child's own segment
	p->usp = running->usp;	//same as parent's usp

	//*** change uDS, uES, uCS, AX in child's ustack ****/
	
	put_word(segment, segment, p->usp);		// uDS=segment
	put_word(segment, segment, p->usp + 2);		// uES=segment
	put_word(0,		segment, p->usp + 2 * 8);		// uax = 0
	put_word(segment, segment, p->usp + 2*10);		// uCS=segment
	return p->pid;
}

// y points at filename in Umode space
int kexec(char *y){
	int i, length = 0;
	char filename[64], *cp = filename;
	u16 segment = running->uss;	// same segment


	/* get filename from U space with a length limit of 64 */
	while( (*cp++ = get_byte(running->uss, y++)) && length++ < 64);

	printf("kexec: filename = %s\n", filename);

	if(!load(filename, segment)){ // load filename to segment
		return -1; //if load failed, return -1 to Umode
	}

	/* re-initialize process ustack for it return to VA=0 */

	for(i=1; i <= 12; i++){
		put_word(0, segment, -2*i);
	}
	running->usp = -24;		// new usp = -24
	/* -1 	-2 	-3  -4 -5 -6 -7 -8 -9 -10 -11 -12 ustack layout */
	/* flag uCS uPC ax bx cx dx bp si  di uES uDS */

	put_word(segment, segment, -2*12);	// saved uDS=segment
	put_word(segment, segment, -2*11);	// saved uES=segment
	put_word(segment, segment, -2*2);	// uCS=segment; uPC=0
	put_word(0x0200, segment, -2*1);	// Umode flag = 0x200

}
