/*
	To create Flash Image, we need input CFE(boot), image, and envram parameters etc.

	Flash size and memory size , we can Get from envram maybe.

													wangxinyu.yy@gmail.com
													2015.6.5
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>   
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "mkFlashImage.h"
#include "version.h"




#define READNVRAM 1
#define READENVRAM 2
#define READFACTORY 3

//#define TRACE printf("%s[%d] \n", __FUNCTION__, __LINE__);


#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]

uint8 hndcrc8(
	uint8 *pdata,	/* pointer to array of data to process */
	uint  nbytes,	/* number of input data bytes to process */
	uint8 crc	/* either CRC8_INIT_VALUE or previous return value */
)
{
	/* hard code the crc loop instead of using CRC_INNER_LOOP macro
	 * to avoid the undefined and unnecessary (uint8 >> 8) operation.
	 */
	while (nbytes-- > 0)
		crc = hnd_crc8_table[(crc ^ *pdata++) & 0xff];

	return crc;
}


uint8 nvram_calc_crc(struct nvram_header *nvh)
{
	struct nvram_header tmp;
	uint8 crc;

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htonl((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
	tmp.config_refresh = htonl(nvh->config_refresh);
	tmp.config_ncdl = htonl(nvh->config_ncdl);

	crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
		sizeof(struct nvram_header) - NVRAM_CRC_START_POSITION,
		CRC8_INIT_VALUE);

	/* Continue CRC8 over data bytes */
	crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);

	return crc;
}


/* returns the CRC8 of the nvram F9中的校验函数*/
static uint8 nvram_calc_crc1(struct nvram_header *nvh)
{
    struct nvram_header tmp;
    uint8 crc;

    
    /* Little-endian CRC8 over the last 11 bytes of the header */
    tmp.crc_ver_init = htonl((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
    tmp.config_refresh = htonl(nvh->config_refresh);
    tmp.config_ncdl = htonl(nvh->config_ncdl);


	printf("line[%d] tmp.crc_ver_init=0x%x\n",__LINE__,tmp.crc_ver_init);
	printf("line[%d] tmp.config_refresh=0x%x\n",__LINE__,tmp.config_refresh);
	printf("line[%d] tmp.config_ncdl=0x%x\n",__LINE__,tmp.config_ncdl);

    crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
        sizeof(struct nvram_header) - NVRAM_CRC_START_POSITION,
        CRC8_INIT_VALUE);

    /* Continue CRC8 over data bytes */
    crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);

    return crc;
}


/* For calculate Flash image check sum. */
uint32_t  byteSum8(uint32_t sum, unsigned char *buf, int len)
{
	int i = 0;
	
	for(i = 0; i < len ; i++)
	{
		sum += buf[i];
	}

	return sum;
}

/* 
	binary size.
*/
static int getBinarySize(int size, int *q)
{
	int	mask;

	mask = size>>1;
	for (*q = 0; mask; mask >>= 1) {
		*q = *q + 1;
	}
	return 1<<*q;
}


static void show_config(void)
{
	printf("\n*********************************************************\n");

	printf("%-20s: %d%s\n", "flash size", flashsize/MB,"M");
	printf("%-20s: %-20s\n", "cfe path", cfepath);
	printf("%-20s: %-20s\n", "image path",imagepath);
	printf("%-20s: %-20s\n", "out FLASH path", flashpath);

}

//根据名字获取相应的值
int getValueByName(char *name,char *value,struct nvram_tuple *head)
{
	struct nvram_tuple *node = NULL;

	if((NULL == name) || (NULL == value) || (NULL == head))
	{
		return -1;
	}


	for(node = head->next; node != NULL; node = node->next)
	{
		if(0 == strcmp(name,node->name))
		{
			strcpy(value,node->value);
			break;
		}
	}

	return 0;
}

//



