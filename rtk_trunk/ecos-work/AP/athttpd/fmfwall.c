/*for fmtwall.c*/
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <time.h>

#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <stdio.h>                     // sprintf().
#include <stdlib.h>


#include "asp.h"
#include "fmget.h"
#include "http.h"
#include "socket.h"
#include "asp_form.h"
#include "apmib.h"
#include "sys_utility.h"
#include "sys_init.h"
#include "common.h"
#ifdef QOS_BY_BANDWIDTH
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_8367R)
#define MAX_BAND_WIDTH 1024000
#else
#define MAX_BAND_WIDTH 102400
#endif
#endif
#if defined(HAVE_FIREWALL)
extern void set_ipfw_rules(char *wan_intf, char *lan_intf);
void formFilter(char *wp, int len)
{
		char *strAddIp, *strAddPort, *strAddMac, *strDelPort, *strDelIp, *strDelMac;
		char *strApplyIp, *strApplyPort;
		char *strDelAllPort, *strDelAllIp, *strDelAllMac, *strVal, *submitUrl, *strComment;
		char *strFrom, *strTo;
		char tmpBuf[100];
		int entryNum, intVal, i, j;
		IPFILTER_T ipEntry, ipentrytmp;
		PORTFILTER_T portEntry, entrytmp;
		MACFILTER_T macEntry, macEntrytmp;
	//	struct in_addr curIpAddr, curSubnet;
		void *pEntry;
		//unsigned long v1, v2, v3;
		int num_id, get_id, add_id, del_id, delall_id, enable_id;
		char *strAddUrl, *strDelUrl;
		char *strDelAllUrl;
		URLFILTER_T urlEntry, urlEntrytmp;
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        char *strEndIpAddr;
        #endif
	
		strAddIp = get_cstream_var(wp, len,("addFilterIp"), "");
		strDelIp = get_cstream_var(wp, len,("deleteSelFilterIp"), "");
		strDelAllIp = get_cstream_var(wp, len,("deleteAllFilterIp"), "");
	
		strAddPort = get_cstream_var(wp, len,("addFilterPort"), "");
		strDelPort = get_cstream_var(wp, len,("deleteSelFilterPort"), "");
		strDelAllPort = get_cstream_var(wp, len,("deleteAllFilterPort"), "");
	
		strAddMac = get_cstream_var(wp, len,("addFilterMac"), "");
		strDelMac = get_cstream_var(wp, len,("deleteSelFilterMac"), "");
		strDelAllMac = get_cstream_var(wp, len,("deleteAllFilterMac"), "");
	
		strAddUrl = get_cstream_var(wp, len,("addFilterUrl"), "");
		strDelUrl = get_cstream_var(wp, len,("deleteSelFilterUrl"), "");
		strDelAllUrl = get_cstream_var(wp, len,("deleteAllFilterUrl"), "");
		
		
		if (strAddIp[0] || strDelIp[0] || strDelAllIp[0]) {
			num_id = MIB_IPFILTER_TBL_NUM;
			get_id = MIB_IPFILTER_TBL;
			add_id = MIB_IPFILTER_ADD;
			del_id = MIB_IPFILTER_DEL;
			delall_id = MIB_IPFILTER_DELALL;
			enable_id = MIB_IPFILTER_ENABLED;
			memset(&ipEntry, '\0', sizeof(ipEntry));
			pEntry = (void *)&ipEntry;
		}
		else if (strAddPort[0] || strDelPort[0] || strDelAllPort[0]) {
			num_id = MIB_PORTFILTER_TBL_NUM;
			get_id = MIB_PORTFILTER_TBL;
			add_id = MIB_PORTFILTER_ADD;
			del_id = MIB_PORTFILTER_DEL;
			delall_id = MIB_PORTFILTER_DELALL;
			enable_id = MIB_PORTFILTER_ENABLED;
			memset(&portEntry, '\0', sizeof(portEntry));
			pEntry = (void *)&portEntry;
		}
		else if (strAddMac[0] || strDelMac[0] || strDelAllMac[0]) {
			num_id = MIB_MACFILTER_TBL_NUM;
			get_id = MIB_MACFILTER_TBL;
			add_id = MIB_MACFILTER_ADD;
			del_id = MIB_MACFILTER_DEL;
			delall_id = MIB_MACFILTER_DELALL;
			enable_id = MIB_MACFILTER_ENABLED;
			memset(&macEntry, '\0', sizeof(macEntry));
			pEntry = (void *)&macEntry;
		}
		else {
			num_id = MIB_URLFILTER_TBL_NUM;
			get_id = MIB_URLFILTER_TBL;
			add_id = MIB_URLFILTER_ADD;
			del_id = MIB_URLFILTER_DEL;
			delall_id = MIB_URLFILTER_DELALL;
			enable_id = MIB_URLFILTER_ENABLED;
			memset(&urlEntry, '\0', sizeof(urlEntry));
			pEntry = (void *)&urlEntry;
		}
		// Set enable flag
		if ( strAddIp[0] || strAddPort[0] || strAddMac[0] || strAddUrl[0]) {
			strVal = get_cstream_var(wp, len,("enabled"), "");
			if ( !strcmp(strVal, "ON"))
				intVal = 1;
			else
				intVal = 0;
	
			if ( apmib_set(enable_id, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set enabled flag error!"));
				goto setErr_filter;
			}
		}
	
		strComment = get_cstream_var(wp, len,("comment"), "");
	
		/* Add IP filter */
		if (strAddIp[0]) {		
			strVal = get_cstream_var(wp, len,("ip"), "");
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
            strEndIpAddr = get_cstream_var(wp, len,("ipendaddr"), "");
			if (!strVal[0] && !strEndIpAddr[0] && !strComment[0])
            #else
			if (!strVal[0] && !strComment[0])
            #endif
				goto setOk_filter;
	
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
			if (!strVal[0] && !strEndIpAddr[0]) {
            #else
			if (!strVal[0]) {
            #endif
				strcpy(tmpBuf, ("Error! No ip address to set."));
				goto setErr_filter;
			}
	
			if(strVal[0]){			
			inet_aton(strVal, (struct in_addr *)&ipEntry.ipAddr);
#if 0
			getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
			getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);
	
			v1 = *((unsigned long *)ipEntry.ipAddr);
			v2 = *((unsigned long *)&curIpAddr);
			v3 = *((unsigned long *)&curSubnet);
	
			if ( (v1 & v3) != (v2 & v3) ) {
				strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
				goto setErr_filter;
			}
#endif
		}
	
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        if (strEndIpAddr[0])
        {
            inet_aton(strEndIpAddr, (struct in_addr *)&ipEntry.ipAddrEnd);
        }
        #endif
		}

		/* Add port filter */
		if (strAddPort[0]) {
			strFrom = get_cstream_var(wp, len,("fromPort"), "");
			strTo = get_cstream_var(wp, len,("toPort"), "");
			if (!strFrom[0] && !strTo[0] && !strComment[0])
				goto setOk_filter;
	
			if (!strFrom[0]) { // if port-forwarding, from port must exist
				strcpy(tmpBuf, ("Error! No from-port value to be set."));
				goto setErr_filter;
			}
			if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of from-port."));
				goto setErr_filter;
			}
			portEntry.fromPort = (unsigned short)intVal;
	
			if ( !strTo[0] )
				portEntry.toPort = portEntry.fromPort;
			else {
				if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, ("Error! Invalid value of to-port."));
					goto setErr_filter;
				}
				portEntry.toPort = (unsigned short)intVal;
			}
	
			if ( portEntry.fromPort  > portEntry.toPort ) {
				strcpy(tmpBuf, ("Error! Invalid port range."));
				goto setErr_filter;
			}
		}
	
		if (strAddPort[0] || strAddIp[0]) {
			strVal = get_cstream_var(wp, len,("protocol"), "");
			if (strVal[0]) {
				if ( strVal[0] == '0' ) {
					if (strAddPort[0])
						portEntry.protoType = PROTO_BOTH;
					else
						ipEntry.protoType = PROTO_BOTH;
				}
				else if ( strVal[0] == '1' ) {
					if (strAddPort[0])
						portEntry.protoType = PROTO_TCP;
					else
						ipEntry.protoType = PROTO_TCP;
				}
				else if ( strVal[0] == '2' ) {
					if (strAddPort[0])
						portEntry.protoType = PROTO_UDP;
					else
						ipEntry.protoType = PROTO_UDP;
				}
				else {
					strcpy(tmpBuf, ("Error! Invalid protocol type."));
					goto setErr_filter;
				}
			}
			else {
				strcpy(tmpBuf, ("Error! Protocol type cannot be empty."));
				goto setErr_filter;
			}
		}
	
		if (strAddMac[0]) {
			strVal = get_cstream_var(wp, len,("mac"), "");
			if (!strVal[0] && !strComment[0])
				goto setOk_filter;
	
			if ( !strVal[0] ) {
				strcpy(tmpBuf, ("Error! No mac address to set."));
				goto setErr_filter;
			}
			if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
				strcpy(tmpBuf, ("Error! Invalid MAC address."));
				goto setErr_filter;
			}
			//add same mac address check
            apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&entryNum);
    		for(j=1;j<=entryNum;j++)
			{
                memset(&macEntrytmp, 0x00, sizeof(macEntrytmp));
                *((char *)&macEntrytmp) = (char)j;
                if ( apmib_get(MIB_MACFILTER_TBL, (void *)&macEntrytmp))
            	{
					if (!memcmp(macEntrytmp.macAddr, macEntry.macAddr, 6))
					{
						strcpy(tmpBuf, ("rule already exist!"));
						goto setErr_filter;
					}
						
            	}
			}
		}
	
		if (strAddUrl[0]) {
			strVal = get_cstream_var(wp, len,"url", "");
			if (!strVal[0])// && !strComment[0])
				goto setOk_filter;
	
			if ( !strVal[0] ) {
				strcpy(tmpBuf, ("Error! No url keyword to set."));
				goto setErr_filter;
			}
			else
			{
				strcpy((char *)urlEntry.urlAddr, strVal);
			}
			//add same url rule check
            apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&entryNum);
    		for(j=1;j<=entryNum;j++)
			{
                memset(&urlEntrytmp, 0x00, sizeof(urlEntrytmp));
                *((char *)&urlEntrytmp) = (char)j;
                if ( apmib_get(MIB_URLFILTER_TBL, (void *)&urlEntrytmp))
            	{
            		if (strlen(urlEntry.urlAddr) == strlen(urlEntrytmp.urlAddr))
        			{
	            		if (!memcmp(urlEntrytmp.urlAddr, urlEntry.urlAddr, strlen(urlEntry.urlAddr)))
            			{
                            strcpy(tmpBuf, ("rule already exist!"));
                            goto setErr_filter;
            			}
        			}
            	}
			}
		}
        if (strAddPort[0]) {
            apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum);
    		for(j=1;j<=entryNum;j++)
            {
                memset(&entrytmp, 0x00, sizeof(entrytmp));
                *((char *)&entrytmp) = (char)j;
                if ( apmib_get(MIB_PORTFILTER_TBL, (void *)&entrytmp))
                {
                    if ((entrytmp.fromPort == portEntry.fromPort) &&
                        (entrytmp.toPort == portEntry.toPort)&&
                        ((entrytmp.protoType == portEntry.protoType)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_UDP)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_TCP)||
                        ((entrytmp.protoType==PROTO_TCP)&&portEntry.protoType==PROTO_BOTH)||
                        ((entrytmp.protoType==PROTO_UDP)&&portEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("rule already exist!"));
                            goto setErr_filter;
                        }
                        if ((((entrytmp.fromPort <= portEntry.fromPort) &&
                        (entrytmp.toPort >= portEntry.fromPort))||
                        ((entrytmp.fromPort <= portEntry.toPort) &&
                        (entrytmp.toPort >= portEntry.toPort)))&&
                        ((entrytmp.protoType == portEntry.protoType)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_UDP)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_TCP)||
                        ((entrytmp.protoType==PROTO_TCP)&&portEntry.protoType==PROTO_BOTH)||
                        ((entrytmp.protoType==PROTO_UDP)&&portEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("port overlap!"));
                            goto setErr_filter;
                        }
                        if ((((entrytmp.fromPort >= portEntry.fromPort) &&
                        (entrytmp.fromPort <= portEntry.toPort))||
                        ((entrytmp.toPort >= portEntry.fromPort) &&
                        (entrytmp.toPort <= portEntry.toPort)))&&
                        ((entrytmp.protoType == portEntry.protoType)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_UDP)||
                        ((entrytmp.protoType==PROTO_BOTH)&&portEntry.protoType==PROTO_TCP)||
                        ((entrytmp.protoType==PROTO_TCP)&&portEntry.protoType==PROTO_BOTH)||
                        ((entrytmp.protoType==PROTO_UDP)&&portEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("port overlap!"));
                            goto setErr_filter;
                        }
                }
            }
        }
        
        if (strAddIp[0]) {
            apmib_get(MIB_IPFILTER_TBL_NUM, (void *)&entryNum);
    		for(j=1;j<=entryNum;j++)
            {
                memset(&ipentrytmp, 0x00, sizeof(ipentrytmp));
                *((char *)&ipentrytmp) = (char)j;
                if ( apmib_get(MIB_IPFILTER_TBL, (void *)&ipentrytmp))
                {
                    #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
                    if (strEndIpAddr[0])
                    {
                        if (((*((unsigned int*)ipentrytmp.ipAddr)) == (*((unsigned int*)ipEntry.ipAddr)))&&
                            ((*((unsigned int*)ipentrytmp.ipAddrEnd))==(*((unsigned int*)ipEntry.ipAddrEnd)))&&
                            ((ipentrytmp.protoType==ipEntry.protoType)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_TCP)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_UDP)||
                            (ipentrytmp.protoType==PROTO_TCP&&ipEntry.protoType==PROTO_BOTH)||
                            (ipentrytmp.protoType==PROTO_UDP&&ipEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("rule already exist!"));
                            goto setErr_filter;
                        }
                        if (((((*((unsigned int*)ipentrytmp.ipAddrEnd))>=(*((unsigned int*)ipEntry.ipAddrEnd)))&&
                            ((*((unsigned int*)ipentrytmp.ipAddr))<=(*((unsigned int*)ipEntry.ipAddrEnd))))||
                            (((*((unsigned int*)ipentrytmp.ipAddrEnd))>=(*((unsigned int*)ipEntry.ipAddr)))&&
                            ((*((unsigned int*)ipentrytmp.ipAddr))<=(*((unsigned int*)ipEntry.ipAddr)))))&&
                            ((ipentrytmp.protoType==ipEntry.protoType)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_TCP)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_UDP)||
                            (ipentrytmp.protoType==PROTO_TCP&&ipEntry.protoType==PROTO_BOTH)||
                            (ipentrytmp.protoType==PROTO_UDP&&ipEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("ip address overlap!"));
                            goto setErr_filter;
                        }
                        if (((((*((unsigned int*)ipEntry.ipAddrEnd))>=(*((unsigned int*)ipentrytmp.ipAddrEnd)))&&
                        ((*((unsigned int*)ipEntry.ipAddr))<=(*((unsigned int*)ipentrytmp.ipAddrEnd))))||
                        (((*((unsigned int*)ipEntry.ipAddrEnd))>=(*((unsigned int*)ipentrytmp.ipAddr)))&&
                        ((*((unsigned int*)ipEntry.ipAddr))<=(*((unsigned int*)ipentrytmp.ipAddr)))))&&
                        ((ipentrytmp.protoType==ipEntry.protoType)||
                        (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_TCP)||
                        (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_UDP)||
                        (ipentrytmp.protoType==PROTO_TCP&&ipEntry.protoType==PROTO_BOTH)||
                        (ipentrytmp.protoType==PROTO_UDP&&ipEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("ip address overlap!"));
                            goto setErr_filter;
                        }
                    }
                    else
                    {
                        if ((((*((unsigned int*)ipentrytmp.ipAddrEnd))>=(*((unsigned int*)ipEntry.ipAddr)))&&
                            ((*((unsigned int*)ipentrytmp.ipAddr))<=(*((unsigned int*)ipEntry.ipAddr))))||
                            (((*((unsigned int*)ipentrytmp.ipAddrEnd))==(*((unsigned int*)ipEntry.ipAddr)))||
                            ((*((unsigned int*)ipentrytmp.ipAddr))==(*((unsigned int*)ipEntry.ipAddr))))&&
                            ((ipentrytmp.protoType==ipEntry.protoType)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_TCP)||
                            (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_UDP)||
                            (ipentrytmp.protoType==PROTO_TCP&&ipEntry.protoType==PROTO_BOTH)||
                            (ipentrytmp.protoType==PROTO_UDP&&ipEntry.protoType==PROTO_BOTH)))
                        {
                            strcpy(tmpBuf, ("ip address overlap!"));
                            goto setErr_filter;
                        }
                    }
                    #else
                    if (((*((unsigned int*)ipentrytmp.ipAddr)) == (*((unsigned int*)ipEntry.ipAddr)))&&
                        ((ipentrytmp.protoType==ipEntry.protoType)||
                        (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_TCP)||
                        (ipentrytmp.protoType==PROTO_BOTH&&ipEntry.protoType==PROTO_UDP)||
                        (ipentrytmp.protoType==PROTO_TCP&&ipEntry.protoType==PROTO_BOTH)||
                        (ipentrytmp.protoType==PROTO_UDP&&ipEntry.protoType==PROTO_BOTH)))
                    {
                        strcpy(tmpBuf, ("rule already exist!"));
                        goto setErr_filter;
                    }
                    #endif
                }
            }
        }
        
		if (strAddIp[0] || strAddPort[0] || strAddMac[0] || strAddUrl[0]) {
			if ( strComment[0] ) {
				if (strlen(strComment) > COMMENT_LEN-1) {
					strcpy(tmpBuf, ("Error! Comment length too long."));
					goto setErr_filter;
				}
				if (strAddIp[0])
					strcpy((char *)ipEntry.comment, strComment);
				else if (strAddPort[0])
					strcpy((char *)portEntry.comment, strComment);
				else if (strAddMac[0])
					strcpy((char *)macEntry.comment, strComment);
			}
	
			if ( !apmib_get(num_id, (void *)&entryNum)) {
				strcpy(tmpBuf, ("Get entry number error!"));
				goto setErr_filter;
			}
			if (strAddUrl[0])
			{
				if ( (entryNum + 1) > MAX_URLFILTER_NUM) {
					strcpy(tmpBuf, ("Cannot add new URL entry because table is full!"));
					goto setErr_filter;
				}
			}
			else
			{
				if ( (entryNum + 1) > MAX_FILTER_NUM) {
					strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
					goto setErr_filter;
				}
			}		
			// set to MIB. try to delete it first to avoid duplicate case
			apmib_set(del_id, pEntry);
			if ( apmib_set(add_id, pEntry) == 0) {
				strcpy(tmpBuf, ("Add table entry error!"));
				goto setErr_filter;
			}
		}
	
	
		/* Delete entry */
		if (strDelPort[0] || strDelIp[0] || strDelMac[0] || strDelUrl[0]) {
			if ( !apmib_get(num_id, (void *)&entryNum)) {
				strcpy(tmpBuf, ("Get entry number error!"));
				goto setErr_filter;
			}
			for (i=entryNum; i>0; i--) {
				snprintf(tmpBuf, 20, "select%d", i);
	
				strVal = get_cstream_var(wp, len,tmpBuf, "");
				if ( !strcmp(strVal, "ON") ) {
	
					*((char *)pEntry) = (char)i;
					if ( !apmib_get(get_id, pEntry)) {
						strcpy(tmpBuf, ("Get table entry error!"));
						goto setErr_filter;
					}
					if ( !apmib_set(del_id, pEntry)) {
						strcpy(tmpBuf, ("Delete table entry error!"));
						goto setErr_filter;
					}
				}
			}
		}
	
		/* Delete all entry */
		if ( strDelAllPort[0] || strDelAllIp[0] || strDelAllMac[0] || strDelAllUrl[0]){
			if (!apmib_set(delall_id, pEntry)){
				strcpy(tmpBuf, ("Delete all table error!"));
				goto setErr_filter;
			}
		}
			
	setOk_filter:
		apmib_update(CURRENT_SETTING);
        
		submitUrl = get_cstream_var(wp,len, "submit-url", "");   // hidden page

		if(MUST_REBOOT != 1)
		{
			kick_event(FIREWARE_EVENT);
			if(submitUrl)
				send_redirect_perm( submitUrl);
		}
		else
			OK_MSG(submitUrl);

		save_cs_to_file();

		return;
	
	
	setErr_filter:
		ERR_MSG(tmpBuf);
		//diag_printf("set firewall error:%s\n",tmpBuf);
		return;
}
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
extern void set_portFwd(void);
#endif

