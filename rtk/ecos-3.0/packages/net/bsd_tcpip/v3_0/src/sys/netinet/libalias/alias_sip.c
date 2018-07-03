/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : SIP ALG PROCESS
* Abstract : 
* Author : lynn_xu (port from osk system)  
*/

/* System include files */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>

#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/time.h>

/* BSD network include files */
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>

#include "alias.h"
#include "alias_local.h"


#define ISALPHA(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define ISDIGIT(c) (((c) >= '0') && ((c) <= '9'))

/* notice: mimes_getport[] and mimes[] must be the same*/
static const char *mimes_getport[]=
{
	"m=audio %lu",
 	"m=image %lu",
 	"m=text %lu",
 	"m=video %lu",
 	"m=application %lu",
 	"m=message %lu",
 	"m=multipart %lu"
};

static const char *mimes[]=
{
	"m=audio",
 	"m=image",
 	"m=text",
 	"m=video",
 	"m=application",
 	"m=message",
 	"m=multipart"
};

#define MIME_NUM sizeof(mimes)/sizeof(mimes[0])

static const char keyEOL[] = "\r\n";
static const char keyInvite[] = "INVITE ";
static const char keyCeq[] = "c=";
static const char keyMeq[] = "m=";
static const char keyOeq[] = "o=";
static const char keyVia[] = "Via:";
static const char keyContact[] = "Contact:";
static const char keyContLen[] = "Content-Length:";
static const char keyRegister[] = "REGISTER ";
static const char keySubscribe[] = "SUBSCRIBE";
static const char keyAck[] = "ACK";
static const char keyBye[] = "BYE";
static unsigned short rtpPortOutToIn = 0;

#define ENDOFSTREAM -1
#define NAT_IN2OUT 1
#define NAT_FROM_OUT 2
#define RTP_RTCP_PORT_GROUP 2
#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#define SIP_CONTROL_PORT_NUMBER 5060

#define SIP_MAX_BUFFER_LEN 1500

static int getline(char *source, char *line)
{
	char *pstr;
	int len;

	pstr = strstr(source, keyEOL);
	if (!pstr)
		return ENDOFSTREAM;

	len = (pstr - source);

	strncpy(line, source, len);
	line[len] = 0;

	return len;
}


typedef unsigned short word;
typedef unsigned int   dword;
typedef unsigned char byte;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

#define WFROMPDU(add)		n2u16(add)
#define n2u16(add) ((((*(byte *)(add)) <<8)&0xff00) |\
	(*((byte *)(add) +1)))

word ipcsum(word *ptr, int len, word resid)
{
	register dword csum = resid;
	int odd = 0;

	if(len & 1) odd = 1;
	len = len >> 1;

	for(;len > 0 ; len--,ptr++){

		csum += WFROMPDU(ptr);
	}
	if(odd){
		csum += (*((byte *)ptr) <<8) & 0xff00;
	}

	/* take care of 1's complement */
	csum = (csum & 0xffff) + (csum >> 16);
	csum = (csum & 0xffff) + (csum >> 16);
	if(csum == 0xffff)csum = 0;
	return((word)csum);
}


void natRedoChecksum(unsigned char *cp)
{
    int    hlen, tlen;
    unsigned char  phdr[12];
    unsigned short csum;
    struct ip *ipHdr;
    struct udphdr *udpHdr;
    int i;
    
    ipHdr = (struct ip *)cp;
    /* check IP header */
    hlen = (ipHdr->ip_hl << 2);
    ipHdr->ip_sum = htons(0);
    csum = ipcsum((uint16 *)ipHdr, hlen, 0);
    ipHdr->ip_sum = htons(~csum);
    cp  += hlen;
    udpHdr = (struct udphdr *) cp;
    //diag_printf("%s: udp check sum = 0X%x \n", __FUNCTION__, udpHdr->uh_sum);
    if ((ipHdr->ip_p == IPPROTO_UDP) && (udpHdr->uh_sum != 0))
    {
        memcpy(&phdr[0], (unsigned char*)&ipHdr->ip_src.s_addr, 4);
        memcpy(&phdr[4], (unsigned char*)&ipHdr->ip_dst.s_addr, 4);
        phdr[8] = 0;
        phdr[9] = ipHdr->ip_p;
        memcpy(&phdr[10], (unsigned char*)&udpHdr->uh_ulen, 2);
        tlen  = ntohs(udpHdr->uh_ulen);
        udpHdr->uh_sum = htons(0);

        csum = ipcsum((uint16 *)phdr, 12, 0);
        csum = ipcsum((uint16 *)cp, tlen, csum);
        udpHdr->uh_sum = htons(~csum);

    }
}