//根据名字设置链表节点中相应的值
int setValueByName(char *name,char *value,struct nvram_tuple *head)
{
	struct nvram_tuple *node = NULL;
	struct nvram_tuple *tmp;
	
	if((NULL == name) || (NULL == value) || (NULL == head))
	{
		printf("%d line is error\n",__LINE__);
		return -1;
	}

	for(node = head->next; node != NULL; node = node->next)
	{
		if(0 == strcmp(name,node->name))
		{
			strcpy(node->value,value);
			//printf("node->name:%s node->value:%s\n",node->name,node->value);
			return 0;
		}
	}
	tmp = (struct nvram_tuple *)malloc(sizeof(struct nvram_tuple));
	if(NULL == tmp)
	{
		perror("malloc");
		return -1;
	}
	tmp->next = NULL;
	tmp->name = (char *)malloc((strlen(name)) + 1);
	if(NULL == tmp->name)
	{
		perror("malloc");
		return -1;
	}
	tmp->value = (char *)malloc((strlen(value)) + 1);
	if(NULL == tmp->value)
	{
		perror("malloc");
		return -1;
	}

	strcpy(tmp->name,name);
	strcpy(tmp->value,value);

	node = head;
	while(node->next != NULL)
		node = node->next;
	
	node->next= tmp;

	return 0;
}



//根据配置构造flash转测试镜像的名字
void makeFlashName(char *flashName)
{
	char productName[16] = {0};
	char version[32] = {0};
	char language[8] = {0};

	if(NULL == flashName)
	{
		return;
	}
	memset(productName,0x0,sizeof(productName));
	memset(version,0x0,sizeof(version));
	memset(language,0x0,sizeof(language));

	sprintf(productName,"%s",__CONFIG_HW_VER__);  //获取硬件版本号
	sprintf(version,"%s",W311R_ECOS_SV);
	sprintf(language,"%s",__CONFIG_WEB_VERSION__);
	if(0 == strcmp("cn",language))
	{
		sprintf(flashName,"%s%s%s%s%s%s%s","FS_",productName,"RTL_",version,"_",language,"_TDC01.bin");
	}
	else
	{
		sprintf(flashName,"%s%s%s%s%s%s%s","FS_",productName,"RTL_",version,"_",language,"_TDE01.bin");
	}

	return ;
}

//根据mac地址构造一个默认的SSID
int makeSsidByMac(char *prev,char *mac,char *ssid)
{
	char wlanmac[64]={'\0'};
	int i = 0,l = 0;

	if((NULL == prev) ||(NULL == mac) || (NULL ==ssid))
	{
		return -1;
	}

	strcpy(wlanmac,mac);

	for(i=0;  wlanmac[i]!='\0' && i<9; ++i){
		if (wlanmac[i+9] != ':'){		
				wlanmac[l]=wlanmac[i+9];
				++l;
			}
	}
	wlanmac[6]='\0';
	
	sprintf(ssid,"%s_%s", prev, wlanmac);
	

	return 0;
}


void addNvramTupleTail(struct nvram_tuple *head, struct nvram_tuple *node)
{
	struct nvram_tuple *newer;

	assert(node!=NULL);	

	newer = malloc(sizeof(struct nvram_tuple));
	if(!newer)
		error_exit("malloc");

	newer->name = strdup(node->name);
	newer->value = strdup(node->value);
	newer->next= NULL;

	while(head->next)
		head = head->next;

	head->next = newer;
}

void showNvramTupleList(struct nvram_tuple *head)
{
	while(head->next)
	{
		printf("%s=%s\n", head->next->name, head->next->value);
		head = head->next;
	}
}

void freeNvramTupleList(struct nvram_tuple *head)
{
	struct nvram_tuple *tmp = head->next;
	struct nvram_tuple *tmp1;
	
	while(tmp)
	{
		tmp1 = tmp->next;
		free(tmp->name);
		free(tmp->value);
		free(tmp);
		tmp = tmp1;
	}

	head->next = NULL;
}



//从默认配置读取nvram
int readnvram_default(int action)
{
	struct nvram_tuple *t;
	struct nvram_tuple *nvramList;

	if(READNVRAM == action)
	{
		t = tenda_nvram_defaults;
		nvramList = &nvramHead;
	}
	else if(READENVRAM == action)
	{
		t = tenda_envram_defaults;
		nvramList = &envramHead;
	}
	else if(READFACTORY == action)
	{
		t = tenda_factory_defaults;
		nvramList = &factoyrnvramHead;
	}
	else 
	{
		printf("unknow action\n");
		return -1;
	}
	
	for (t ; t->name; t++)
	{
		addNvramTupleTail(nvramList, t);	
	}

	return 0;
}