#ifdef QOS_BY_BANDWIDTH
static const char _md1[] = "ip_qos_quarant_minband", _md2[] = "ip_qos_restrict_maxband", _mdwfq[] = "wfq", _mdsp[] = "sp";
static const char s4dashes[] = "----";

#define QOS_BW_CHECK_FAIL				-1
#define QOS_BW_NOT_OVERSIZE			0
#define QOS_UPLINK_BW_OVERSIZE		0x1
#define QOS_DOWNLINK_BW_OVERSIZE		0x2
#define QOS_BOTHLINK_BW_OVERSIZE		0x3
#define udp	1
#define tcp 2
#define all 3
// Only for "Guaranteed minimum bandwidth",
// to check current uplink or downlink bandwidth added uplink & downlink bandwidth at previous rules
// whether larger than totoal uplink or downlink bandwidth
/////////////////////////////////////////////////////////////////////////////
int ipQosList(int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	IPQOS_T entry;
	char	*mode, bandwidth[10], bandwidth_downlink[10],weight[10], prior[10];
	char	mac[20], ip[40], protocol[10],*tmpStr;
	char 	tmpbuf[TMP_BUF_SIZE]="";
	int qos_mode=0;
	int entry_qos_mode=0;
	
	if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	apmib_get( MIB_QOS_MODE, (void *)&qos_mode);
	//diag_printf("qos_mode:%d [%s]:[%d].\n",qos_mode,__FUNCTION__,__LINE__);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_localaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_macaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_mode)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_valid)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_upband)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
      	
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_downband)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent); 	

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_weight)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent); 

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_priority)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent); 
	
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(ip_qos_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	memset(&entry, '\0', sizeof(entry));
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry))
			return -1;
		
		entry_qos_mode=0;
		if ( (entry.mode & QOS_RESTRICT_IP)  != 0) {
#ifdef IPCONFLICT_UPDATE_FIREWALL
//		diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
		*((unsigned int*)(entry.local_ip_start))=get_conflict_ip(*((unsigned int*)(entry.local_ip_start)));
		*((unsigned int*)(entry.local_ip_end))=get_conflict_ip(*((unsigned int*)(entry.local_ip_end)));
//		diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
#endif

			if(((struct in_addr *)entry.local_ip_start)->s_addr)
				tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
			else
				tmpStr ="any";
		//	strcpy(mac, tmpStr);
		//	tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_end));
			if(entry.protocol == udp)
				strcpy(protocol,"udp");
			else if(entry.protocol == tcp)
				strcpy(protocol,"tcp");
			else
				strcpy(protocol,"both");
			
			if((entry.local_port_start)&&(entry.local_port_end))
			{
				if(entry.local_port_start==entry.local_port_end)
					sprintf(ip, "%s:[%d] %s", tmpStr,entry.local_port_start,protocol);
				else	
					sprintf(ip, "%s:[%d-%d] %s", tmpStr,entry.local_port_start,entry.local_port_end,protocol);
			}
			else
				sprintf(ip, "%s %s", tmpStr,protocol);
			
			strcpy(mac, s4dashes);
		}
		else if ( (entry.mode & QOS_RESTRICT_MAC)  != 0) {
			if((entry.mac[0]==0)&&(entry.mac[1]==0)&&(entry.mac[2]==0)
				&&(entry.mac[3]==0)&&(entry.mac[4]==0)&&(entry.mac[5]==0))
			sprintf(mac,"any");
			else
			sprintf(mac, "%02x%02x%02x%02x%02x%02x",
				entry.mac[0],entry.mac[1],entry.mac[2],entry.mac[3],entry.mac[4],entry.mac[5]);
			strcpy(ip, s4dashes);
		}
		else //all
		{
			strcpy(ip, s4dashes);
			strcpy(mac, s4dashes);
		}
		
		if(entry.mode & QOS_WFQ_SCH)
		{
			/*wfq*/
			entry_qos_mode=1;
			mode = (char *)_mdwfq;
		}
		else if(entry.mode & QOS_SPRIO_SCH)
		{
			/*sp*/
			mode = (char *)_mdsp;
		}
		else
		{
			if ( (entry.mode & QOS_RESTRICT_MIN)  != 0)
				mode = (char *)_md1;
			else
				mode = (char *)_md2;
		}
		
    	if(entry.bandwidth == 0)
    		sprintf(bandwidth, "%s", "-");
		else
			snprintf(bandwidth, 10, "%ld", entry.bandwidth);

		if(entry.bandwidth_downlink == 0)
    		sprintf(bandwidth_downlink, "%s", "-");
		else
			snprintf(bandwidth_downlink, 10, "%ld", entry.bandwidth_downlink);

		if(entry.weight == 0)
			sprintf(weight,"%s","-");
		else
			snprintf(weight,10,"%d",entry.weight);


		if(entry.prior == 0)
			sprintf(prior,"%s","-");
		else
			snprintf(prior,10,"%d",entry.prior);
	
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<tr><td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",ip);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",mac);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",mode);
	
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		//diag_printf("entry_qos_mode:%d,[%s]:[%d].\n",entry_qos_mode,__FUNCTION__,__LINE__);
		
		if(qos_mode == entry_qos_mode){
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n","valid");
		}	
		else
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n","invalid");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);	
		
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",bandwidth);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",bandwidth_downlink);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",weight);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",prior);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<td align=center bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",
				i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	}
	return nBytesSent;
}
#endif

