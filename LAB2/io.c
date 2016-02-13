#define MAXLEN 128
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

u16 BASE;
char *table = "0123456789ABCDEF";

int rpu(u16 x) 
{
	char c;
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putc(c);
	}
}

int printu(u16 x)
{
	BASE = 10;
	if (x == 0)
		putc('0');
	else
		rpu(x);
	putc(' ');
}

int printd(u16 x)
{
	BASE = 10;
	if(x == 0)
		putc('0');
	else
	{
		if (x < 0)
		{
			putc('-');
			x = -x;
		}

		rpu(x);
	}
	putc(' ');
}

int printo(u16 x)
{
	BASE = 8;
	if(x == 0)
		putc('0');
	else
	{
		putc('0');
		rpu(x);
	}
	putc(' ');
}

int printx(u16 x)
{
	BASE = 16;
	if(x == 0)
		putc('0');
	else
	{
		putc('0');
		putc('x');
		rpu(x);
	}
	putc(' ');
}

int prints(char *s)
{
	while(*s != 0){
		putc(*s++);
	}	
}

int printf(char *fmt, ...)
{	
	char *cp = fmt; //cp points to the fmt string
	u16 *ip = (u16 *) &fmt + 1; //ip points to the first item
		
	u32 *up;

	while(*cp){
		if(*cp != '%'){
			putc(*cp);
			if(*cp=='\n')
				putc('\r');
			cp++; continue;
		}
		cp++;
		switch(*cp){
			case 'c' : putc(*ip); break;
			case 's' : prints(*ip); break;
			case 'u' : printu(*ip); break;
			case 'd' : printd(*ip); break;
			case 'x' : printx(*ip); break;
		
		}
		cp++; ip++;
	}	

}

//caller must provide REAL memory s[MAXLEN]
char *gets(char s[ ]){
	char c, *t = s; 
	int len = 0;
	
	while( (c = getc() ) != '\r' && len < MAXLEN - 1){
		*t++ = c;
		putc(c);
		len++;
	}

	*t = 0;
	return s;

}






