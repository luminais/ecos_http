/*
 *  Performance Profiling routines
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 */

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_intr.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/var_arch.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/infra/diag.h>
#include <string.h>

#include <cyg/hal/romeperf.h>

#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __IRAM
#else
#define __IRAM		__attribute__ ((section(".iram-gen")))
#endif

#define rtlglue_malloc(size)	malloc(size)
#define rtlglue_free(p)	free(p)
#define rtlglue_printf diag_printf


enum CP3_COUNTER
{
	CP3CNT_CYCLES = 0,
	CP3CNT_NEW_INST_FECTH,
	CP3CNT_NEW_INST_FETCH_CACHE_MISS,
	CP3CNT_NEW_INST_MISS_BUSY_CYCLE,
	CP3CNT_DATA_STORE_INST,
	CP3CNT_DATA_LOAD_INST,
	CP3CNT_DATA_LOAD_OR_STORE_INST,
	CP3CNT_EXACT_RETIRED_INST,
	CP3CNT_RETIRED_INST_FOR_PIPE_A,
	CP3CNT_RETIRED_INST_FOR_PIPE_B,
	CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS,
	CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE,
	CP3CNT_RESERVED12,
	CP3CNT_RESERVED13,
	CP3CNT_RESERVED14,
	CP3CNT_RESERVED15,
};

char *cp3_control_description[16] = {
	"cycles",
	"new instruction fetches",
	"instructions of ICACHE miss and uncacheable fetch",
	"busy cycles of ICACHE miss and uncacheable fetch",
	"data store instructions",
	"data load instructions",
	"load or store instructions",
	"execution-completed instructions",
	"reserved",
	"reserved",
	"instructions of DCACHE misses and uncacheable load",
	"busy cycles of DCACHE misses and uncacheable load/store",
	"reserved",
	"reserved",
	"reserved",
	"reserved"};

/* Local variables */
static uint64 tempVariable64;
static uint32 tempVariable32;
static uint64 currCnt[4];

/* Global variables */
#ifdef CONFIG_WIRELESS_LAN_MODULE
static uint64 cnt1, cnt2;
static rlx_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];
static uint32 rlx_romeperf_inited = 0;
static uint32 rlx_romeperf_enable = TRUE;
#else
uint64 cnt1, cnt2;
rlx_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];
uint32 rlx_romeperf_inited = 0;
uint32 rlx_romeperf_enable = TRUE;
#endif

char romePerfDesc[ROMEPERF_INDEX_MAX][32];
	
static int rlx_measurement_type = 0;
static int rlx_measurement_monctl[4];

__IRAM void CP3_COUNTER0_INIT( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la		$9, 0x80000000	;\
	or		$8, $9			;\
	mtc0	$8, $12			;\
");
}

__IRAM uint32 CP3_COUNTER0_IS_INITED( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la		$9, tempVariable32;\
	sw		$8, 0($9)		;\
");
	return tempVariable32;
}

__IRAM void CP3_COUNTER0_START( void )
{
    if (rlx_measurement_type==0) {
	/* Inst */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_NEW_INST_FETCH_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
	rlx_measurement_monctl[0] = CP3CNT_CYCLES;
	rlx_measurement_monctl[1] = CP3CNT_NEW_INST_FECTH;
	rlx_measurement_monctl[2] = CP3CNT_NEW_INST_FETCH_CACHE_MISS;
	rlx_measurement_monctl[3] = CP3CNT_NEW_INST_MISS_BUSY_CYCLE;
    }
    else if (rlx_measurement_type==1) {
	/* Data (LOAD+STORE) */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
	rlx_measurement_monctl[0] = CP3CNT_CYCLES;
	rlx_measurement_monctl[1] = CP3CNT_DATA_LOAD_OR_STORE_INST;
	rlx_measurement_monctl[2] = CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS;
	rlx_measurement_monctl[3] = CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE;
    }
    else {
	/* Data (STORE) */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_DATA_LOAD_INST)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
	rlx_measurement_monctl[0] = CP3CNT_DATA_LOAD_INST;
	rlx_measurement_monctl[1] = CP3CNT_DATA_STORE_INST;
	rlx_measurement_monctl[2] = CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS;
	rlx_measurement_monctl[3] = CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE;
    }

__asm__ __volatile__ \
("  ;\
	la		$8, tempVariable32	;\
	lw		$8, 0($8)			;\
	ctc3 	$8, $0				;\
");
}

