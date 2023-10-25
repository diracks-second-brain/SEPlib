#include<sitedef.h>
#ifndef SU_SEP2
#define SU_SEP2 ads

#include "/net/gis/CWP//include/segy.h"

#undef gettr
#undef gettra
#undef puttr

#define gettr(x) tgettr("in", (x))
#define gettra(x, y) tgettra("in", (x), (y))
#define puttr(x) tputtr("out", (x))

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
extern int tgettr(char*,segy*);
extern int tgettra(char*,segy*,int);
extern int tputtr(char*,segy*);
void both_initargs(int,char**);
extern int finish_susep(void);
_XFUNCPROTOEND
#else
extern int tgettr(char*,segy*);
extern int tgettra(char*,segy*,int);
extern int tputtr(char*,segy*);
void both_initargs(int,char**);
extern int finish_susep(void);
#endif




#endif
/*  $Id: segy.h.in,v 1.1.1.1 2004/03/25 06:37:22 cvs Exp $ */
