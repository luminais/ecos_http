/*
 * NVRAM variable manipulation (direct mapped flash)
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_rw.c,v 1.53 2010/08/02 17:35:45 Exp $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <mipsinc.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <flashutl.h>
#include <hndsoc.h>
#include <sbchipc.h>

#ifdef __ECOS
extern int kernel_initial;
#endif

//liao li fang +++
extern int nvram_erase(void);
#define BOOT_NVRAM_SPACE  0x1000
#define BOOT_NVRAM_NUM    128

//add by tenda
/* nvram data structure */
struct envram_data {
	char name[64];
	char value[64];
};
static struct nvram_header *header;
static struct envram_data envramdata[BOOT_NVRAM_NUM];
static int hasinit = 0;
int envram_init(int argc, char* argv[])
{
	char *name, *value, *end, *eq;
	int i = 0;
	if (!(header = (struct nvram_header *) MALLOC(NULL, BOOT_NVRAM_SPACE))) {
		printf("nvram_commit: out of memory\n");
		return -12; /* -ENOMEM */
	}
	
	/* Read boot NVRAM */
	sysFlashInit(NULL);
	bootnvRead((uchar *)header, BOOT_NVRAM_SPACE);
	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + BOOT_NVRAM_SPACE - 2;
	end[0] = end[1] = '\0';
	
	bzero(envramdata,sizeof(struct envram_data)*BOOT_NVRAM_NUM);
	for (; *name && i<BOOT_NVRAM_NUM; name = value + strlen(value) + 1,i++) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		strcpy(envramdata[i].name , name);
		strcpy(envramdata[i].value , value);
		//*eq = '=';
	}
	for(;i<BOOT_NVRAM_NUM;i++){
		envramdata[i].name[0] = 0;
	}
	hasinit = 1;
	//MFREE(NULL, header, BOOT_NVRAM_SPACE);
	return 0;
}

int envram_show(int argc, char* argv[])
{
	int i = 0;
	if(hasinit == 0)
	{
		envram_init(0, NULL);
	}
	for(i=0;i<BOOT_NVRAM_NUM;i++)
	{
		if(envramdata[i].name[0] !=0 )
		{
			diag_printf("%s=%s\n",envramdata[i].name,envramdata[i].value);
		}
	}
	return 0;
}

int envram_to_nvram(void)
{
	int i = 0;
	if(hasinit == 0)
	{
		envram_init(0, NULL);
	}
	for(i=0;i<BOOT_NVRAM_NUM;i++)
	{
		if(envramdata[i].name[0] !=0 )
		{
			nvram_set(envramdata[i].name,envramdata[i].value);
		}
	}
	return 0;
}
	
	
int envram_get(int argc, char* argv[])
{
	char *var = NULL;
	char *name = NULL;
	int found = 0,i = 0;
	if(hasinit == 0)
	{
		envram_init(0, NULL);
	}
	if (argc < 3)
		return -1;
	name = argv[2];
	for(i=0;i<256;i++)
	{
		if(envramdata[i].name[0] !=0 )
		{
			if(strcmp(envramdata[i].name,name)==0)
			{
				found = 1;
				break;
			}
		}else{
			break;
		}
	}
	if(found == 1)
	{
		var = envramdata[i].value;
		//huangxiaoli add  20111009
		argv[2] = var;
	}
	else
	{
		diag_printf("not found\n");
	}
	if (var)
		diag_printf("%s\n", var);

	return 0;
}
int envram_set(int argc, char* argv[])
{
	char *name;
	char *value;
	char *p;
	int i = 0,found = 0;
	if(hasinit == 0)
	{
		envram_init(0, NULL);
	}
	if (argc < 3)
		return -1;

	name = argv[2];
	value = strchr(name, '=');
	if (value == NULL || strlen(value) >= 64){
		diag_printf("value illegal: %s\n",value?value:"NULL");
		return -1;
	}	
	p = value;
	*value++ = 0;
	
	for(i=0;i<256;i++)
	{
		if(envramdata[i].name[0] !=0 )
		{
			if(strcmp(envramdata[i].name,name)==0)
			{
				found = 1;
				break;
			}
		}else{
			break;
		}
	}
	if(found == 1)
	{
		strcpy( envramdata[i].value,value);
		//diag_printf("set value success\n");
	}
	else
	{
		if(i<256){
			strcpy( envramdata[i].name,name);
			strcpy( envramdata[i].value,value);
		}
	}
	//for cli_nvram_set on product test
	*p = '=';
	return 0;
}
int envram_commit(int argc, char* argv[])
{
	//struct nvram_header *header;
	char * ptr = NULL;
	char *end = NULL;
	int i = 0;
	if(hasinit == 0)
	{
		//envram_init(0, NULL);
		return 0;
	}
	/*if (!(header = (struct nvram_header *) MALLOC(NULL, BOOT_NVRAM_SPACE))) {
		printf("nvram_commit: out of memory\n");
		return -12; 
	}*/
	
	ptr = (char *) header + sizeof(struct nvram_header);
	bzero(ptr, BOOT_NVRAM_SPACE - sizeof(struct nvram_header));

	/* Leave space for a double NUL at the end */
	end = (char *) header + BOOT_NVRAM_SPACE - 2;

	/* Write out all tuples */
	for (i = 0; i <BOOT_NVRAM_NUM; i++) {
		if(envramdata[i].name[0] !=0 ) {
			if ((ptr + strlen(envramdata[i].name) + 1 + strlen(envramdata[i].value) + 1) > end)
			{
				break;
			}
			ptr += sprintf(ptr, "%s=%s",envramdata[i].name, envramdata[i].value) + 1;
		}
	}
//	diag_printf((char *) &header[1],(char *) &header[2]);
/* End with a double NUL */
	ptr += 2;

	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);

	/* Set new CRC8 */
	header->crc_ver_init |= nvram_calc_crc(header);

	bootnvWriteChars((unsigned char *) header, BOOT_NVRAM_SPACE);
	MFREE(NULL, header, BOOT_NVRAM_SPACE);
	hasinit = 0;
	return 0;
}
//
