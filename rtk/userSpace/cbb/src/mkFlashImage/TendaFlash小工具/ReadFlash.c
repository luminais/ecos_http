/********************************************************************
作用:		用于从flash烧录文件中提取nvram,envram等参数，并以txt文档的方式输出
生成日期:	2017年5月9日

********************************************************************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>


#define MB (1024 *1024)

//flash的大小
int flashsize = 0;		


//定义各个段的起始地址
#define BOOTSTARTADDR	0x00000000
#define ENVRAMSTARTADDR	(0x6000)	//24K
#define NVRAMSTARTADDR	((flashsize) - (0x8000))  //flash的后32K用于存放nvram
#define IMAGESTARTADDR	(0x8000) // 32K


//定义各个区段的大小
#define BOOTLEN		(0x6000)	//boot存放的长度为24K空间
#define ENVRAMLEN	(0x2000)	//envram存放的空间为8K
#define NVRAMLEN	(0x8000)	//nvram存放空间为32K
#define IMAGELEN	((NVRAMSTARTADDR) - (IMAGESTARTADDR))	//存放镜像的地址用nvram的起始地址减去image的起始地址得到

//定义生成文件的名字
#define ENVRAMNAME	"envram.txt"		
#define NVRAMNAME	"nvram.txt"
#define BOOTNAME	"boot.bin"
#define IMAGENAME	"RTK0_MAGIC_upgradefile.bin"

//int readfile(char *buf,FILE *fp,int baseaddr,int len)


//读取envram 读取成功返回0，失败返回-1
int readenvram(FILE *fp)
{
	//printf("=================nvram show=================\n\n");
	int i = 0;
	int cnt = 0;
	char ch;
	char str[256] = {0};
	FILE *writefp = NULL;

	if(NULL == fp)
	{
		printf("argument is NULL\n");
		return -1;
	}

	memset(str,0x0,sizeof(str));

	writefp = fopen(ENVRAMNAME,"w+");
	if(NULL == writefp)
	{
		perror("writefp");
		return -1;
	}

	fseek(fp,20L,SEEK_CUR);		//偏移20字节跳过头部

	for(i = 0; i < ENVRAMLEN; i++)
	{
		fscanf(fp,"%c",&ch);
		if('\0' == ch)
		{
			if(0 != cnt)
			{
				//printf("%s\n",str);
				strcat(str,"\r\n");
				fwrite(str,1,cnt + 2,writefp);
			}
			cnt = 0;
			memset(str,0x0,sizeof(str));

			fscanf(fp,"%c",&ch);
			if('\0' == ch)
			{
				break;
			}
		}

		memcpy(str + cnt,&ch,1);
		cnt++;
	}

	
	fclose(writefp);
	//printf("\n=================envram show=================\n");
	
	return 0;
}





//读取nvram配置
int readnvram(FILE *fp)
{
	//printf("=================nvram show=================\n\n");
	int i = 0;
	int cnt = 0;
	char ch;
	char str[256] = {0};
	FILE *writefp = NULL;

	if(NULL == fp)
	{
		printf("argument is NULL\n");
		return -1;
	}

	memset(str,0x0,sizeof(str));

	writefp = fopen(NVRAMNAME,"w+");
	if(NULL == writefp)
	{
		perror("writefp");
		return -1;
	}

	fseek(fp,20L,SEEK_CUR);		//偏移20字节跳过头部

	for(i = 0; i < NVRAMLEN; i++)
	{
		fscanf(fp,"%c",&ch);
		if('\0' == ch)
		{
			if(0 != cnt)
			{
				//printf("%s\n",str);
				strcat(str,"\r\n");
				fwrite(str,1,cnt + 2,writefp);
			}
			cnt = 0;
			memset(str,0x0,sizeof(str));

			fscanf(fp,"%c",&ch);
			if('\0' == ch)
			{
				break;
			}
		}

		memcpy(str + cnt,&ch,1);
		cnt++;
	}

	
	fclose(writefp);
	//printf("\n=================envram show=================\n");
	
	return 0;
}



//从flash中读写boot
int readboot(FILE *fp)
{
	FILE *writefp = NULL;
	char *buf = NULL;
	int cnt = 0;

	if(NULL == fp)
	{
		printf("fp is NULL\n");
		return -1;
	}

	writefp = fopen(BOOTNAME,"w+");
	if(NULL == writefp)
	{
		perror("fopen");
		return -1;
	}
	
	buf = malloc(BOOTLEN + 8);	//多申请8个字节
	if(NULL == buf)
	{
		printf("malloc buf fail\n");
		fclose(writefp);
		return -1;
	}
	memset(buf,0x0,BOOTLEN + 8);

	cnt = fread(buf,1,BOOTLEN,fp);
	if(cnt ==fwrite(buf,1,cnt,writefp))
	{
		printf("make boot succee\n");
	}

	fclose(writefp);
	free(buf);
	return 0;
}

//从flash中提取出升级文件
int readimage(FILE *fp)
{
#if 0
	FILE *writefp = NULL;
	char *buf = NULL;
	int cnt = 0;
	
	if(NULL == fp)
	{
		printf("fp is NULL\n");
		return -1;
	}

	writefp = fopen(IMAGENAME,"w+");
	if(NULL == writefp)
	{
		perror("writefp");
		return -1;
	}

	buf = malloc(IMAGELEN + 8);		//多申请8个字节
	if(NULL == buf)
	{
		printf("malloc buf error!\n");
		fclose(writefp);
		return -1;
	}
	memset(buf,0x0,sizeof(buf));

	cnt = fread(buf,1,IMAGELEN,fp);
	printf("cnt:%d\n",cnt);
	if(cnt == fwrite(buf,1,cnt,writefp))
	{
		printf("make image succee\n");
	}

	free(buf);
	fclose(writefp);
	return 0;
#endif
}

void showusage(void)
{
	printf("\nUsage:./a.out [option] [flashpath] [flashsize]\n");
	
	printf("[option]\n");
	printf("\tenvram:get all envram config\n");
	printf("\tnvram:get all nvram config\n");
	printf("\tboot:get boot.bin from flash.bin\n");

	printf("[flashpath]\n");
	printf("\tthe path of input file\n");

	printf("[flashsize]\n");
	printf("\tthe size of input file\n");
	printf("\t1:size of input file is 1M\n");
	printf("\t2:size of input file is 2M\n");

	printf("example:\n");
	printf("./a.out envram flash.bin 2\n");
	printf("get all envram config from flash.bin,the size of flash.bin is 2M\n");

	return ;
}


//输入参数	./a.out opiont(envram or nvram) flashpath flashsize
int main(int argc,char *argv[])
{
	int i = 0;
	char opiont[32] = {0};
	char flashpath[256] = {0};	//存放flash路径
	char *buf = NULL;		//buf用于存放需要的数据
	FILE *readfp = NULL;
	FILE *writefp = NULL;
	char outputname[256] = {0};		//生成新文件的名字
	char line[1024] = {0};
	
	if(argc < 3)
	{
		//printf("input error\n");
		showusage();
		
		return -1;
	}

	memset(opiont,0x0,sizeof(opiont));
	strncpy(opiont,argv[1],sizeof(opiont));
	
	memset(flashpath,0x0,sizeof(flashpath));
	strncpy(flashpath,argv[2],sizeof(flashpath));

	flashsize = (atoi(argv[3])) *MB;

	buf = malloc(flashsize);
	if(NULL == buf)
	{
		printf("malloc error\n");
		return -1;
	}
	memset(buf,0x0,flashsize);
	
	printf("================input argument=======================\n");
	printf("opiont:%s\n",argv[1]);
	printf("flashpath:%s\n",argv[2]);
	printf("flash size:%d\n",flashsize);
	printf("================input argument=======================\n");

	readfp = fopen(flashpath,"rb");
	if(NULL == readfp)
	{
		printf("fopen fail\n");
		free(buf);
		return -1;
	}

	if(0 == strcmp(opiont,"envram"))
	{
		printf("read envram\n");
		fseek(readfp,ENVRAMSTARTADDR,SEEK_SET);		//偏移到24K的地方
		readenvram(readfp);
	}
	else if(0 == strcmp(opiont,"nvram"))
	{
		printf("read nvram\n");
		fseek(readfp,NVRAMSTARTADDR,SEEK_SET);
		readnvram(readfp);
	}
	else if(0 == strcmp(opiont,"boot"))
	{
		printf("read boot\n");
		fseek(readfp,BOOTSTARTADDR,SEEK_SET);
		readboot(readfp);
	}
	else if(0 == strcmp(opiont,"image"))
	{
		//printf("read image\n");
		//fseek(readfp,IMAGESTARTADDR,SEEK_SET);
		//readimage(readfp);
	}
	else
	{
		printf("unknow what to do\n");
		fclose(readfp);
		free(buf);
		return -1;
	}

	fclose(readfp);
	free(buf);

	return 0;
}





