//读入boot
int fillBuffFromBoot(char *buf, int offset, char *file)
{
	FILE *fp = NULL;
	struct stat st;
	int fileLen = 0;
	int ret = 0;

	if(-1 == stat(file, &st))	//获取文件状态
		error_exit("stat");
	fileLen = st.st_size;

	fp = fopen(file, "r");
	if(!fp)
		error_exit("fopen");

	if(0 != fseek(fp,BOOTHEADERLEN,SEEK_SET))		//偏移跳过boot头的16字节
	{
		printf("fseek error!\n");
		fclose(fp);
		return -1;
	}
	
	ret = fread(buf + offset, sizeof(char), fileLen - BOOTHEADERLEN, fp);	//

	fclose(fp);

	if(ret != (fileLen - BOOTHEADERLEN))
	{
		printf("Read %s %d[Real length %d]\n", file, fileLen, ret);
	}
	
	return ret != (fileLen - BOOTHEADERLEN);
}

//读入升级文件
int fillBuffFromImage(char *buf, int offset, char *file)
{
	FILE *fp = NULL;
	struct stat st;
	int fileLen = 0;
	int ret = 0;

	if(-1 == stat(file, &st))	//获取文件状态
		error_exit("stat");
	fileLen = st.st_size;

	fp = fopen(file, "r");
	if(!fp)
		error_exit("fopen");

	if(0 != fseek(fp,IMAGEHEADERLEN,SEEK_SET))		//偏移跳过boot头的16字节
	{
		printf("fseek error!\n");
		fclose(fp);
		return -1;
	}
	
	ret = fread(buf + offset, sizeof(char), fileLen - IMAGEHEADERLEN, fp);	//

	fclose(fp);

	if(ret != (fileLen - IMAGEHEADERLEN))
	{
		printf("Read %s %d[Real length %d]\n", file, fileLen, ret);
	}
	
	return ret != (fileLen - IMAGEHEADERLEN);
}




/* If success return 0 */
int fillBuffFromFile(char *buf, int offset, char *file)
{
	FILE *fp = NULL;
	struct stat st;
	int fileLen = 0;
	int ret = 0;

	if(-1 == stat(file, &st))	//获取文件状态
		error_exit("stat");
	fileLen = st.st_size;

	fp = fopen(file, "r");
	if(!fp)
		error_exit("fopen");

	ret = fread(buf + offset, sizeof(char), fileLen, fp);	//

	fclose(fp);

	if(ret != fileLen)
	{
		printf("Read %s %d[Real length %d]\n", file, fileLen, ret);
	}
	
	return ret != fileLen;
}

/* */
int fillBuffEnvram(char *imageBuf)
{
	struct nvram_header *header = (struct nvram_header *)(imageBuf + BOOT_ENVRAM_OFFSET);
	struct nvram_tuple *node = envramHead.next;

	memset(header, 0x0, BOOT_ENVRAM_SPACE);
	
	char * ptr = NULL;
	char *end = NULL;
	int i = 0;
	int crc;
	char *tmp;
	
	ptr = (char *) header + sizeof(struct nvram_header);
	
	/* Leave space for a double NUL at the end */
	end = (char *) header + BOOT_ENVRAM_SPACE - 2;
	TRACE;
	/* Write out all tuples */
	for (i = 0; i <BOOT_NVRAM_NUM && node; i++, node = node->next) 
	{
		if ((ptr + strlen(node->name) + 1 + strlen(node->value) + 1) > end)
		{
				break;
		}
		
		ptr += sprintf(ptr, "%s=%s",node->name, node->value) + 1;
	}

	TRACE;
	
	/* End with a double NUL */
	//ptr += 2;
	ptr ++;
	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);	//此种方法才是求长度的正确方法
	

	/* Set header value */
	header->magic = 0x464C5348;

	//在F9中config_ncdl是随机值，所以这里直接赋值为0，省去各种平台之间的大小端转换
	header->config_ncdl = 0x0;

	//在bcm中config_refresh是用于标记flash大小的，F9不关注这个字段，直接赋为0，省去各种平台之间的转换
	header->config_refresh = 0x0;
	
	header->crc_ver_init = htonl(NVRAM_VERSION << 8);
	
	/* Set new CRC8 */
	header->crc_ver_init |=( crc = nvram_calc_crc(header ));/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	
	header->len = htonl(header->len);
	header->crc_ver_init = htonl(header->crc_ver_init);

	return 0;
}