__IRAM void CP3_COUNTER0_STOP( void )
{
__asm__ __volatile__ \
("	;\
	ctc3 	$0, $0			;\
");
}

__IRAM uint64 CP3_COUNTER0_GET( void )
{
__asm__ __volatile__ \
("	;\
	la		$8, tempVariable64;\
	mfc3	$9, $9			;\
	sw		$9, 0($8)		;\
	mfc3	$9, $8			;\
	sw		$9, 4($8)		;\
");
	return tempVariable64;
}

__IRAM void CP3_COUNTER0_GET_ALL( void )
{
__asm__ __volatile__ \
("	;\
	la		$4, currCnt		;\
	mfc3	$9, $9			;\
	sw		$9, 0x00($4)	;\
	mfc3	$9, $8			;\
	sw		$9, 0x04($4)	;\
	mfc3	$9, $11			;\
	sw		$9, 0x08($4)	;\
	mfc3	$9, $10			;\
	sw		$9, 0x0C($4)	;\
	mfc3	$9, $13			;\
	sw		$9, 0x10($4)	;\
	mfc3	$9, $12			;\
	sw		$9, 0x14($4)	;\
	mfc3	$9, $15			;\
	sw		$9, 0x18($4)	;\
	mfc3	$9, $14			;\
	sw		$9, 0x1C($4)	;\
");
}

int32 rlx_romeperfInit(int how)
{
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_START();

	rlx_romeperf_inited = TRUE;
	rlx_romeperf_enable = TRUE;
	memset( &romePerfStat, 0, sizeof( romePerfStat ) );
	if (how)
		memset( romePerfDesc, 0, sizeof( romePerfDesc ) );
		
	return SUCCESS;
}

int32 rlx_romeperfReset(int type)
{
	rlx_measurement_type = type;
	rlx_romeperfInit(0);
	return SUCCESS;
}

/* old fashion function, for reference only. */
/*
int32 rlx_romeperfStart(void)
{
	if ( rlx_romeperf_inited == FALSE ) rlx_romeperfInit();
	
	START_AND_GET_CP3_COUNTER0( cnt1 );

	return SUCCESS;
}

int32 rlx_romeperfStop( uint64 *pDiff )
{
	if ( rlx_romeperf_inited == FALSE ) rlx_romeperfInit();
	
	STOP_AND_GET_CP3_COUNTER0( cnt2 );

	*pDiff = cnt2 - cnt1;
	return SUCCESS;
}
*/

