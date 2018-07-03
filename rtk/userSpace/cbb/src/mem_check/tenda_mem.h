#ifndef __TENDA_MEM_H__
#define __TENDA_MEM_H__
#include<mem_check.h>
#define malloc(s) tenda_malloc(s, __FUNCTION__)  
#define calloc(c, s) tenda_calloc(c, s,__FUNCTION__)  
#define free(p) tenda_free(p)
#endif