#if defined(HAVE_FIREWALL)
void formDMZ(char *postData, int len)
{
	char *submitUrl, *strSave, *strVal;
	char tmpBuf[100];
	int intVal;
	struct in_addr ipAddr, curIpAddr, curSubnet;
	unsigned long v1, v2, v3;

	strSave = get_cstream_var(postData, len,("save"), "");

	if (strSave[0]) {
		strVal = get_cstream_var(postData,len,("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set(MIB_DMZ_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_dmz;
		}

		strVal = get_cstream_var(postData,len,("ip"), "");
		if (!strVal[0]) {
			goto setOk_dmz;
		}
		inet_aton(strVal, &ipAddr);
#if 0
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
		getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

		v1 = *((unsigned long *)&ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);
		if (v1) {
			if ( (v1 & v3) != (v2 & v3) ) {
				strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
				goto setErr_dmz;
			}
		}
#endif
		if ( apmib_set(MIB_DMZ_HOST, (void *)&ipAddr) == 0) {
			strcpy(tmpBuf, ("Set DMZ MIB error!"));
			goto setErr_dmz;
		}
	}

setOk_dmz:
	apmib_update_web(CURRENT_SETTING);


	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
	if(MUST_REBOOT != 1)
	{
		kick_event(FIREWARE_EVENT);
		if(submitUrl)
			send_redirect_perm( submitUrl);
	}
	else
		OK_MSG(submitUrl);

	save_cs_to_file();

  	return;

setErr_dmz:
	ERR_MSG(tmpBuf);
}
/////////////////////////////////////////////////////////////////////////////
void formPortFw(char *postData, int len)
{
	char *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
	char *strIp, *strFrom, *strTo, *strComment;
	char tmpBuf[100];
	int entryNum, intVal, i;
	PORTFW_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3;

	strAddPort = get_cstream_var(postData,len, ("addPortFw"), "");
	strDelPort = get_cstream_var(postData, len, ("deleteSelPortFw"), "");
	strDelAllPort = get_cstream_var(postData,len, ("deleteAllPortFw"), "");

	memset(&entry, '\0', sizeof(entry));

	/* Add new port-forwarding table */
	if (strAddPort[0]) {
		strVal = get_cstream_var(postData, len,("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_PORTFW_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_portfw;
		}

		strIp = get_cstream_var(postData, len,("ip"), "");
		strFrom = get_cstream_var(postData,len, ("fromPort"), "");
		strTo = get_cstream_var(postData,len, ("toPort"), "");
		strComment = get_cstream_var(postData,len, ("comment"), "");

		if (!strIp[0] && !strFrom[0] && !strTo[0] && !strComment[0])
			goto setOk_portfw;

		if (!strIp[0]) {
			strcpy(tmpBuf, ("Error! No ip address to set."));
			goto setErr_portfw;
		}

		inet_aton(strIp, (struct in_addr *)&entry.ipAddr);
#if 0
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
		getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
			goto setErr_portfw;
		}
#endif

		if ( !strFrom[0] ) { // if port-forwarding, from port must exist
			strcpy(tmpBuf, ("Error! No from-port value to be set."));
			goto setErr_portfw;
		}
		if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of from-port."));
			goto setErr_portfw;
		}
		entry.fromPort = (unsigned short)intVal;

		if ( !strTo[0] )
			entry.toPort = entry.fromPort;
		else {
			if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of to-port."));
				goto setErr_portfw;
			}
		}
		entry.toPort = (unsigned short)intVal;

		if ( entry.fromPort  > entry.toPort ) {
			strcpy(tmpBuf, ("Error! Invalid port range."));
			goto setErr_portfw;
		}

		strVal = get_cstream_var(postData,len, ("protocol"), "");
		if (strVal[0]) {
			if ( strVal[0] == '0' )
				entry.protoType = PROTO_BOTH;
			else if ( strVal[0] == '1' )
				entry.protoType = PROTO_TCP;
			else if ( strVal[0] == '2' )
				entry.protoType = PROTO_UDP;
			else {
				strcpy(tmpBuf, ("Error! Invalid protocol type."));
				goto setErr_portfw;
			}
		}
		else {
			strcpy(tmpBuf, ("Error! Protocol type cannot be empty."));
			goto setErr_portfw;
		}

		if ( strComment[0] ) {
			if (strlen(strComment) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_portfw;
			}
			strcpy((char *)entry.comment, strComment);
		}
		if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_portfw;
		}

		if ( (entryNum + 1) > MAX_FILTER_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_portfw;
		}

		// Check if there is any port overlapped
		if (strAddPort[0]) {
    		for (i=1; i<=entryNum; i++) {
    			PORTFW_T checkEntry;
    			*((char *)&checkEntry) = (char)i;
    			if ( !apmib_get(MIB_PORTFW_TBL, (void *)&checkEntry)) {
    				strcpy(tmpBuf, ("Get table entry error!"));
    				goto setErr_portfw;
    			}
    			if ( ( (entry.fromPort <= checkEntry.fromPort &&
    					entry.toPort >= checkEntry.fromPort) ||
    			       (entry.fromPort >= checkEntry.fromPort &&
    				entry.fromPort <= checkEntry.toPort)
    			     )&&
    			       (entry.protoType & checkEntry.protoType) ) {
    				strcpy(tmpBuf, ("Setting port range has overlapped with used port numbers!"));
    				goto setErr_portfw;
    			}
    		}
      }
		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_PORTFW_DEL, (void *)&entry);
		if ( apmib_set(MIB_PORTFW_ADD, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_portfw;
		}
	}

	/* Delete entry */
	if (strDelPort[0]) {
		if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_portfw;
		}

		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = get_cstream_var(postData,len, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_PORTFW_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_portfw;
				}
				if ( !apmib_set(MIB_PORTFW_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_portfw;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllPort[0]) {
		if ( !apmib_set(MIB_PORTFW_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_portfw;
		}
	}

setOk_portfw:
	apmib_update_web(CURRENT_SETTING);
	
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
	if(MUST_REBOOT != 1)
	{
		kick_event(FIREWARE_EVENT);
		if(submitUrl)
			send_redirect_perm( submitUrl);
	}
	else
		OK_MSG(submitUrl);

	save_cs_to_file();

  	return;

setErr_portfw:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int portFwList(int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	PORTFW_T entry;
	char	*type, portRange[20], *ip;
	char 	tmpbuf[TMP_BUF_SIZE]="";

	if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_localipaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_proto)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_portrange)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_comm)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_PORTFW_TBL, (void *)&entry))
			return -1;
