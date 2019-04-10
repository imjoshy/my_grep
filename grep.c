#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include "grep.h"

#define NULL 0
const int BUFSIZE = 100, BLKSIZE = 4096, NBLK = 2047, FNSIZE = 128, LBSIZE = 4096, ESIZE = 256, GBSIZE = 256, NBRA = 5, CBRA = 1, CCHR = 2, CDOT = 4, CCL = 6, NCCL = 8, CDOL = 10, CEOF = 11, CKET = 12, CBACK = 14, CCIRC = 15, STAR = 01, ARGC_ERROR = 1, READ = 0, WRITE = 1, MAXCHAR = 1000000, MAXLINE = 1000;
char	line[70], *linp = line, buf[BUFSIZE], Q[] = "", T[] = "TMP", savedfile[FNSIZE], file[FNSIZE], linebuf[LBSIZE], expbuf[ESIZE+4], genbuf[LBSIZE], *nextip, *linebp;
int	bufp = 0, peekc, lastc, given, ninbuf, io, pflag, vflag = 1, oflag, listf, col, tfile = -1, tline, iblock = -1, oblock = -1, ichanged, nleft, names[26], anymarks, nbra, subnewa, fchange, wrapp, nbra, subnewa, fchange, wrapp, numlines = 0;
unsigned int	*addr1, *addr2, *dot, *dol, *zero;
long	count;
char	*globp, *tfname, *loc1, *loc2, ibuff[BLKSIZE], obuff[BLKSIZE], *braslist[NBRA], braelist[NBRA], filebuf[MAXCHAR], *fileline[MAXLINE], grepbuf[GBSIZE];
unsigned nlall = 128;
char	*mktemp(char *);
char	tmpXXXXX[50] = "/tmp/eXXXXX";
jmp_buf	savej;
typedef void	(*SIG_TYP)(int);

int main(int argc, char *argv[]) {
  if (argc != 3) { 
    fprintf(stderr, "Usage: ./grep searchre file(s)\n"); 
    exit(ARGC_ERROR); 
  }
  zero = (unsigned *)malloc(nlall * sizeof(unsigned));  
  tfname = mktemp(tmpXXXXX);  
  init();
  readfile(argv[2]);		//Read this file
  search(argv[1]);		//Search for the word
  printf("\nquitting...\n");
  exit(1);
}

//WORKS
void readfile(const char* filename) {		//Read file into buffer and put into fileline line by line
	FILE* f = fopen(filename, "r");		//Open the file filename to read only
	if (f) {		//If file failed to open, f will be NULL
		int c;
		int i = 0;
		//Read text from file into string buffer filebuf
		for(; ((c = fgetc(f)) != EOF);) { 
			filebuf[i++] = c;
		}
		filebuf[i] = '\0';		//End of filebuf
		i = 0;		//Reset i
		for(fileline[i] = strtok(filebuf, "\n"); fileline[i] != NULL;) {
			fileline[++i] = strtok(NULL, "\n");
			++numlines;
		}
		//File is now placed into a char** by line
		fclose(f);
	}
	else { printf("Failed to open %s.\n", filename); }
}

//KIND OF WORKS
void search(const char* word) {
	int i = 0;
	for(; i < numlines; ++i) {		//Search for the word in each line
		if(strstr(fileline[i], word) != NULL) {		//If word found in the line
			printf("%s\n", fileline[i]);
		}
	}
}

int getch_(void) {  
  if (lastc=peekc) {
		peekc = 0;
		return(lastc);
  }
  char c = (bufp > 0) ? buf[--bufp] : getchar();  
  lastc = c & 0177;
  return lastc;
}

void ungetch_(int c) { 
  if (bufp >= BUFSIZE) {  
    printf("ungetch: overflow\n"); 
  }
  else {  
    buf[bufp++] = c; 
  } 
}

/*	//PROVIDED but does not work for my program
void search(const char* re) {  
  char buf[GBSIZE];  
  snprintf(buf, sizeof(buf), "/%s\n", re);  // / and \n very important 
  printf("g%s", buf);
  const char* p = buf + strlen(buf) - 1;
  while (p >= buf) { ungetch_(*p--); }
  global(1);
}
*/

void greperror(char c) {  
  getch_();  /* throw away '\n' */
  snprintf(grepbuf, sizeof(grepbuf), "\'%c\' is a non-grep command", c);  
  puts_(grepbuf);
}

void grepline(void) {
  getch_();  // throw away newline after command
  for (int i = 0; i < 50; ++i) { putchr_('-'); }   
  putchr_('\n');
}

