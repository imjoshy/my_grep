//
//  grep.h
//  Grep_project
//
//  Created by Josh Maranan on 3/15/19.
//  Copyright © 2019 Josh Maranan. All rights reserved.
//

#ifndef grep_h
#define grep_h



int    close(int);
int    fork(void);
int    wait(int *);

char    *mktemp(char *);

char *getblock(unsigned int atl, int iof);
char *getline_(unsigned int tl);
char *place(char *sp, char *l1, char *l2);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
int backref(int i, char *lp);
void blkio(int b, char *buf, ssize_t (*iofcn)(int, void*, size_t));
void blkio_(int b, char *buf, ssize_t (*iofcn)(int, const void*, size_t));
int cclass(char *set, int c, int af);
void commands(void);
void compile(int eof);
void error(char *s);
int execute(unsigned int *addr);
void exfile(void);
void filename(int comm);
int getchr(void);
int getfile(void);
int getnum(void);
void global(int k);
void greperror(char c);
void grepline(void);
void grepreadfile(int);
void init(void);
unsigned int *address(void);
void newline(void);
void nonzero(void);
void onhup(int n);
void onintr(int n);
void print(void);
void putchr(int ac);
void putd(void);
void putfile(void);
int putline(void);
void quit(int n);
void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);
void setnoaddr(void);
void squeeze(int i);
void cerror(void);

#endif /* grep_h */