#ifdef IPCONFLICT_UPDATE_FIREWALL               
		*((unsigned int*)(entry.ipAddr))=get_conflict_ip(*((unsigned int*)(entry.ipAddr)));
#endif

		ip = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = "----";

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		if ( entry.fromPort == 0)
			strcpy(portRange, "----");
		else if ( entry.fromPort == entry.toPort )
			snprintf(portRange, 20, "%d", entry.fromPort);
		else
			snprintf(portRange, 20, "%d-%d", entry.fromPort, entry.toPort);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",ip);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",type);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",portRange);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.comment);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
			
	}
	return nBytesSent;
}
#endif

#ifdef QOS_BY_BANDWIDTH
/////////////////////////////////////////////////////////////////////////////
void formIpQoS(char *wp, int len)
{
	char *submitUrl, *strAdd, *strDel, *strVal, *strDelAll;
	char *strIpStart, *strIpEnd, *strMac, *strBandwidth, *strBandwidth_downlink;
	char *strPort,*strPort2,*strProtocol,*strWeight, *strPrior;
	char tmpBuf[100];
	int entryNum, intVal, i;
	IPQOS_T entry,entrytmp;
	unsigned long totalUplinkBw, totalDownlinkBw;
	int ret;
	int j=0;
	unsigned int ip1=0,ip2=0;
	unsigned char mac[6] = {0};
	struct in_addr ips,ipe;
	unsigned int ports=0,porte=0;
	unsigned int proto=0;
	int upLinkBw,dowLinkBw,weight=0;
	
	int prior = 0;
	unsigned long fsUplinkBw, fsDownLinkBw;
	
	int qosType=0;
	int entryQosType=0;
	int wfqEntryNum=0,wfqEntryNum2=0;
//displayPostDate(wp->post_data);
	
	strAdd = get_cstream_var(wp, len, ("addQos"), "");
	strDel = get_cstream_var(wp, len, ("deleteSel"), "");
	strDelAll = get_cstream_var(wp, len, ("deleteAll"), "");

	memset(&entry, '\0', sizeof(entry));
	memset(&ips,0,sizeof(ips));
	if (strAdd[0]) {
		strVal = get_cstream_var(wp, len,("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_QOS_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr;
		}
		
		if (intVal == 0)
			goto setOk;
		
		strVal = get_cstream_var(wp, len,("qosType"), "");
		if ( !strcmp(strVal, "wfq"))
			intVal = 1;
		else
			intVal = 0;
		
		qosType =intVal;
		//diag_printf("qosType:%d,[%s]:[%d].\n",qosType,__FUNCTION__,__LINE__);
		if ( apmib_set( MIB_QOS_MODE, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr;
		}

		strVal = get_cstream_var(wp, len, ("automaticUplinkSpeed"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set mib error!"));
			goto setErr;
		}

		if (intVal == 0) {
			strVal = get_cstream_var(wp, len, ("manualUplinkSpeed"), "");
			string_to_dec(strVal, &intVal);
			if ( apmib_set( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set mib error!"));
				goto setErr;
			}
			totalUplinkBw=intVal;
		}
		else{
			// Auto uplink speed
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_8367R)
			totalUplinkBw=1024000;		// 1000Mbps
#else
			totalUplinkBw=102400;		// 100Mbps
#endif
		}

		strVal = get_cstream_var(wp, len, ("automaticDownlinkSpeed"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set( MIB_QOS_AUTO_DOWNLINK_SPEED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set mib error!"));
			goto setErr;
		}

		if (intVal == 0) {
			strVal = get_cstream_var(wp, len, ("manualDownlinkSpeed"), "");
			string_to_dec(strVal, &intVal);
			if ( apmib_set( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set mib error!"));
				goto setErr;
			}
			totalDownlinkBw=intVal;
		}
		else{
			// Auto uplink speed
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_8367R)
			totalDownlinkBw=1024000;		// 1000Mbps
#else
			totalDownlinkBw=102400;		// 100Mbps
#endif
		}

		strIpStart = get_cstream_var(wp, len, ("ipStart"), "");
		//strIpEnd = get_cstream_var(wp, len, ("ipEnd"), "");
		strProtocol = get_cstream_var(wp, len, ("protocol"), "");
		strPort = get_cstream_var(wp, len, ("portStart"), "");
		strPort2 = get_cstream_var(wp, len, ("portEnd"), "");
		strMac = get_cstream_var(wp, len, ("mac"), "");
		strBandwidth = get_cstream_var(wp, len,("bandwidth"), "");
		strBandwidth_downlink = get_cstream_var(wp, len,("bandwidth_downlink"), "");
		strWeight = get_cstream_var(wp, len,("qos_weight"), ""); 
		strPrior = get_cstream_var(wp, len,("qos_priority"), "");
		strVal = get_cstream_var(wp, len,("addressType"), "");
		string_to_dec(strVal, &intVal);		
		string_to_dec(strBandwidth, &upLinkBw);
		string_to_dec(strBandwidth_downlink, &dowLinkBw);
		string_to_dec(strWeight, &weight);
		string_to_dec(strPrior, &prior);

		if(upLinkBw > MAX_BAND_WIDTH )	
		{
			strcpy(tmpBuf, ("Up link bandwidth exceed the max range"));
			goto setErr;
		}

		if(upLinkBw > totalUplinkBw)
		{
			strcpy(tmpBuf, ("rule up link bandwidth exceed pipe up bandwidth range"));
			goto setErr;
		}
		
		if(dowLinkBw > MAX_BAND_WIDTH )	
		{
			strcpy(tmpBuf, ("Down link bandwidth exceed the max range"));
			goto setErr;
		}

		if(dowLinkBw > totalDownlinkBw)
		{
			strcpy(tmpBuf, ("rule down link bandwidth exceed pipe bandwidth range"));
			goto setErr;
		}
		
		if (intVal == 0) { // IP
		#if 1
			inet_aton(strIpStart, &ips);
			string_to_dec(strPort, &ports);
			string_to_dec(strPort2, &porte);
			proto= atoi(strProtocol);
			
			//inet_aton(strIpEnd, &ipe);
			//printf("ips:%x[%s]:[%d].\n",ips.s_addr,__FUNCTION__,__LINE__);
			if(upLinkBw||dowLinkBw||totalUplinkBw||totalDownlinkBw)
			{
				apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum);
				for(j=1;j<=entryNum;j++)
				{
					*((char *)&entrytmp) = (char)j;
					if ( apmib_get(MIB_QOS_RULE_TBL, (void *)&entrytmp))
					{
						
						if(entrytmp.mode&QOS_WFQ_SCH){
							entryQosType=1;
							if(entrytmp.weight)
								wfqEntryNum++;
							
						}	
						else
							entryQosType=0;
												
						if((entryQosType == qosType)&&((qosType&&weight)||(qosType==0 && prior)))
						if(entrytmp.mode & QOS_RESTRICT_IP)
						{
							ip1=(*((struct in_addr *)entrytmp.local_ip_start)).s_addr;
							//ip2=(*((struct in_addr *)entry.local_ip_end)).s_addr;
							//printf("ip1:%x,[%s]:[%d].\n",ip1,__FUNCTION__,__LINE__);
						
							if((ips.s_addr == ip1)||(ips.s_addr==0)||(ip1==0))
							{
								/*not need match port*/	
								if(ports==0||entrytmp.local_port_start==0)
								{
									if((proto==entrytmp.protocol)
										||(proto==3)||(entrytmp.protocol==3))
									{
										strcpy(tmpBuf, (" ip address conflict!"));
										goto setErr;
									}
								}
								else if(((ports >= entrytmp.local_port_start) && (ports <= entrytmp.local_port_end))
								||((porte >= entrytmp.local_port_start) && (porte <=entrytmp.local_port_end))
								||((ports < entrytmp.local_port_start) && (porte > entrytmp.local_port_end)))
								{
									if((proto==entrytmp.protocol)
										||(proto==3)||(entrytmp.protocol==3))
									{
										strcpy(tmpBuf, (" port setting conflict!"));
										goto setErr;
									}
								}
							}
							
							
						}
					}
				}
			}
			#endif
			inet_aton(strIpStart, (struct in_addr *)&entry.local_ip_start);
			entry.protocol = atoi(strProtocol);
			string_to_dec(strPort, &entry.local_port_start);
			string_to_dec(strPort2, &entry.local_port_end);
	//		inet_aton(strIpEnd, (struct in_addr *)&entry.local_ip_end);
			entry.mode = QOS_RESTRICT_IP;
		}
		else if (intVal == 1) { //MAC
			string_to_hex(strMac, mac, 12);
			//diag_printf("mac:%02x%02x%02x%02x%02x%02x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum);
			
			for(j=1;j<=entryNum;j++)
			{
				*((char *)&entrytmp) = (char)j;
				if ( apmib_get(MIB_QOS_RULE_TBL, (void *)&entrytmp))
				{
					if(entrytmp.mode&QOS_WFQ_SCH){
						entryQosType=1;
						if(entrytmp.weight)
							wfqEntryNum++;
						
					}	
					else
						entryQosType=0;
					if((entryQosType == qosType)&&((qosType&&weight)||(qosType==0 && prior)))
					if(entrytmp.mode & QOS_RESTRICT_MAC)
					{
						/*printf("[%s]:[%d]entry.mac:%02x%02x%02x%02x%02x%02x\n",__FUNCTION__,__LINE__,
						entry.mac[0],entry.mac[1],entry.mac[2],entry.mac[3],entry.mac[4],entry.mac[5]);*/
						if(((entrytmp.mac[0]==mac[0])&&(entrytmp.mac[1]==mac[1])
						&&(entrytmp.mac[2]==mac[2])&&(entrytmp.mac[3]==mac[3])
						&&(entrytmp.mac[4]==mac[4])&&(entrytmp.mac[5]==mac[5]))
						||((entrytmp.mac[0]==0)&&(entrytmp.mac[1]==0)
						&&(entrytmp.mac[2]==0)&&(entrytmp.mac[3]==0)
						&&(entrytmp.mac[4]==0)&&(entrytmp.mac[5]==0)))
						{
							strcpy(tmpBuf, (" mac address conflict!"));
							goto setErr;
						}
						
					}
				}
			}
			if(!((0==mac[0])&&(0==mac[1])&&(0==mac[2])
				&&(0==mac[3])&&(0==mac[4])&&(0==mac[5])))
			{
				if (!string_to_hex(strMac, entry.mac, 12))
				{
					//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
					strcpy(tmpBuf, ("MAC input fail!"));
					goto setErr;
				}
			}	
			entry.mode = QOS_RESTRICT_MAC;
		}
		else
		{
			entry.mode = QOS_RESTRICT_ALL;
		}

		strVal = get_cstream_var(wp, len,("mode"), "");
		if (strVal[0] == '1')
			entry.mode |= QOS_RESTRICT_MIN;
		else
			entry.mode |= QOS_RESTRICT_MAX;
		
		if(qosType){
			entry.mode |= QOS_WFQ_SCH;
			//diag_printf("qosType:%d,%x,[%s]:[%d].\n",qosType,entry.mode,__FUNCTION__,__LINE__);
		}
		else
			entry.mode |= QOS_SPRIO_SCH;
		
		string_to_dec(strBandwidth, &intVal);
		entry.bandwidth = (unsigned long)intVal;

		string_to_dec(strBandwidth_downlink, &intVal);
		entry.bandwidth_downlink = (unsigned long)intVal;
		if(strWeight){
			string_to_dec(strWeight, &intVal);
			entry.weight = (unsigned long)intVal;
		}
		else
			entry.weight = 1;
		
		if(strPrior){
			string_to_dec(strPrior, &intVal);
			entry.prior = (unsigned long)intVal;
		}
		else
		{
			entry.prior = 0;
		}
					
		entry.enabled = 1;
		if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr;
		}

		if ( (entryNum + 1) > MAX_QOS_RULE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr;
		}
		//diag_printf("wfqEntryNum:%d,%d,%d,[%s]:[%d].\n",wfqEntryNum,wfqEntryNum2,weight,__FUNCTION__,__LINE__);
		// set to MIB. try to delete it first to avoid duplicate case
	
		apmib_set(MIB_QOS_DEL, (void *)&entry);
		if(upLinkBw||dowLinkBw||totalUplinkBw||totalDownlinkBw)
		{	
			//if((wfqEntryNum && weight)||(wfqEntryNum==0))
			if(weight || prior)
			if ( apmib_set(MIB_QOS_ADD, (void *)&entry) == 0) {
				strcpy(tmpBuf, ("Add table entry error!"));
				goto setErr;
			}
		}
	}

	/* Delete entry */
	if (strDel[0]) {
		if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr;
		}

		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = get_cstream_var(wp, len, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr;
				}
				if ( !apmib_set(MIB_QOS_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAll[0]) {
		if ( !apmib_set(MIB_QOS_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr;
		}
	}

setOk:
	apmib_update(CURRENT_SETTING);
	submitUrl = get_cstream_var(wp,len, "submit-url", "");   // hidden page
	if(MUST_REBOOT != 1)
	{
		kick_event(FIREWARE_EVENT);
		if(submitUrl)
			send_redirect_perm( submitUrl);
	}
	else
		OK_MSG(submitUrl);

	save_cs_to_file();

  	return;

setErr:
	ERR_MSG(tmpBuf);

}
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
#ifndef HAVE_NOETH
extern void set_vlanInfo(void);
extern void rtl_reinitByVlanSetting(void);
#endif
char checkVlanExist(unsigned short vid);
char delExistVlan(unsigned short vlanid);



char delVlanBind(unsigned short vlanid)
{	
	VLAN_NETIF_BIND_T vlanNetifBind={0};
	int i=0,entryNum=0;
	if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)&vlanNetifBind) = (char)i;
		if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL, (void *)&vlanNetifBind))
		{
			printf("%s:%d \n",__FUNCTION__,__LINE__);
			return -1;
		}
		if(vlanid == vlanNetifBind.vlanId)
		{
			if(!apmib_set(MIB_VLAN_NETIF_BIND_DEL, (void *)&vlanNetifBind))
			{
				printf("%s:%d \n",__FUNCTION__,__LINE__);
				return -1;
			}
			i--;
			entryNum--;
		}			
	}
	return 0;
}
char modExistVlan( VLAN_T* pVlan)
{
	VLAN_T VlanEntry[2]={0};
	VLAN_NETIF_BIND_T vlanNetifBind={0};
	int i=0,entryNum=0;
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)VlanEntry) = (char)i;
		if ( !apmib_get(MIB_VLAN_TBL, (void *)VlanEntry))
		{
			fprintf(stderr, "Get MIB_VLAN_TBL error!\n");
			return -1;	
		}
		//printf("%s:%d id=%d\n",__FILE__,__LINE__,VlanEntry[0].vlanId);
		if(VlanEntry[0].vlanId==pVlan->vlanId)
		{
			memcpy(&(VlanEntry[1]),pVlan,sizeof(VLAN_T));
			if ( !apmib_set(MIB_VLAN_MOD, (void *)VlanEntry))
			{
				fprintf(stderr, "Set MIB_VLAN_MOD error!\n");
				return -1;
			}
			if(delVlanBind(pVlan->vlanId)<0)
			{
				fprintf(stderr, "delVlanBind error! vlanId=%d\n",pVlan->vlanId);
				return -1;
			}
			return 1;
		}
	}
	//printf("%s:%d id=%d\n",__FILE__,__LINE__,pVlan->vlanId);
	return 0;//vlan not exist
}