void updateNvramFromEnvram(void)
{
	struct nvram_tuple *envnode = envramHead.next;


	for(;envnode;envnode = envnode->next)	
	{
		setValueByName(envnode->name,envnode->value,&nvramHead);
	}

	return ;
}

//将转测试需要修改的参数覆盖到nvram
void updateNvramFromFactory(void)
{
	struct nvram_tuple *factorynode = factoyrnvramHead.next;

	
	for(;factorynode;factorynode = factorynode->next)	
	{
		setValueByName(factorynode->name,factorynode->value,&nvramHead);
	}

	return ;
}


int fillBuffnvram(char *imageBuf)
{
	struct nvram_header *header = (struct nvram_header *)(imageBuf + (flashsize - NVRAM_SPACE));
	struct nvram_tuple *node = envramHead.next;
	char  et0macaddr[128] = {0};
	char wl_ssid[128] = {'\0'};
	char  defaultssid[128] = {0};
	int ret = 0;
	int crc;

	int isEnvram = 0;

	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32 config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32 config_ncdl;	/* ncdl values for memc */

	//从envram 链表中获取mac地址et0macaddr
	memset(et0macaddr,0x0,sizeof(et0macaddr));
	memset(wl_ssid,0x0,sizeof(wl_ssid));
	getValueByName("et0macaddr",et0macaddr,&envramHead);
	getValueByName(WL_SSID,wl_ssid,&nvramHead);
	//根据mac构造默认的ssid
	memset(defaultssid,0x0,sizeof(defaultssid));
	makeSsidByMac(wl_ssid,et0macaddr,defaultssid);
	

	//添加几个无线相关的值到nvram 
	setValueByName(WL_SSID,defaultssid,&nvramHead);
	setValueByName(EXTEND_SSID,defaultssid,&nvramHead);
	setValueByName(WL0_SSID,defaultssid,&nvramHead);
	setValueByName(DEFAULT_SSID,defaultssid,&nvramHead);
	//show_list(nvramHead.next);
	
	updateNvramFromEnvram();		//将envram的数据备份一份到nvram
	updateNvramFromFactory();		//将产测需要修改的nvram参数覆盖进去

	node = nvramHead.next;
	
	
	
	memset(header, 0x0, NVRAM_SPACE);
	
	char * ptr = NULL;
	char *end = NULL;
	int i = 0;
		
	ptr = (char *) header + sizeof(struct nvram_header);
	
	/* Leave space for a double NUL at the end */
	end = (char *) header + NVRAM_SPACE - 2;
	TRACE;
	/* Write out all tuples */
	for (i = 0; i <NVRAM_NUM && node; i++, node = node->next) 
	{
		if ((ptr + strlen(node->name) + 1 + strlen(node->value) + 1) > end)
		{
				break;
		}
		ptr += sprintf(ptr, "%s=%s",node->name, node->value) + 1;
	}

	TRACE;
	
	/* End with a double NUL */
	ptr += 2;
	//ptr ++;
	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);
	

	/* Set header value */
	header->magic = 0x464C5348; 

	//F9不关注此字段
	header->config_ncdl = 0x0;   //it can from  envram

	//F9不关注此字段，直接设置为0，省去各种平台之间的大小端转换
	header->config_refresh = 0x0;
	
	header->crc_ver_init = htonl(NVRAM_VERSION << 8);/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	
	/* Set new CRC8 */
	header->crc_ver_init |= (crc = nvram_calc_crc(header ));/* 0:7 crc, 8:15 ver, 16:31 sdram_init */

	header->len = htonl(header->len);
	header->crc_ver_init = htonl(header->crc_ver_init);
	
	return 0;
}