void commands(void) {  
	int c;  
	char lastsep;
        for (;;) {  
	unsigned int* a1;
    	if (pflag) { 
		pflag = 0;  
		addr1 = addr2 = dot;  
		print(); 
	}  
	c = '\n';
       for (addr1 = 0;;) {  
	lastsep = c;  
	a1 = address();  
	c = getch_();
        if (c != ',' && c != ';') { break; }  
	if (lastsep==',') { error(Q); }
        if (a1==0) {  a1 = zero+1;  
		if (a1 > dol) { a1--; }  
	}  
	addr1 = a1;  
	if (c == ';') { dot = a1; }
       }
       if (lastsep != '\n' && a1 == 0) { a1 = dol; }
       if ((addr2 = a1)==0) { 
	given = 0;  
	addr2 = dot;  
       } 
       else { given = 1; }  
       if (addr1==0) { addr1 = addr2; }
       switch(c) {
       case 'p':  case 'P':   
	print();  
	continue;
      case EOF:  default:  
	return;
    }
  }
}

void print(void) {
	unsigned int *a1 = addr1;
	do {
		puts_(getline_(*a1++));
	} while (a1 <= addr2);
	dot = addr2;
	pflag = 0;
}

unsigned int* address(void) {
	int sign = 1;
	unsigned int *a = dot, *b;
	int opcnt = 0, nextopand = -1;
	int c;
	do {
		do c = getch_(); while (c==' ' || c=='\t');
		if ('0'<=c && c<='9') {
			peekc = c;
			if (!opcnt)
				a = zero;
		} else switch (c) {
		case '$':
			a = dol;
			/* fall through */
		case '.':
			if (opcnt)
				error(Q);
			break;
		case '\'':
			c = getch_();
			if (opcnt || c<'a' || 'z'<c)
				error(Q);
			a = zero;
			do a++; while (a<=dol && names[c-'a']!=(*a&~01));
			break;
		case '?':
			sign = -sign;
			/* fall through */
		case '/':
			compile(c);
			b = a;
			for (;;) {
				a += sign;
				if (a<=zero)
					a = dol;
				if (a>dol)
					a = zero;
				if (execute(a))
					break;
				if (a==b)
					error(Q);
			}
			break;
		default:
			if (nextopand == opcnt) {
				a += sign;
				if (a<zero || dol<a)
					continue;       /* error(Q); */
			}
			if (c!='+' && c!='-' && c!='^') {
				peekc = c;
				if (opcnt==0)
					a = 0;
				return (a);
			}
			sign = 1;
			if (c!='+')
				sign = -sign;
			nextopand = ++opcnt;
			continue;
		}
		sign = 1;
		opcnt++;
	} while (zero<=a && a<=dol);
	error(Q);
	/*NOTREACHED*/
	return 0;
}


void setwide(void) {
	if (!given) {
		addr1 = zero + (dol>zero);
		addr2 = dol;
	}
}

void filename(int comm) {
	char *p1, *p2;
	int c = getch_();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			error(Q);
		p2 = file;
		while (*p2++ = *p1++)
			;
		return;
	}
	if (c!=' ')
		error(Q);
	while ((c = getch_()) == ' ')
		;
	if (c=='\n')
		error(Q);
	p1 = file;
	do {
		if (p1 >= &file[sizeof(file)-1] || c==' ' || c==EOF)
			error(Q);
		*p1++ = c;
	} while ((c = getch_()) != '\n');
	*p1++ = 0;
	if (savedfile[0]==0 || comm=='e' || comm=='f') {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

void error(char *s) {
	int c;
	wrapp = 0;
	putchr_('?');
	puts_(s);
	count = 0;
	lseek(0, (long)0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getch_()) != '\n' && c != EOF)
			;
	if (io > 0) {
		close(io);
		io = -1;
	}
	longjmp(savej, 1);
}

int getchr(void) {
	char c;
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}
	if (read(0, &c, 1) <= 0)
		return(lastc = EOF);
	lastc = c&0177;
	return(lastc);
}

int getfile(void) {
	int c;
	char *lp = linebuf, *fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				if (lp>linebuf) {
					puts_("'\\n' appended");
					*genbuf = '\n';
				}
				else return(EOF);
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) {
				if (*fp++ & 0200)
					break;
			}
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0')
			continue;
		if (c&0200 || lp >= &linebuf[LBSIZE]) {
			lastc = '\n';
			error(Q);
		}
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