void formVlanAdd(char *postData, int len)
{
	char * strVal=NULL;
	char* errMsgBuf=NULL;
	int enabled=0,num=0,netIfNum=0,i=0,value=0;
	char nameBuf[64]={0};
	#if defined(HAVE_SYSTEM_REINIT) 	
	DHCP_T wan_mode;
	#endif	
	errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);
	if(!errMsgBuf)
	{
		printf("malloc fail!\n");
		return;
	}
	if(handle_fmGetValAndSetChkbox("vlan_enable",MIB_VLAN_ENABLED,&enabled,postData,len,errMsgBuf)<0)		
		goto setErr_VLANADD;

	if(enabled)
	{
		VLAN_NETIF_BIND_T vlanNetifBind={0};		
		VLAN_T vlan={0};
		NETIFACE_T netifEntry={0};
		strVal=get_cstream_var(postData, len,"vlanId", "");
		if(!strVal[0] || !strcmp(strVal,"0"))
		{//only enable vlan
			goto setOK_VLANADD;
		}
		vlan.vlanId=atoi(strVal);
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        strVal = get_cstream_var(postData, len,"ForwardRule", "");
        vlan.ForwardRule = atoi(strVal);
        #endif
		if((value=modExistVlan(&vlan))<0)
		{
			printf("modify exist vlan fail! %d\n",vlan.vlanId);
			return;
		}
		//printf("%s:%d value=%d vlan=%d\n",__FILE__,__LINE__,value,vlan.vlanId);
		if(!value && !apmib_set(MIB_VLAN_ADD,(void*)&vlan))
		{//vlan not exist,add it
			strcpy(errMsgBuf,"set MIB_VLAN_ADD fail!");
			goto setErr_VLANADD;
		}
	//add vlanNetBind
		
		if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&netIfNum)) 
		{
	  		fprintf(stderr, "Get table entry error!\n");
			return;
		}	
		for(i=1;i<=netIfNum;i++)
		{
			*((char *)&netifEntry) = (char)i;
			if ( !apmib_get(MIB_NETIFACE_TBL, (void *)&netifEntry))
				return;
			
			sprintf(nameBuf,"tagTbl_radio%d",netifEntry.netifId);
			strVal=get_cstream_var(postData, len,nameBuf, "");
			value=atoi(strVal);
			if(value==1)//untaged
			{
				vlanNetifBind.tagged=0;
			}else if(value==2)//taged
			{
				vlanNetifBind.tagged=1;
			}else
				continue;
			vlanNetifBind.vlanId=vlan.vlanId;
			vlanNetifBind.netifId=netifEntry.netifId;
			if(!apmib_set(MIB_VLAN_NETIF_BIND_ADD,(void*)&vlanNetifBind))
			{
				strcpy(errMsgBuf,"set MIB_VLAN_NETIF_BIND_ADD fail!");
				goto setErr_VLANADD;
			}
		}
		
	}
