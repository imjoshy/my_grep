#ifndef grep_h
#define grep_h

char    *mktemp(char *);

char *getblock(unsigned int atl, int iof);
char *getline_(unsigned int tl);
char *place(char *sp, char *l1, char *l2);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
void blkio_write(int b, char *buf, ssize_t (*iofcn)(int, const void*, size_t));
void blkio_read(int b, char *buf, ssize_t (*iofcn)(int, void*, size_t));
void commands(void);
void compile(int eof);
void error(char *s);
int execute(unsigned int *addr);
void filename(int comm);
int getchr(void);
int getfile(void);
void global(int k);
void greperror(char c);
void grepline(void);
void caseread_(int c);
void readfile(const char* filename);
void search(const char* re);
void init(void);
unsigned int *address(void);
void print(void);
void puts_(char *sp);
void putchr_(int ac);
void putd(void);
void putfile(void);
int putline(void);
void quit(int n);
void setwide(void);

#endif /* grep_h */