int append(int (*f)(void), unsigned int *a) {
	unsigned int *a1, *a2, *rdot;
	int nline = 0, tl;
	while ((*f)() == 0) {
		if ((dol-zero)+1 >= nlall) {
			unsigned *ozero = zero;
			nlall += 1024;
			if ((zero = (unsigned *)realloc((char *)zero, nlall*sizeof(unsigned)))==NULL) {
				error("MEM?");
			}
			dot += zero - ozero;
			dol += zero - ozero;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

void quit(int n) {
	if (vflag && fchange && dol!=zero) {
		fchange = 0;
		error(Q);
	}
	unlink(tfname);
	exit(0);
}

char * getline_(unsigned int tl) {
	char *bp = getblock(tl, READ), *lp = linebuf;
	int nl = nleft;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl+=(BLKSIZE/2), READ);
			nl = nleft;
		}
	return(linebuf);
}

int putline(void) {
	unsigned int tl = tline;
	char *bp = getblock(tl, WRITE), *lp = linebuf;
	int nl = nleft;
	fchange = 1;
	tl &= ~((BLKSIZE/2)-1);
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl+=(BLKSIZE/2), WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

char* getblock(unsigned int atl, int iof) {
	int bno = (atl/(BLKSIZE/2)), off = (atl<<1) & (BLKSIZE-1) & ~03;
	if (bno >= NBLK) {
		lastc = '\n';
		error(T);
	}
	nleft = BLKSIZE - off;
	if (bno==iblock) {
		ichanged |= iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

void blkio(int b, char *buf, int (*iofcn)(int, char*, int)) {
	lseek(tfile, (long)b*BLKSIZE, 0);
	if ((*iofcn)(tfile, buf, BLKSIZE) != BLKSIZE) {
		error(T);
	}
}

void init(void) {
	int *markp;
	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; )
		*markp++ = 0;
	subnewa = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	dot = dol = zero;
}

void global(int k) {
	char *gp;
	int c;
	unsigned int *a1;
	char globuf[GBSIZE];
	if (globp)
		error(Q);
	setwide();
	if ((c=getch_())=='\n')
		error(Q);
	compile(c);
	gp = globuf;
	while ((c = getch_()) != '\n') {
		if (c==EOF)
			error(Q);
		if (c=='\\') {
			c = getch_();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			error(Q);
	}
	if (gp == globuf)
		*gp++ = 'p';
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		*a1 &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(a1)==k)
			*a1 |= 01;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			dot = a1;
			globp = globuf;
			commands();
			a1 = zero;
		}
	}
}

void defchar(char** ep, int* c) {
	*ep++ = CCHR;
	*ep++ = c;
}

void compile(int eof) {
	int c;
	char *ep = expbuf;
	char *lastep;
	char bracket[NBRA], *bracketp = bracket;
	int cclcnt;
	if ((c = getch_()) == '\n') {
		peekc = c;
		c = eof;
	}
	if (c == eof) {
		if (*ep==0)
			error(Q);
		return;
	}
	nbra = 0;
	if (c=='^') {
		c = getch_();
		*ep++ = CCIRC;
	}
	peekc = c;
	lastep = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE]) {
			exit(0);
		}
		c = getch_();
		if (c == '\n') {
			peekc = c;
			c = eof;
		}
		if (c==eof) {
			if (bracketp != bracket) {
				exit(0);
			}
			*ep++ = CEOF;
			return;
		}
		if (c!='*')
			lastep = ep;

		switch (c) {
		case '\\':
			if ((c = getch_())=='(') {
				if (nbra >= NBRA) {
					exit(0);
				}
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket) {
					exit(0);
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c>='1' && c<'1'+NBRA) {
				*ep++ = CBACK;
				*ep++ = c-'1';
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n') {
				exit(0);
			}
			*ep++ = c;
			continue;
		case '.':
			*ep++ = CDOT;
			continue;
		case '\n':
			exit(0);

		case '*':
			*lastep |= STAR;
			continue;
		case '$':
			*ep++ = CDOL;
			continue;
		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getch_()) == '^') {
				c = getch_();
				ep[-2] = NCCL;
			}
			do {
				if (c=='\n') {
					exit(0);
				}
				if (c=='-' && ep[-1]!=0) {
					if ((c=getch_())==']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
						if (ep>=&expbuf[ESIZE]) {
							exit(0);
						}
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE]) {
					exit(0);
				}
			} while ((c = getch_()) != ']');
			lastep[1] = cclcnt;
			continue;
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

int execute(unsigned int *addr) {
	char *p1, *p2;
	int c;
	for (c=0; c<NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	p2 = expbuf;
	if (addr == (unsigned *)0) {
		if (*p2==CCIRC)
			return(0);
		p1 = loc2;
	} else if (addr==zero)
		return(0);
	else
		p1 = getline_(*addr);
	if (*p2==CCIRC) {
		loc1 = p1;
		return(advance(p1, p2+1));
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}

int advance(char *lp, char *ep) {
	char *curlp;
	int i;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);
	case CEOF:
		loc2 = lp;
		return(1);

	default:
		error(Q);
	}
}
void puts_(char *sp) {
	col = 0;
	while (*sp)
		putchr_(*sp++);
	putchr_('\n');
}

void putchr_(int ac) {
	char *lp = linp;
	int c = ac;
	if (listf) {
			if (col > (72-4-2)) {
				col = 8;
				*lp++ = '\\';
				*lp++ = '\n';
				*lp++ = '\t';
			}
			col++;
			if (c=='\b' || c=='\t' || c=='\\') {
				*lp++ = '\\';
				if (c=='\b')
					c = 'b';
				else if (c=='\t')
					c = 't';
				col++;
			} else if (c<' ' || c=='\177') {
				*lp++ = '\\';
				*lp++ =  (c>>6)    +'0';
				*lp++ = ((c>>3)&07)+'0';
				c     = ( c    &07)+'0';
				col += 3;
			}
	}
	*lp++ = c;
	if(c == '\n' || lp >= &line[64]) {
		linp = line;
		write(oflag?2:1, line, lp-line);
		return;
	}
	linp = lp;
}