setOK_VLANADD:
	if(checkIfPvidVlanBind()<0)
	{
		strcpy(errMsgBuf,"checkIfPvidVlanBind fail!");
		goto setErr_VLANADD;
	}
	value=0;
	apmib_set(MIB_CURRENT_VLAN_ID, (void *)&value);
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
	apmib_set(MIB_CURRENT_VLAN_ID, (void *)&value);//how to display when vlanid=0
    #endif
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	apmib_update_web(CURRENT_SETTING);	
#if defined(HAVE_SYSTEM_REINIT) 	
#ifndef HAVE_NOETH
	set_vlanInfo();
	rtl_reinitByVlanSetting();
#endif	
	apmib_get(MIB_WAN_DHCP,&wan_mode);
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_mode);
	//diag_printf("%s %d wan_mode=%d \n", __FUNCTION__, __LINE__, wan_mode);
#endif
	strVal=get_cstream_var(postData, len,"submit-url", "");
	if (strVal[0])
	{
		#if defined(HAVE_SYSTEM_REINIT) 	
		wait_redirect("Apply Changes",10,strVal);
		kick_reinit_m(SYS_WAN_M);
		#else	
		OK_MSG(strVal);
		#endif
	}
	#if 0
    if (MUST_REBOOT != 1)
    {
    	if (strVal[0])
    		send_redirect_perm(strVal);
    }
    else
    	OK_MSG(strVal);
    #endif

	save_cs_to_file();

	return;
