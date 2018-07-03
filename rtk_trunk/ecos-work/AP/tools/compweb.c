#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../apmib/apmib.h"

#define MAXFNAME	60

#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | ((((v>>8)&0xff)<<16)&0xff0000) | \
				((((v>>16)&0xff)<<8)&0xff00) | (((v>>24)&0xff)&0xff) )
#define WORD_SWAP(v) ((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))
#define __PACK__	__attribute__ ((packed))

/*
typedef struct _file_entry {
	char name[MAXFNAME];
	unsigned long size;
} file_entry_T;
typedef struct _header {
	unsigned char signature[4];
	unsigned long addr;
	unsigned long burn_addr;
	unsigned long len;
} HEADER_T, *HEADER_Tp;
*/

/////////////////////////////////////////////////////////////////////////////
static int compress(char *inFile, char *outFile)
{
	char tmpBuf[100];
	sprintf(tmpBuf, "chmod 666 %s", inFile);
	system(tmpBuf);
	//sprintf(tmpBuf, "bzip2 -9 -c %s > %s", inFile, outFile);
#ifdef HAVE_LZMA
	sprintf(tmpBuf, "./lzma e %s %s", inFile, outFile);
#else
	sprintf(tmpBuf, "gzip -9 -c %s > %s", inFile, outFile);
#endif
	system(tmpBuf);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
#if 0
static unsigned char CHECKSUM(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#endif

/////////////////////////////////////////////////////////////////////////////
static int lookfor_homepage_dir(FILE *lp, char *dirpath, int is_for_web)
{
	char file[MAXFNAME];
	char *p;
	struct stat sbuf;

	fseek(lp, 0L, SEEK_SET);
	dirpath[0] = '\0';

	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}
		if (is_for_web)
			p=strstr(file, "index.htm");

		else
			p=strrchr(file, '/');
		if (p) {

			*p = '\0';
			strcpy(dirpath, file);
			if(strlen(dirpath)>0 && dirpath[strlen(dirpath)-1]=='/')
				dirpath[strlen(dirpath)-1]='\0';
// for debug
//printf("Found dir=%s\n", dirpath);
			return 0;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
static void strip_dirpath(char *file, char *dirpath)
{
	char *p, tmpBuf[MAXFNAME];

	if ((p=strstr(file, dirpath))) {
		strcpy(tmpBuf, &p[strlen(dirpath)]);
		strcpy(file, tmpBuf);
	}
// for debug
//printf("adding file %s\n", file);
}


/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	char *outFile, *fileList;
	char *compFile="tmp_comp.htm";
	char *platform;
	char *tag;
	int fh;
	int tmp_fh;
	struct stat sbuf;
	struct stat tmp_sbuf;
	FILE *lp;
	char file[MAXFNAME];
	char tmpFile[100], dirpath[100];
	char buf[512];
	FILE_ENTRY_T entry;
	unsigned char	*p;
	int i, len, fd, nFile, is_web=1, pad=0;
	IMG_HEADER_T head;
#ifndef CONFIG_WEB_COMP_TWICE
	unsigned int offset=0;
#endif
	platform = argv[1];
	fileList = argv[2];
	outFile = argv[3];
//	if ( argc > 4)
//		is_web = 0;

	if(!strcmp(platform, "vpn"))
#if defined(CONFIG_RTL_8196B)
		tag = "w6bv";
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
                tag = "w6cv";
#else		
		tag = "webv";
#endif
	else if(!strcmp(platform, "gw"))
#if defined(CONFIG_RTL_8196B)
		tag = "w6bg";
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
                tag = "w6cg";
#else		
		tag = "webg";
#endif		
	else if(!strcmp(platform, "ap"))
#if defined(CONFIG_RTL_8196B)
		tag = "w6ba";
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
                tag = "w6ca";
#else
		tag = "weba";
#endif
	else if(!strcmp(platform, "cl"))
#if defined(CONFIG_RTL_8196B)
		tag = "w6bc";
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
                tag = "w6cc";
#else		
		tag = "webc";
#endif
	else if(!strcmp(platform, "signature")) {
		tag = argv[4];
	}
	else{
		printf("unknow platform!\n");
		return 0;
	}
	if(strstr(fileList,"jffs2"))
	{
		bzero(tmpFile,sizeof(tmpFile));
		strcpy(tmpFile,fileList);
	}
	else
	{
#if defined(CONFIG_WEB_COMP_TWICE) || !defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)
		fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
#else
		sprintf(tmpFile, "%sXXXXXX",  outFile);
		fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
#endif
		if (fh == -1) {
			printf("Create output file error %s!\n", outFile );
			exit(1);
		}
		lseek(fh, 0L, SEEK_SET);
	
		if ((lp = fopen(fileList, "r")) == NULL) {
			printf("Can't open file list %s\n!", fileList);
			exit(1);
		}
		if (lookfor_homepage_dir(lp, dirpath, is_web)<0) {
			printf("Can't find home.asp page\n");
			fclose(lp);
			exit(1);
		}
	
		fseek(lp, 0L, SEEK_SET);
		nFile = 0;
		while (fgets(file, sizeof(file), lp) != NULL) {
			if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
				*p = '\0';
			}
			if (*file == '\0') {
				continue;
			}
			if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
				continue;
			}
#if defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)	
			if(strstr(file,"lzma"))
				continue;
				
			strcpy(entry.name, file);
			strip_dirpath(entry.name, dirpath);
			entry.size_uncomp = DWORD_SWAP(sbuf.st_size);

			/*compress each file first*/	
			if ( compress(file, compFile) < 0) {
				printf("compress file error!\n");
				close(fh);	
				fclose(lp);
				exit(1);
			}
			
			if (stat(compFile, &tmp_sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
				continue;
			}
			
			/*save the compressed file*/
			entry.size = DWORD_SWAP(tmp_sbuf.st_size);
#ifndef	CONFIG_WEB_COMP_TWICE		
			entry.offset = DWORD_SWAP(offset);
#endif	
			if ( write(fh, (const void *)&entry, sizeof(entry))!=sizeof(entry) ) {
				printf("Write file failed!\n");
				close(fh);				
				fclose(lp);
				exit(1);
			}
			
			chmod(compFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			tmp_fh = open(compFile, O_RDONLY);
			if(tmp_fh<0){
				printf("Can't open file %s\n", compFile);
				close(fh);
				fclose(lp);
				exit(1);
			}
			lseek(tmp_fh, 0L, SEEK_SET);
			i = 0;
			while ((len = read(tmp_fh, buf, sizeof(buf))) > 0) {
				if ( write(fh, (const void *)buf, len)!=len ) {
					printf("Write file failed!\n");
					close(fh);
					close(tmp_fh);
					fclose(lp);
					exit(1);
				}
				i += len;
			}
			close(tmp_fh);
			if ( i != tmp_sbuf.st_size ) {
				printf("Size mismatch in file %s!\n", compFile);
			}
#ifndef CONFIG_WEB_COMP_TWICE
			else{
				offset  = offset + sizeof(FILE_ENTRY_T) + i;
				//printf("name:%s size:%d un_comp:%d offset:%d\n",entry.name,DWORD_SWAP(entry.size),DWORD_SWAP(entry.size_uncomp),DWORD_SWAP(entry.offset));
			}
#endif
#else
			if ((fd = open(file, O_RDONLY)) < 0) {
				printf("Can't open file %s\n", file);
				exit(1);
			}
			lseek(fd, 0L, SEEK_SET);
			strip_dirpath(file, dirpath);
			strcpy(entry.name, file);
			entry.size = DWORD_SWAP(sbuf.st_size);
			if ( write(fh, (const void *)&entry, sizeof(entry))!=sizeof(entry) ) {
				printf("Write file failed!\n");
				close(fh);				
				fclose(lp);
				exit(1);
			}
			i = 0;
			while ((len = read(fd, buf, sizeof(buf))) > 0) {
				if ( write(fh, (const void *)buf, len)!=len ) {
					printf("Write file failed!\n");
					exit(1);
				}
				i += len;
			}
			close(fd);
			if ( i != sbuf.st_size ) {
				printf("Size mismatch in file %s!\n", file );
			}
#endif
			nFile++;		
			
		
		}

		fclose(lp);
		close(fh);
		sync();

	// for debug -------------
#if 0
	sprintf(tmpFile, "cp %s web.lst -f", outFile);
	system(tmpFile);
#endif
	//-------------------------
	/*for second compress */
#if defined(CONFIG_WEB_COMP_TWICE) || !defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)
	
		sprintf(tmpFile, "%sXXXXXX",  outFile);
		if(mkstemp(tmpFile)== -1) {
			printf("mk temp file error!\n");
			unlink(compFile);
			exit(1);
		}

		if ( compress(outFile, tmpFile) < 0) {
			printf("compress file error!\n");
			unlink(compFile);
			exit(1);
		}	
#endif
	}

#ifndef CONFIG_WEB_COMP_TWICE	
	chmod(tmpFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
#endif
	// append header
	if (stat(tmpFile, &sbuf) != 0) {
		printf("Create file error!\n");
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}
	if((sbuf.st_size+1)%2)
		pad = 1;
	p = malloc(sbuf.st_size + 1 + pad);
	if ( p == NULL ) {
		printf("allocate buffer failed!\n");
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}	
	memset(p, 0 , sbuf.st_size + 1);
	memcpy(head.signature, tag, 4);
	head.len = sbuf.st_size + 1 + pad;
#ifdef JFFS2_SUPPORT
	printf("%s:%d len=%d leftLen=%d\n",__FILE__,__LINE__,sbuf.st_size,CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET);
	if(CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET<sbuf.st_size)
#else
	printf("%s:%d len=%d leftLen=%d\n",__FILE__,__LINE__,head.len+sizeof(IMG_HEADER_T),CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET);
	if(CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET<head.len+sizeof(IMG_HEADER_T))
#endif
	{
		printf("webpage over size!!!!!Please remove some unused html files\n or enable 'Flash Mapping Support'in menuconfig and enlarge the offset of Ecos image!\n");
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		return -1;
	}

	head.len = DWORD_SWAP(head.len);
	head.startAddr = DWORD_SWAP(WEB_PAGE_OFFSET);
	head.burnAddr = DWORD_SWAP(WEB_PAGE_OFFSET);
		
	if ((fd = open(tmpFile, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile);
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}
	lseek(fd, 0L, SEEK_SET);
	if ( read(fd, p, sbuf.st_size) != sbuf.st_size ) {
		printf("read file error!\n");
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}
	close(fd);

	p[sbuf.st_size + pad] = CHECKSUM(p, (sbuf.st_size+pad));
	
	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", outFile );
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}

	if ( write(fh, &head, sizeof(head)) != sizeof(head)) {
		printf("write header failed!\n");
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}

	if ( write(fh, p, (sbuf.st_size+1+pad) ) != (sbuf.st_size+1+pad)) {
		printf("write data failed!\n");
		free(p);
		unlink(tmpFile);
		unlink(compFile);
		exit(1);
	}

	close(fh);
	//chmod(outFile,  DEFFILEMODE);
	chmod(outFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);

	sync();

	free(p);
	unlink(tmpFile);
#if defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)
	unlink(compFile);
#endif
	return 0;
}
