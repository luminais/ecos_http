#include <sys/param.h>
//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>


#include "apmib.h"
#include "common.h"


#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
static int rtl_vlan_support_enable_old = 0;
extern int rtl_vlan_support_enable;
void rtl_setPortDefVlanByDevName(unsigned short pVid, unsigned char pPri, unsigned char pCfi, unsigned char *name);
int rtl_addRtlVlanEntry(unsigned short vid);
int rtl_delRtlVlanEntry(unsigned short vid);
int rtl_flushRtlVlanEntry(void);
int rtl_addRtlVlanMemberPortByDevName(unsigned short vid, unsigned char *name);
int rtl_delRtlVlanMemPortByDevName(unsigned short vid, unsigned char *name);
int rtl_setRtlVlanPortTagByDevName(unsigned short vid, unsigned char *name, int tag);
int getNetifEntry(int netifId,NETIFACE_Tp pNetifEntry);
#endif


#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
#if defined(CONFIG_RTL_HARDWARE_NAT)
int flush_hw_table_flag = 0;
#endif
void rtl_reinitByVlanSetting(void)
{
	if(rtl_vlan_support_enable_old != rtl_vlan_support_enable){
		extern void configure_bridge();
		configure_bridge();

		extern void configure_nic();
		configure_nic();

		#if defined(CONFIG_RTL_HARDWARE_NAT)
		flush_hw_table_flag = 1;
		#endif
		extern void configure_opmode();
		configure_opmode();
	}
}

void set_vlanInfo(void)
{
	int entryNum=0, netifEntryNum =0, vidEntryNum=0, i=0, j=0, intVal=0;
	VLAN_NETIF_BIND_T entry={0};
	VLAN_T vlanEntry={0};
	NETIFACE_T netifEntry={0};

	rtl_vlan_support_enable_old = rtl_vlan_support_enable;
	rtl_vlan_support_enable = 0;
	apmib_get(MIB_VLAN_ENABLED, (void *)&intVal);
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif

	if(intVal == 0)
		return;

	/*Set dev pvid, default priority and cfi*/
	if (!apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&netifEntryNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return;
	}
	
	for(i=1;i<=netifEntryNum;i++)
	{
		*((char *)&netifEntry) = (char)i;
		if (!apmib_get(MIB_NETIFACE_TBL, (void *)&netifEntry))
			return;
		
		rtl_setPortDefVlanByDevName(netifEntry.netifPvid, netifEntry.netifDefPriority, netifEntry.netifDefCfi, netifEntry.netifName);
	}
	
	if (!apmib_get(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return;
	}
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&vidEntryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return;
	}
	
	rtl_flushRtlVlanEntry();
	
	for (i=1; i<=vidEntryNum; i++) 
	{
		*((char *)&vlanEntry) = (char)i;
		
		if (!apmib_get(MIB_VLAN_TBL,(void *)&vlanEntry))
			return;
		
		rtl_addRtlVlanEntry(vlanEntry.vlanId);
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        rtl_setRtlVlanForwardRule(vlanEntry.vlanId, vlanEntry.ForwardRule);
        #endif
		for(j=1;j<=entryNum;j++)
		{
			*((char *)&entry) = (char)j;
			if(!apmib_get(MIB_VLAN_NETIF_BIND_TBL,(void*)&entry))
				return;

			if(entry.vlanId==vlanEntry.vlanId)
			{
				if(getNetifEntry(entry.netifId,&netifEntry)<0)
				{
					printf("get netIface entry fail vid=%d\n",entry.netifId);
					return;
				}
				
				if(!netifEntry.netifEnable) 
					continue;
				
				rtl_addRtlVlanMemberPortByDevName(vlanEntry.vlanId, netifEntry.netifName);
				
				if(entry.tagged)
					rtl_setRtlVlanPortTagByDevName(vlanEntry.vlanId, netifEntry.netifName, 1);
			}
		}
	}
	
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	rtl_addCustomPassthroughVlanInfo();
	#endif
	
	rtl_vlan_support_enable = 1;
}
#endif