setErr_VLANADD:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return;
}
void formVlan(char *postData, int len)
{
	char * strVal=NULL , *submitUrl=NULL;
	VLAN_T vlan={0};
	int vlanNum=0,i=0,value=0,netifNum=0;
	char nameBuf[64]={0};	 
	#if defined(HAVE_SYSTEM_REINIT) 	
	DHCP_T wan_mode;
	int modifyVlanNoReinit = 0;
	#endif

	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&vlanNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return ;
	}
	
	for(i=1;i<=vlanNum;i++)
	{
		*((char *)&vlan) = (char)i;
		if ( !apmib_get(MIB_VLAN_TBL, (void *)&vlan))
			return ;
		sprintf(nameBuf,"modifyVlan%d",vlan.vlanId);
		value=vlan.vlanId;
		strVal=get_cstream_var(postData, len,nameBuf, "");
		if(strVal[0])
		{
			if ( !apmib_set(MIB_CURRENT_VLAN_ID, (void *)&value))
				return ;
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
            value=vlan.ForwardRule;
            if ( !apmib_set(MIB_CURRENT_VLAN_FORWARD_RULE, (void *)&value))
                return ;
            #endif
			#if defined(HAVE_SYSTEM_REINIT)
			modifyVlanNoReinit = 1;
			#endif
		}
	}
	
	strVal=get_cstream_var(postData, len,"deleteSelVlanItem", "");
	if(strVal[0])
	{//del select vlan item
		for(i=1;i<=vlanNum;i++)
		{
			*((char *)&vlan) = (char)i;
			if ( !apmib_get(MIB_VLAN_TBL, (void *)&vlan))
				return ;
			sprintf(nameBuf,"vlanBindItemChb%d",vlan.vlanId);
			strVal=get_cstream_var(postData, len,nameBuf, "");
			//printf("%s:%d strVal=%s name=%s\n",__FILE__,__LINE__,strVal,nameBuf);
			if(strVal[0] && !strcmp(strVal,"ON"))
			{
				//printf("%s:%d vlanId=%d\n",__FILE__,__LINE__,vlan.vlanId);
				delExistVlan(vlan.vlanId);
				if(delVlanBind(vlan.vlanId)<0)
					return ;
				i=1;
			}
		}	
	}
	strVal=get_cstream_var(postData, len,"deleteAllVlanItem", "");
	if(strVal[0])
	{//del all vlan item
		VLAN_NETIF_BIND_T entry={0};
		if ( !apmib_set(MIB_VLAN_DELALL, (void *)&vlan)) 
		{
			fprintf(stderr, "Get table entry error!\n");
			return ;
		}
		if ( !apmib_set(MIB_VLAN_NETIF_BIND_DELALL, (void *)&entry)) 
		{
			fprintf(stderr, "Get table entry error!\n");
			return ;
		}
	}
	if(checkIfPvidVlanBind()<0)
	{
		fprintf(stderr, "checkIfPvidVlanBind fail!\n");
		return ;
	}
	apmib_update_web(CURRENT_SETTING);
#if defined(HAVE_SYSTEM_REINIT)	
#ifndef HAVE_NOETH
	set_vlanInfo();
	rtl_reinitByVlanSetting();
#endif
	apmib_get(MIB_WAN_DHCP,&wan_mode);
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_mode);
	//diag_printf("%s %d wan_mode=%d \n", __FUNCTION__, __LINE__, wan_mode);
#endif
	strVal=get_cstream_var(postData, len,"submit-url", "");
	if ((strVal[0]))
	{
		#if defined(HAVE_SYSTEM_REINIT)
		if(modifyVlanNoReinit)
			wait_redirect("Apply Changes",0,strVal);
		else
		{
			wait_redirect("Apply Changes",10,strVal);
			kick_reinit_m(SYS_WAN_M);
		}
		#else	
		OK_MSG(strVal);
		#endif
	}
	#if 0
    if (MUST_REBOOT != 1)
    {
    	if (strVal[0])
    		send_redirect_perm(strVal);
    }
    else
    	OK_MSG(strVal);	
	#endif

	save_cs_to_file();

	return;

}
void formNetIf(char *postData, int len)
{
	char *strVal=NULL;
	int netifNum=0,i=0;
	char nameBuf[64]={0};
#if defined(HAVE_SYSTEM_REINIT) 	
	 DHCP_T wan_mode;
#endif

	strVal=get_cstream_var(postData, len,"applyNetIf", "");
	if(strVal[0])
	{
		NETIFACE_T netifEntry[2]={0};
		if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&netifNum)) 
		{
			fprintf(stderr, "Get table entry error!\n");
			return -1;
		}
		for(i=1;i<=netifNum;i++)
		{
			*((char *)netifEntry) = (char)i;
			if ( !apmib_get(MIB_NETIFACE_TBL, (void *)netifEntry))
				return -1;				
			memcpy(&(netifEntry[1]),&(netifEntry[0]),sizeof(NETIFACE_T));
			
			sprintf(nameBuf,"Pvid%d",netifEntry[0].netifId);
			strVal=get_cstream_var(postData, len,nameBuf, "");
			if(strVal[0])
			{
				netifEntry[1].netifPvid=atoi(strVal);
			}
			if(netifEntry[0].netifPvid!=netifEntry[1].netifPvid)
			{
				if(delVlanBindEntry(netifEntry[0].netifPvid,netifEntry[0].netifId)<0)
					return -1;
			}
			
			sprintf(nameBuf,"defPri%d",netifEntry[0].netifId);
			strVal=get_cstream_var(postData, len,nameBuf, "");
			if(strVal[0])
			{
//				printf("%s:%d strVal=%s\n",__FILE__,__LINE__,strVal);
				netifEntry[1].netifDefPriority=atoi(strVal);
			}
			
			sprintf(nameBuf,"defCfi%d",netifEntry[0].netifId);
			strVal=get_cstream_var(postData, len,nameBuf, "");
			if(strVal[0]&& !strcmp(strVal,"ON"))
			{
				netifEntry[1].netifDefCfi=1;
			}else
				netifEntry[1].netifDefCfi=0;
			if(!apmib_set(MIB_NETIFACE_MOD,(void*)netifEntry))
			{
				printf("set MIB_NETIFACE_MOD fail!");
				return -1;
			}			
		}
	}	
	if(checkIfPvidVlanBind()<0)
	{
		fprintf(stderr, "checkIfPvidVlanBind fail!\n");
		return -1;
	}
	apmib_update_web(CURRENT_SETTING);
	
#if defined(HAVE_SYSTEM_REINIT) 	
#ifndef HAVE_NOETH
	set_vlanInfo();
	rtl_reinitByVlanSetting();
#endif
	apmib_get(MIB_WAN_DHCP,&wan_mode);
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_mode);
	//diag_printf("%s %d wan_mode=%d \n", __FUNCTION__, __LINE__, wan_mode);
#endif
	strVal=get_cstream_var(postData, len,"submit-url", "");
	if (strVal[0])
	{
		#if defined(HAVE_SYSTEM_REINIT) 	
		wait_redirect("Apply Changes",10,strVal);
		kick_reinit_m(SYS_WAN_M);
		#else	
		OK_MSG(strVal);
		#endif
	}
	#if 0
    if (MUST_REBOOT != 1)
    {
    	if (strVal[0])
    		send_redirect_perm(strVal);
    }
    else
    	OK_MSG(strVal);
    #endif

	save_cs_to_file();

	return 0;
}