static char ipxString[24];
char *getIPString(unsigned int ip)
{
    unsigned char *ipa;
    ipa= (unsigned char*)&ip;
    sprintf( &ipxString[0], "%d.%d.%d.%d", ipa[0], ipa[1], ipa[2], ipa[3]);
    return((char*)&ipxString);
}


void getIpPortFromString(char*in,int ip[4],int *port,unsigned int *xip){
	 if(!in) return;
  	 sscanf(in,"%lu.%lu.%lu.%lu:%lu;", &ip[0], &ip[1], &ip[2], &ip[3], port);				 
	 *xip=(ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3];

}
//return -1 ---error
int  SipMessageBodyFix(struct ip *pip, struct alias_link *link, int xlat,char* in,char* out,int isInvite)
{
		int count=0;
		char* msgbody=in;
		char* outbody=out;
        char line[128]={0};
        u_short salias = 0, base_alias = 0;
        int j;
        struct alias_link *rtp_link = NULL, *relink = NULL;
        struct in_addr null_addr, fixIp = {0}; 
        
		if(!in||!out)
            return 0;
		switch(xlat)
        {
    		case NAT_IN2OUT:
    			fixIp = GetAliasAddress(link);
    			break;
    		case NAT_FROM_OUT:
    			break;
    		default:
    			return -1;
		}
		while ((count = getline(msgbody, outbody)) != ENDOFSTREAM)
		{

			 msgbody = msgbody + count + 2;
			 if (!strncmp(outbody, keyCeq, strlen(keyCeq)))
		     {//c=
		      		
		          //diag_printf("%s: change \"%s\" to ", __FUNCTION__, outbody);				    
			      if(fixIp.s_addr) 
                  {
    				  memset(outbody,0,count);
    				  sprintf(outbody, "c=IN IP4 %s", getIPString((fixIp.s_addr)));
			      }
			     //diag_printf("\"%s\"\n", outbody);
		     } 
             else if (!strncmp(outbody, keyOeq, strlen(keyOeq)))
		     {//o=
			      char *tmp;				      
			      tmp = strstr(outbody, "IP4");
			      //diag_printf("%S: change \"%s\" to ", __FUNCTION__, outbody);
                  if(fixIp.s_addr && tmp)
                  {
                      memset(tmp,0,count-(tmp-outbody));
                      sprintf(tmp, "IP4 %s", getIPString((fixIp.s_addr)));
                  }
			     //diag_printf("\"%s\"\n", outbody);
		     }
             else if (isInvite && (!strncmp(outbody, keyMeq, strlen(keyMeq))))//just for the packet from lan
             //else if ((!strncmp(outbody, keyMeq, strlen(keyMeq))))//invite request and ok response
		     {//m=
		      	  int i, mime_type=-1;
			      unsigned int port;
			      char *tmpstr;
                  //diag_printf("%s %d outbody=%s\n", __FUNCTION__, __LINE__, outbody);
				  for(i = 0; i < MIME_NUM; i++)
				  {
					  if(sscanf(outbody, mimes_getport[i], &port) == 1) 
					  {
						  mime_type = i;
						  break;
					  }
				  }
                  //diag_printf("%s %d mime_type=%d i=%d\n", __FUNCTION__, __LINE__, mime_type, i);
			      tmpstr = strstr(outbody, " ");
			      if(mime_type == -1 || !tmpstr )
                    goto end;
			      tmpstr = strstr((char *) (tmpstr + 1), " ");
			      if(!tmpstr)
                    goto end;
			      strcpy(line, (char *) (tmpstr + 1));
                  
                  if (xlat == NAT_IN2OUT)
                  {
                      null_addr.s_addr = 0;
                      if (0 == (salias = FindNewPortGroup(null_addr,
                                    FindAliasAddress(pip->ip_src),
                                htons(port), 0, 
                                RTP_RTCP_PORT_GROUP, 
                                IPPROTO_UDP, 1)))
                      {  
#ifdef DEBUG
                          fprintf(stderr,
                          "PacketAlias/sip: Cannot find contiguous RTSP data ports\n");
#endif
                      }
                      else 
                      {
                           //diag_printf("%s %d salias=%d\n", __FUNCTION__, __LINE__, salias);
                           base_alias = ntohs(salias);
                           for (j = 0; j < RTP_RTCP_PORT_GROUP; j++) 
                           {
                               rtp_link = FindSipOut(GetOriginalAddress(link), null_addr,
                                htons(port + j),htons(base_alias + j),IPPROTO_UDP);

                                    
                                                                  
                                if (rtp_link != NULL) {
#ifndef NO_FW_PUNCH
                                  PunchFWHole(rtp_link);
#endif
                                } else {
#ifdef DEBUG
                                  fprintf(stderr,
                                  "PacketAlias/SIP: Cannot allocate SIP data ports\n");
#endif
                                  break;
                                }
                            }
                           //diag_printf("%S: change \"%s\" to ", __FUNCTION__, outbody);
                           memset(outbody,0,count);
                           sprintf(outbody, "%s %d %s", mimes[mime_type], salias, line);
                           //diag_printf("\"%s\"\n", outbody);   
                      }
                  }

                

		    }
             
		end:
		    strcat(outbody, keyEOL);
		    outbody += strlen(outbody);
		 }
        
		 return 0;
}
//return -1 error
int SipMessageHeaderFix(struct ip *pip, struct alias_link *link, int xlat,char* in,char* out,int bReq,char* outbody)
{
	int count=0;
	char *msgheader=in;
	char *outheader=out;
	char line[128]={0};
	int viaip[4];
	int viaport;
	unsigned int viaIp=0;//fix
    struct alias_link *vialink = NULL;
    struct in_addr srcaddr = {0}, destaddr = {0}, aliasaddr = {0};
    struct in_addr tmpaddr = {0}, tmpip = {0};
    u_short aliasport = 0, srcport = 0, destport = 0;
    
	if(!in||!out)
        return 0;
	memset(viaip,'\0',sizeof(viaip));
    srcaddr = GetOriginalAddress(link);
    destaddr = GetDestAddress(link);
    aliasaddr = GetAliasAddress(link);
    srcport = GetOriginalPort(link);
    destport = GetDestPort(link);
    aliasport = GetAliasPort(link);
    
 	 while ((count = getline(msgheader, outheader)) != ENDOFSTREAM)
     {
         //diag_printf("%s: outheader=%s\n", __FUNCTION__, outheader);
         msgheader += (count + 2);	//0D0A    

         if (!strncmp(outheader, keyVia, strlen(keyVia)))
	     {//process the message header via
		      char *viastr = 0;
		      viastr = strstr(outheader, ";");
		      if(!viastr) 
                goto end;
		      sprintf(line, "%s", viastr);
              getIpPortFromString(outheader+strlen("Via: SIP/2.0/UDP "),viaip, &viaport,&viaIp); 
              tmpip.s_addr = viaIp;
              if(xlat == NAT_FROM_OUT)
		      {
                   if((ntohs(viaport) == aliasport) && (ntohl(viaIp) == aliasaddr.s_addr))//fix multiple via situation
                       sprintf(outheader, "Via: SIP/2.0/UDP %s:%d%s", getIPString((srcaddr.s_addr)), htons(srcport), line);
    		          
		      }
              else if(xlat == NAT_IN2OUT)
		     {
				    if(viaport && (ntohl(viaIp) == srcaddr.s_addr))//fix 
					    sprintf(outheader, "Via: SIP/2.0/UDP %s:%d%s", getIPString((aliasaddr.s_addr)), htons(aliasport), line);
                   
		     }
		    // diag_printf("\"%s\"\n", outheader);
	    } 
        else if(!strncmp(outheader, keyInvite, strlen(keyInvite))|| !strncmp(outheader, keySubscribe, strlen(keySubscribe))||
            !strncmp(outheader, keyAck, strlen(keyAck))||!strncmp(outheader, keyBye, strlen(keyBye)))
        {
             char *tmpstr;
		     tmpstr = strstr(outheader, "@");				   
		     if(tmpstr)
             {
			 	  char* spacestr=strstr(tmpstr," ");
				  if(!spacestr) 
                    goto end;
				  memset(line,0,sizeof(line));					
				  strcpy(line,spacestr);
			 	  if(sscanf(tmpstr+strlen("@"),"%lu.%lu.%lu.%lu:%lu",&viaip[0],&viaip[1],&viaip[2],&viaip[3],&viaport)==5)
                  {
						viaIp = (viaip[0]<<24)+(viaip[1]<<16)+(viaip[2]<<8)+viaip[3];
                        //diag_printf("%s[%d]: viaIp=%s\n",__FUNCTION__, __LINE__, getIPString(viaIp));
						if((viaIp != srcaddr.s_addr) && (viaIp != aliasaddr.s_addr)) 
                            goto end;
						memset(tmpstr+strlen("@"),0,count-(outheader-tmpstr-strlen("@")));
						if(xlat == NAT_FROM_OUT)
                        {
							sprintf(tmpstr+strlen("@"),"%s:%d",getIPString((srcaddr.s_addr)), srcport);
						}else if(xlat==NAT_IN2OUT)
						{
							sprintf(tmpstr+strlen("@"),"%s:%d",getIPString((aliasaddr.s_addr)), aliasport);
						}
						strcat(outheader,line);
                        //diag_printf("%s[%d]: invite message change to \"%s\" \n",__FUNCTION__, __LINE__, outheader);
			        }
		      }
	    
	    }
	    else if (!strncmp(outheader, keyContact, strlen(keyContact)))
        {

            char *data = outheader;
            char *data_limit = outheader + strlen(outheader);
            char *starturi;
            char *addrstart;
            unsigned int ip[4], port;
            while (data < data_limit)
            {
                /* find the topmost tag */
                if (strnicmp(data, keyContact, strlen(keyContact)))
                {
            	     data++;
            	     continue;
                }
            //diag_printf("Found contact header\n");
            /* Look for sip: */
            while (strnicmp(data, "sip:", 4))
            {
            	if (data == data_limit)
            		 break;
            	 data++;
            }
            data += 4;

            /* we have to look to see if there's user info in the contact header */
            starturi = data;
            while (*data != '@' && *data != '>' && *data != ';' && *data != '\n' && *data != '\r' && *data != '?' && *data != ',')
            {
            	if (data == data_limit)
            		break;
            	data++;
            }

            /* check for userinfo */
            if (*data == '@')
            {
            	  /* woop! */
            	  data++;
            }
            else
            {
            	  data = starturi;
            }

            /* we should be fine now */
            addrstart = data;

            /* this accepts FQDNs or dotted quads */
            while (ISALPHA(*data) || ISDIGIT(*data) || (*data == '.') || (*data == '-'))
            {
            	  data++;
            	  if (data == data_limit)
            		  break;
            }

            /* skip the port information too, if any */
            if (*data == ':')
            {
            	  data++;
            	  while (ISDIGIT(*data))
            		  data++;
            }
            if (sscanf(addrstart, "%lu.%lu.%lu.%lu:%lu", &ip[0], &ip[1], &ip[2], &ip[3], &port) == 5)
            {
                  unsigned int contactIp = (ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3];
            	  char manglestr[1500] = { 0 };
            	  memcpy(manglestr, outheader, addrstart - outheader);
            	  if(xlat==NAT_IN2OUT)
            	  {
            	      if(ntohl(contactIp) == srcaddr.s_addr)
            	 	      sprintf(manglestr + (addrstart - outheader), "%s:%d", getIPString((aliasaddr.s_addr)), htons(aliasport));
            		  else
            		      sprintf(manglestr + (addrstart - outheader), "%s:%d", getIPString(htonl(contactIp)), port);
            	  }
            	  else
                  {
            		   if(ntohl(contactIp) == aliasaddr.s_addr)
            		       sprintf(manglestr + (addrstart - outheader), "%s:%d", getIPString((srcaddr.s_addr)), htons(srcport));	
            		   else
            		   	   sprintf(manglestr + (addrstart - outheader), "%s:%d", getIPString(htonl(contactIp)), port);
            	  }
            	  memcpy(manglestr + strlen(manglestr), data, strlen(outheader) - (data - outheader));
            	  //diag_printf("%s:Contact Before:%s\n", __FUNCTION__, outheader);
            	  memset(outheader, 0, strlen(outheader));
            	  sprintf(outheader, "%s", manglestr);
            	  outheader[strlen(outheader)] = '\0';
            	  //diag_printf("%S:Contact After:%s\n", __FUNCTION__, outheader);
            }
            break;
            }
        }
		else if (!strncmp(outheader, keyContLen, strlen(keyContLen)))
	    {
		    sprintf(outheader, "%s %d", keyContLen, strlen(outbody));
	    }
end:
	    strcat(outheader, keyEOL);
	    outheader += strlen(outheader);
    }
	return 0;
}
 int AliasHandleSip(struct ip *pip, struct alias_link *link)
{
    int    hlen, tlen, dlen;
    struct udphdr *ud;
	char *sipmsg;
	char *msgheader, *msgbody;
	int len, isModified = FALSE;
	int bReq = FALSE;
	//char outheader[1500]={0};
	//char outbody[1500]={0};
	char *outheader = NULL;
	char *outbody = NULL;
    int xlat;

	outheader = malloc(SIP_MAX_BUFFER_LEN, M_ALG, M_NOWAIT);
	if (outheader == NULL){
		diag_printf("%s:%d malloc failed!\n", __FUNCTION__, __LINE__);
		return -1;
	}
	outbody = malloc(SIP_MAX_BUFFER_LEN, M_ALG, M_NOWAIT);
	if (outbody == NULL){
		diag_printf("%s:%d malloc failed!\n", __FUNCTION__, __LINE__);
		if (outheader){
			free(outheader, M_ALG);
		}
		return -1;
	}
	//init memory
	memset(outheader, '\0', SIP_MAX_BUFFER_LEN);
	memset(outbody, '\0', SIP_MAX_BUFFER_LEN);
	
    ud = (struct udphdr *)((char *)pip + (pip->ip_hl << 2));
    hlen = (pip->ip_hl << 2) + sizeof(struct udphdr);
    tlen = (pip->ip_len);   
    dlen = tlen - hlen;
    sipmsg = (char*)pip;
    sipmsg += hlen;
    sipmsg[dlen] = 0; // pad NULL char. 

    //judge direction
    if (ntohs(ud->uh_dport) == SIP_CONTROL_PORT_NUMBER)
        xlat = NAT_IN2OUT;
    else if (ntohs(ud->uh_sport) == SIP_CONTROL_PORT_NUMBER)
        xlat = NAT_FROM_OUT;
    else
        xlat = 0;
    
	switch (xlat)
	  {
	  case NAT_IN2OUT:
	  case NAT_FROM_OUT:
 		  if ((sipmsg[0]) && (!strncmp(sipmsg, keyInvite, strlen(keyInvite))))
		  {
		      bReq = TRUE;
		  }
          else
		  {
			  bReq = FALSE;
		  }
		  if(!strncmp(sipmsg, keyRegister, strlen(keyRegister)))
          {
              bReq = TRUE;
          }
		  msgheader = sipmsg;
		  if (0 == (msgbody = strstr(sipmsg, "\r\n\r\n")))
			  break; 
		
		  msgbody[2] = 0;
		  msgbody += 4;
		 if(msgbody[4] == 0) 
		 {
    		 goto MSG_HEADER_HANDLE;
 		 }
		 SipMessageBodyFix(pip, link, xlat,msgbody, outbody,bReq);
MSG_HEADER_HANDLE:
		  // now we build header.		
		  SipMessageHeaderFix(pip, link, xlat, msgheader,outheader,bReq,outbody);
		  strcat(outheader, keyEOL);
		  memset(sipmsg, 0, dlen);
		  memcpy(sipmsg,outheader,strlen(outheader));

		  memcpy(sipmsg+strlen(outheader),outbody,strlen(outbody));
		  sipmsg[strlen(outheader)+strlen(outbody)]='\0';
		  // modify ip/udp/length
		  len = strlen(sipmsg);
		  len += sizeof(struct udphdr);
          ud->uh_ulen = htons(len);
		  len += (pip->ip_hl << 2);
          pip->ip_len = (len);
		  isModified = TRUE;
		  break;
	  default:
		  break;
	  }

	if (isModified)
	  {
		  //diag_printf("isModified=%d\n", isModified);
		#if BYTE_ORDER == LITTLE_ENDIAN
		HTONS(pip->ip_len);
		HTONS(pip->ip_off);
		#endif		
		  natRedoChecksum((char *) pip);
		#if BYTE_ORDER == LITTLE_ENDIAN
		NTOHS(pip->ip_len);
		NTOHS(pip->ip_off);
		#endif
	  }
	if (outheader){
		free(outheader, M_ALG);
	}
	if (outbody){
		free(outbody, M_ALG);
	}
	return 0;
#if 0	
err:
	return -1;
#endif	
}