int wirteBuffToFile(char *buf, int len, char *file)
{
	FILE *fp = NULL;
	int ret = 0;

	fp = fopen(file, "w");
	if(!fp)
		error_exit("fopen");

	ret = fwrite(buf , sizeof(char), len, fp);

	fclose(fp);

	if(ret != len)
	{
		printf("Write %s %d[Real length %d]\n", file, len, ret);
	}
	
	return ret != len;

}
/*
	Real work begin.
	If success return 0,
	others 	return -1.
*/
int mkFlashImage(void)
{
	int ret = 0;
	char *imageBug = NULL;
	char flashName[128] = {0};

	readnvram_default(READENVRAM);  //读入envram
	readnvram_default(READNVRAM);	//读入nvram
	readnvram_default(READFACTORY);	//读入需要修改的nvram

	

	//调试时暂时打开，程序完成了记得关闭
	//flashsize = 2*MB;

	if(!flashsize)
		error_exit("Could not Get \"flash_size\" From envram. Please add.\n\n");
	
	imageBug = malloc(flashsize + MORE_FLASH_SIZE);
	if(!imageBug)
		error_exit("malloc");
	memset(imageBug, 0XFF, flashsize + MORE_FLASH_SIZE);
	
	TRACE;
	//Fill cfe
	ret = fillBuffFromBoot(imageBug, 0x0, cfepath);		//读入boot
	if(ret)
	{
		printf("fillBUffFromFile error\n");
		free(imageBug);
		return -1;
	}
	TRACE;
	//Fill envram 
	ret = fillBuffEnvram(imageBug);
	TRACE;

	//Fill image
	ret = fillBuffFromImage(imageBug, BOOT_LENGTH_ON_FLASH, imagepath);
	if(ret)
	{
		printf("fillBUffFromFile error\n");
		free(imageBug);
		return -1;
	}

	TRACE;

	//Fill nvram 
	ret = fillBuffnvram(imageBug);

	//show_list(&nvramHead);

	memset(flashName,0x0,sizeof(flashName));
	makeFlashName(flashName);    //构造flash的文件名
	strcat(flashpath,flashName);
	printf("%-20s:%s\n","flashpath",flashpath);

	//OK, All done.write Flash image to file
	wirteBuffToFile(imageBug, flashsize, flashpath);	//将生成的flash写到指定路径

	// Out put create informations.
	//show_config();

	
	printf("All done!!!\n");
	printf("*********************************************************\n");

	/* Some clean works. */
	free(imageBug);
	freeNvramTupleList(&nvramHead);
	freeNvramTupleList(&envramHead);
	
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	char *optstring = "c:i:e:o:n:f:m:vh";	//需要的参数
	int option_index = 0;  
	int opt = 0;
	struct stat st;

	if(2 == __CONFIG_FLASH_SIZE__)
	{
		flashsize = 2 * MB;
	}
	else if(1 == __CONFIG_FLASH_SIZE__)
	{
		flashsize = 1 * MB;
	}
	else 
	{
		printf("unknow flash size\n");
		return -1;
	}


	while(1)
	{
		char *endptr;
		opt = getopt_long(argc, argv, optstring, long_options, &option_index);
		if(-1 == opt )
		{
			break;

		}

		switch(opt)
		{
			case 'c':
				strcpy(cfepath, optarg);
				break;
			case 'i':
				strcpy(imagepath, optarg);	
				break;
			case 'o':
				strcpy(flashpath, optarg);
				break;					
			case 'v':
				verbose = 1;
				break;
			case 'h': /* fall down */
			default:
				printf("opt=%c, show help\n", opt);
				show_usage();
				break;
		}
	}

	
	if(verbose)
	{
		show_config();
	}

	/* Fast Check  parameters. 检查输入的参数*/
	if(!strlen(cfepath) || !strlen(imagepath) ||!strlen(flashpath))
	{
		printf("Necessary arguments missing.[cfepath or imagepath or nvrampath or envrampath or flashpath] !!!\n\n");
		show_usage();
		return -1;
	}

	//检查输入文件的状态
	char *path;
	if(-1 == stat(path = cfepath, &st) ||
			-1 == stat(path = imagepath, &st)
			)
	{
		printf("%s is not exsit or error.\n", path );
		return -1;
	}

	/* All Important work is this.生成flash镜像的过程函数 */
	ret = mkFlashImage();


	if(ret)
	{
		/* Somethings wrong happen. */
		printf("mkFlashImage error.\n");
		return -1;
	}
	
	return 0;
}