int showVlanTagTbl(int argc,char**argv)
{
	NETIFACE_T netifEntry={0};
	VLAN_NETIF_BIND_T bindEntry={0};
	int i=0,nBytesSent=0,entryNum=0,currentVid=0,ret=0;
	char tmpbuf[TMP_BUF_SIZE]="";
	if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	if(!apmib_get(MIB_CURRENT_VLAN_ID,(void*)&currentVid))
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for (i=1; i<=entryNum; i++) 
	{
		*((char *)&netifEntry) = (char)i;
		
		if (!apmib_get(MIB_NETIFACE_TBL,(void *)&netifEntry))
			return -1;
		if(!netifEntry.netifEnable) continue;
		if((ret=getBindEntry(currentVid,netifEntry.netifId,&bindEntry))<0)
			return -1;;
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>",netifEntry.netifName);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
		if(ret && bindEntry.tagged)
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_0\" value=2 checked></b>\
	      </td>",netifEntry.netifId,i-1);
     	 else
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_0\" value=2></b>\
	      </td>",netifEntry.netifId,i-1);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
		if(ret && !bindEntry.tagged)
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_1\" value=1 checked></b>\
	      	</td>",netifEntry.netifId,i-1);
	    else
	    	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_1\" value=1></b>\
	      	</td>",netifEntry.netifId,i-1);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		if(!ret)
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_2\" value=0 checked></b>\
	     	</td></tr>",netifEntry.netifId,i-1);
	     else
	     	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><b>\
	        <input type=\"radio\" name=\"tagTbl_radio%d\" id=\"tagTblRadioId%d_2\" value=0></b>\
	      	</td></tr>",netifEntry.netifId,i-1);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	}
	return 1;
}
int vlanList(int argc, char **argv)
{
	int	nBytesSent=0, entryNum=0,vidEntryNum=0, i=0,j=0,priority=0;
	VLAN_NETIF_BIND_T entry={0};
	VLAN_T vlanEntry={0};
	NETIFACE_T netifEntry={0};
	char	*type, *ip;
	char tmpbuf[TMP_BUF_SIZE]="";
	char *tagVlanBuf=NULL,*unTagVlanBuf=NULL;

	tagVlanBuf=(char*)malloc(TMP_BUF_SIZE);
	unTagVlanBuf=(char*)malloc(TMP_BUF_SIZE);
	if(!tagVlanBuf||!unTagVlanBuf)
	{
		printf("malloc fail!\n");
		goto vlanList_FAIL;
	}
	bzero(tagVlanBuf,TMP_BUF_SIZE);
	bzero(unTagVlanBuf,TMP_BUF_SIZE);
	if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");		
		goto vlanList_FAIL;		
	}
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&vidEntryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		goto vlanList_FAIL;
	}

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_id)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);	
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_taged)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);	
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_untaged)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_forwardrule)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
    	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_modify)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);	
    #else
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_modify)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);	    
    #endif
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_settbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	for (i=1; i<=vidEntryNum; i++) 
	{
		*((char *)&vlanEntry) = (char)i;
		
		if (!apmib_get(MIB_VLAN_TBL,(void *)&vlanEntry))
			goto vlanList_FAIL;
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</font></td>\n",vlanEntry.vlanId);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		strcpy(tagVlanBuf,"");
		strcpy(unTagVlanBuf,"");
		for(j=1;j<=entryNum;j++)
		{
			*((char *)&entry) = (char)j;
			if(!apmib_get(MIB_VLAN_NETIF_BIND_TBL,(void*)&entry))
				goto vlanList_FAIL;
			if(entry.vlanId==vlanEntry.vlanId)
			{
				if(getNetifEntry(entry.netifId,&netifEntry)<0)
				{
					printf("get netIface entry fail vid=%d\n",entry.netifId);
					goto vlanList_FAIL;
				}
				if(!netifEntry.netifEnable) continue;
				if(entry.tagged)
				{
					strcat(tagVlanBuf," ");
					strcat(tagVlanBuf,netifEntry.netifName);
				}
				else
				{
					strcat(unTagVlanBuf," ");
					strcat(unTagVlanBuf,netifEntry.netifName);
				}
			}
		}
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",tagVlanBuf);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",unTagVlanBuf);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        char fwdbuff[12] = {0};
        if (vlanEntry.ForwardRule == 1)
            sprintf(fwdbuff, "%s", "NAT");
        else if (vlanEntry.ForwardRule == 2)
            sprintf(fwdbuff, "%s", "Bridge");
        else
            sprintf(fwdbuff, "%s", "no rule");            
        nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
        "<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",fwdbuff);
        cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"submit\"  name=\"modifyVlan%d\" id=\"modifyVlanId%d\"></font></td>\n<script>document.vlanForm.modifyVlan%d.value=vlan_settbl_modify;</script>",vlanEntry.vlanId,i-1,vlanEntry.vlanId);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        #else
				nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"submit\"  name=\"modifyVlan%d\" id=\"modifyVlanId%d\"></font></td>\n<script>document.vlanForm.modifyVlan%d.value=vlan_settbl_modify;</script>",vlanEntry.vlanId,i-1,vlanEntry.vlanId);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        #endif
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"checkbox\" value=\"ON\" name=\"vlanBindItemChb%d\" id=\"vlanBindItemChb%d\"></font></td></tr>\n",vlanEntry.vlanId,i-1);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	}

	if(tagVlanBuf != NULL)	free(tagVlanBuf);
	if(unTagVlanBuf != NULL)	free(unTagVlanBuf);	
	return nBytesSent;
vlanList_FAIL:
	if(tagVlanBuf != NULL)	free(tagVlanBuf);
	if(unTagVlanBuf != NULL)	free(unTagVlanBuf);
	return -1;
}
void set_netif(int id,char* name,int pvid,NETIFACE_T* pEntry)
{
	pEntry->netifId=id;
	strcpy(pEntry->netifName,name);
	pEntry->netifPvid=pvid;
}
int netIfList(int argc,char**argv)
{
	int	nBytesSent=0, entryNum=0, i=0,j=0;

	NETIFACE_T netifEntry={0};
	char	*type, *ip;
	char tmpbuf[TMP_BUF_SIZE]="";
	char buf[64]={0};

	/*apmib_set(MIB_NETIFACE_DELALL,&netifEntry);
	set_netif(1,"eth0",1,&netifEntry);
	apmib_set(MIB_NETIFACE_ADD,&netifEntry);
	set_netif(2,"eth2",2,&netifEntry);
	apmib_set(MIB_NETIFACE_ADD,&netifEntry);
	set_netif(3,"eth3",3,&netifEntry);
	apmib_set(MIB_NETIFACE_ADD,&netifEntry);
	set_netif(4,"eth4",4,&netifEntry);
	apmib_set(MIB_NETIFACE_ADD,&netifEntry);
	*/
	if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	//printf("%s:%d entryNum=%d\n",__FILE__,__LINE__,entryNum);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_nettbl_name)</script></b></font></td>\n");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_nettbl_pvid)</script></b></font></td>\n");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_nettbl_defprio)</script></b></font></td>\n");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(vlan_nettbl_defcfi)</script></b></font></td></tr>\n");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	for(i=1;i<=entryNum;i++)
	{		
		*((char *)&netifEntry) = (char)i;
		
		if (!apmib_get(MIB_NETIFACE_TBL,(void *)&netifEntry))
			return -1;
		if(!netifEntry.netifEnable) continue;
		//printf("%s:%d netifEntry=%d %s %d %d %d\n",__FILE__,__LINE__,netifEntry.netifId,netifEntry.netifName,netifEntry.netifPvid,netifEntry.netifDefPriority,netifEntry.netifDefCfi);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n",netifEntry.netifName);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b><input type=\"text\" name=\"Pvid%d\" id=\"PvidId%d\" size=\"5\" maxlength=\"4\" value=%d></b></font></td>\n",netifEntry.netifId,i-1,netifEntry.netifPvid);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">\n\
		<select name=\"defPri%d\" id=\"defPriId%d\" value=%d>\n",netifEntry.netifId,i-1,netifEntry.netifDefPriority);
  		for(j=0;j<8;j++)
  		{
  			if(j==netifEntry.netifDefPriority)
  				sprintf(buf,"<option value=%d selected>%d</option>\n",j,j);
  			else
  				sprintf(buf,"<option value=%d>%d</option>\n",j,j);
  			nBytesSent+=strlen(buf);	
  			strcat(tmpbuf,buf);
  		}
  		nBytesSent+=strlen("</select>&nbsp;</font></td>\n");
  		strcat(tmpbuf,"</select>&nbsp;</font></td>\n");
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		if(netifEntry.netifDefCfi)
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">\
			<input type=\"checkbox\" value=\"ON\" name=\"defCfi%d\" id=\"defCfiId%d\" checked >&nbsp;</font></td></tr>\n",netifEntry.netifId,i-1);
		else
			nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">\
			<input type=\"checkbox\" value=\"ON\" name=\"defCfi%d\" id=\"defCfiId%d\" >&nbsp;</font></td></tr>\n",netifEntry.netifId,i-1);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	}	

	return nBytesSent;
}
#if 0
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
int vlanIDIsWan(int vid)
{
    if (vid < 0 || vid > 4096)
        return -1;

    NETIFACE_T netifEntry={0};
    VLAN_NETIF_BIND_T bindEntry={0};
	VLAN_T vlan={0};
	int i=0,entryNum=0, vlanNum=0, ret = 0;
	if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&vlanNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	for(i=1;i<=vlanNum;i++)
	{
		*((char *)&vlan) = (char)i;
		if ( !apmib_get(MIB_VLAN_TBL, (void *)&vlan))
			return -1;
		
		if(vlan.vlanId == vid)
			break;
	}

	if(i > vlanNum)
		return -1;
		
	if(vlan.ForwardRule == 2)
		return -1;

	for (i=1; i<=entryNum; i++) 
	{
		*((char *)&netifEntry) = (char)i;
		
		if (!apmib_get(MIB_NETIFACE_TBL,(void *)&netifEntry))
			return -1;
		if(!netifEntry.netifEnable) continue;
        if((ret=getBindEntry(vid,netifEntry.netifId,&bindEntry))<0)
            return -1;
        if (ret == 1 && (!strcmp(netifEntry.netifName, "wan")))
            return 1;
	}
    
    return -1;    
}
#endif
#endif
#endif


