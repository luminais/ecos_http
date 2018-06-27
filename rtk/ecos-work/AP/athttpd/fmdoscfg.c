#include <network.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "http.h"
#include "apmib.h"
#include "sys_utility.h"
#include "multipart.h"
#include "asp.h"
#include "sys_init.h"


/*#ifdef HOME_GATEWAY
#define DOS_SUPPORT
#endif*/

void formDosCfg(char *postData, int len)
{
	char	*submitUrl, *tmpStr;
	char	tmpBuf[100];
	int	floodCount=0,blockTimer=0;
	long	enabled = 0;

	submitUrl = get_cstream_var(postData,len,"submit-url","");   // hidden page
	
#ifdef DOS_SUPPORT
	apmib_get(MIB_DOS_ENABLED, (void *)&enabled);

	tmpStr = get_cstream_var(postData,len,"dosEnabled","");
	if(!strcmp(tmpStr, "ON")) {
		enabled |= 1;

		tmpStr = get_cstream_var(postData,len,"sysfloodSYN","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 2;
			tmpStr = get_cstream_var(postData,len,"sysfloodSYNcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSSYN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~2;
		}
		tmpStr = get_cstream_var(postData,len,"sysfloodFIN","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 4;
			tmpStr = get_cstream_var(postData,len,"sysfloodFINcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSFIN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~4;
		}
		tmpStr = get_cstream_var(postData,len,"sysfloodUDP","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 8;
			tmpStr = get_cstream_var(postData,len,"sysfloodUDPcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSUDP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~8;
		}
		tmpStr = get_cstream_var(postData,len,"sysfloodICMP","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10;
			tmpStr = get_cstream_var(postData,len,"sysfloodICMPcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSICMP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x10;
		}
		tmpStr = get_cstream_var(postData,len,"ipfloodSYN","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20;
			tmpStr = get_cstream_var(postData,len,"ipfloodSYNcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPSYN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x20;
		}
		tmpStr = get_cstream_var(postData,len,"ipfloodFIN","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40;
			tmpStr = get_cstream_var(postData,len,"ipfloodFINcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPFIN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x40;
		}
		tmpStr = get_cstream_var(postData,len,"ipfloodUDP","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x80;
			tmpStr = get_cstream_var(postData,len,"ipfloodUDPcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPUDP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x80;
		}
		tmpStr = get_cstream_var(postData,len,"ipfloodICMP","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x100;
			tmpStr = get_cstream_var(postData,len,"ipfloodICMPcount","");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPICMP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x100;
		}
		tmpStr = get_cstream_var(postData,len,"TCPUDPPortScan","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x200;

			tmpStr = get_cstream_var(postData,len,"portscanSensi","");
			if( tmpStr[0]=='1' ) {
				enabled |= 0x800000;
			}
			else{
				enabled &= ~0x800000;
			}
		}
		else{
			enabled &= ~0x200;
		}
		tmpStr = get_cstream_var(postData,len,"ICMPSmurfEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400;
		}
		else{
			enabled &= ~0x400;
		}
		tmpStr = get_cstream_var(postData,len,"IPLandEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x800;
		}
		else{
			enabled &= ~0x800;
		}
		tmpStr = get_cstream_var(postData,len,"IPSpoofEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x1000;
		}
		else{
			enabled &= ~0x1000;
		}
		tmpStr = get_cstream_var(postData,len,"IPTearDropEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x2000;
		}
		else{
			enabled &= ~0x2000;
		}
		tmpStr = get_cstream_var(postData,len,"PingOfDeathEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x4000;
		}
		else{
			enabled &= ~0x4000;
		}
		tmpStr = get_cstream_var(postData,len,"TCPScanEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x8000;
		}
		else{
			enabled &= ~0x8000;
		}
		tmpStr = get_cstream_var(postData,len,"TCPSynWithDataEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10000;
		}
		else{
			enabled &= ~0x10000;
		}
		tmpStr = get_cstream_var(postData,len,"UDPBombEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20000;
		}
		else{
			enabled &= ~0x20000;
		}
		tmpStr = get_cstream_var(postData,len,"UDPEchoChargenEnabled","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40000;
		}
		else{
			enabled &= ~0x40000;
		}
		tmpStr = get_cstream_var(postData,len,"sourceIPblock","");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400000;
			tmpStr = get_cstream_var(postData,len,"IPblockTime","");
			string_to_dec(tmpStr,&blockTimer);
			if ( apmib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer) == 0) {
				strcpy(tmpBuf, ("Set DoS IP Block Timer error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x400000;
			blockTimer = 0;
			if ( apmib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer) == 0) {
				strcpy(tmpBuf, ("Set DoS IP Block Timer error!"));
				goto setErr;
			}
		}
	}
	else
		enabled = 0;

	if ( apmib_set(MIB_DOS_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, ("Set DoS enable error!"));
		goto setErr;
	}

	apmib_update_web(CURRENT_SETTING);

#endif
#ifdef HAVE_SYSTEM_REINIT
			{
				wait_redirect("Apply Changes", 3,submitUrl);
				sleep(1);
				kick_reinit_m(SYS_DOS_M);
			}
#else
#ifndef NO_ACTION
	run_init_script("all");
#endif

	OK_MSG(submitUrl);
#endif

	save_cs_to_file();

	return;

setErr:
	ERR_MSG(tmpBuf);
}


