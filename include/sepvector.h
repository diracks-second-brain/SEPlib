#ifndef SEPVECTOR_H
#define SEPVECTOR_H yada
#include<prototypes.h>

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int setvalf(float *array, int size, float val);
int setvali(int *array, int size, int val);
_XFUNCPROTOEND
#else
int setvalf();
int setvali();
#endif
#endif