int32 rlx_romeperfGet( uint64 *pGet )
{
	if ( rlx_romeperf_inited == FALSE ) return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	*pGet = CP3_COUNTER0_GET();
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

int32 rlx_romeperfPause( void )
{
	if ( rlx_romeperf_inited == FALSE ) return FAILED;

	rlx_romeperf_enable = FALSE;
	
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	
	return SUCCESS;
}

int32 rlx_romeperfResume( void )
{
	if ( rlx_romeperf_inited == FALSE ) return FAILED;

	rlx_romeperf_enable = TRUE;
	
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();
 	
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

void rlx_romeperfSetDesc(uint32 index, char *desc)
{
	memcpy(romePerfDesc[index], desc, 31);
	romePerfDesc[index][31]='\0';
}

__IRAM int32 rlx_romeperfEnterPoint( uint32 index )
{
	if ( rlx_romeperf_inited == FALSE ||
	     rlx_romeperf_enable == FALSE ) return FAILED;
	if ( index >= (sizeof(romePerfStat)/sizeof(rlx_romeperf_stat_t)) )
		return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();
	romePerfStat[index].tempCycle[0] = currCnt[0];
	romePerfStat[index].tempCycle[1] = currCnt[1];
	romePerfStat[index].tempCycle[2] = currCnt[2];
	romePerfStat[index].tempCycle[3] = currCnt[3];
	romePerfStat[index].hasTempCycle = TRUE;
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

__IRAM int32 rlx_romeperfExitPoint( uint32 index )
{
	if ( rlx_romeperf_inited == FALSE ||
	     rlx_romeperf_enable == FALSE ) return FAILED;
	if ( index >= (sizeof(romePerfStat)/sizeof(rlx_romeperf_stat_t)) )
		return FAILED;
	if ( romePerfStat[index].hasTempCycle == FALSE )
		return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();
	
	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();
	romePerfStat[index].accCycle[0] += currCnt[0]-romePerfStat[index].tempCycle[0];
	romePerfStat[index].accCycle[1] += currCnt[1]-romePerfStat[index].tempCycle[1];
	romePerfStat[index].accCycle[2] += currCnt[2]-romePerfStat[index].tempCycle[2];
	romePerfStat[index].accCycle[3] += currCnt[3]-romePerfStat[index].tempCycle[3];
	romePerfStat[index].hasTempCycle = FALSE;
	romePerfStat[index].executedNum++;
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

rlx_romeperf_stat_t statSnapShot[ROMEPERF_INDEX_MAX];

int32 rlx_romeperfDump( int start, int end )
{
#if 0
	int i;

	rtlglue_printf( "index %30s %12s %8s %10s\n", "description", "accCycle", "totalNum", "Average" );
	for( i = start; i <= end; i++ )
	{
		if ( romePerfStat[i].executedNum == 0 )
		{
			rtlglue_printf( "[%3d] %30s %12s %8s %10s\n", i, romePerfDesc[i], "--", "--", "--" );
		}
		else
		{
			int j;
			rtlglue_printf( "[%3d] %30s ", 
			                i, romePerfDesc[i] );
			for( j =0; j < sizeof(romePerfStat[i].accCycle)/sizeof(romePerfStat[i].accCycle[0]);
			     j++ )
			{
				uint32 *pAccCycle = (uint32*)&romePerfStat[i].accCycle[j];
				uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/romePerfStat[i].executedNum)) +
				                   /* Low-word */(pAccCycle[1]/romePerfStat[i].executedNum);

				rtlglue_printf( "%12llu %8u %10u\n",
				                romePerfStat[i].accCycle[j],
				                romePerfStat[i].executedNum, 
				                avrgCycle
				                );
				rtlglue_printf( " %3s  %30s ", "", "" );
			}
			rtlglue_printf( "\r" );
		}
	}
	
	return SUCCESS;
#else
	int i;
	cyg_uint32 _old;

	/*
	rlx_romeperf_stat_t* statSnapShot = rtlglue_malloc(sizeof(rlx_romeperf_stat_t) * (end - start + 1) );
	if( statSnapShot == NULL )
	{
		rtlglue_printf("statSnapShot mem alloc failed\n");
		return FAILED;
	}
	*/
	rtlglue_printf( "\nindex %32s %12s %8s %10s\n", "description", "accCycle", "totalNum", "Average" );
	
	if (end >= ROMEPERF_INDEX_MAX)
		end = ROMEPERF_INDEX_MAX-1;
	HAL_DISABLE_INTERRUPTS(_old);
	for( i = start; i <= end; i++ )
	{
		int j;
		for( j =0; j < sizeof(romePerfStat[i].accCycle)/sizeof(romePerfStat[i].accCycle[0]); j++ )
		{
			statSnapShot[i].accCycle[j]  = romePerfStat[i].accCycle[j];
			statSnapShot[i].tempCycle[j] = romePerfStat[i].tempCycle[j];
		}
		statSnapShot[i].executedNum  = romePerfStat[i].executedNum;
		statSnapShot[i].hasTempCycle = romePerfStat[i].hasTempCycle;
	}
	HAL_RESTORE_INTERRUPTS(_old);

	for( i = start; i <= end; i++ )
	{
		if ( statSnapShot[i].executedNum == 0 )
		{
			rtlglue_printf( "[%03d] %32s %12s %8s %10s\n", i, romePerfDesc[i], "--", "--", "--" );
		}
		else
		{
			int j;
			rtlglue_printf( "[%03d] %32s ", i, romePerfDesc[i] );
			for( j =0; j < sizeof(statSnapShot[i].accCycle)/sizeof(statSnapShot[i].accCycle[0]); j++ )
			{
				uint32 *pAccCycle = (uint32*)&statSnapShot[i].accCycle[j];
				uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/statSnapShot[i].executedNum)) +
				                   /* Low-word */(pAccCycle[1]/statSnapShot[i].executedNum);
				rtlglue_printf( "%12llu %8u %10u",
				statSnapShot[i].accCycle[j], 
				statSnapShot[i].executedNum, 
				avrgCycle );
				rtlglue_printf( " %s\n", cp3_control_description[rlx_measurement_monctl[j]]);
				rtlglue_printf( " %3s  %32s ", "", "" );
			}
			rtlglue_printf( "\r" );
		}
	}

	//rtlglue_free(statSnapShot);
	
	return SUCCESS;
#endif
}
