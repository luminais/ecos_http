/*
 *  Performance Profiling Header File
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 */

#ifndef _ROMEPERF_H_
#define _ROMEPERF_H_

#include <pkgconf/hal.h>
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT

typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

struct rlx_romeperf_stat_s {
	uint64 accCycle[4];
	uint64 tempCycle[4];
	uint32 executedNum;
	uint32 hasTempCycle:1; /* true if tempCycle is valid. */
};
typedef struct rlx_romeperf_stat_s rlx_romeperf_stat_t;

#define ROMEPERF_INDEX_MAX 32

int32 rlx_romeperfInit(int how);
inline int32 rlx_romeperfStart(void);
inline int32 rlx_romeperfStop(void);
int32 rlx_romeperfReset(int type);
void rlx_romeperfSetDesc(uint32 index, char *desc);
int32 rlx_romeperfEnterPoint( uint32 index );
int32 rlx_romeperfExitPoint( uint32 index );
int32 rlx_romeperfDump( int start, int end );
int32 rlx_romeperfPause( void );
int32 rlx_romeperfResume( void );
int32 rlx_romeperfGet( uint64 *pGet );

extern rlx_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];

#else

#define rlx_romeperfInit(x) do {} while(0)
#define rlx_romeperfEnterPoint(x) do {} while(0)
#define rlx_romeperfExitPoint(x) do {} while(0)
#define rlx_romeperfReset(x) do {} while(0)
#define rlx_romeperfDump(x, y) do {} while(0)
#define rlx_romeperfSetDesc(x, y) do {} while(0)

#endif /* CYGPKG_HAL_ROMEPERF_SUPPORT */

#endif/* _ROMEPERF_H_ */
