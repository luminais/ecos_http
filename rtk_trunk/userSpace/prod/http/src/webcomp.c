/*
 * webcomp -- Compile web pages into C source
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: webcomp.c,v 1.3 2002/10/24 14:44:50 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	Usage: webcomp prefix filelist >webrom.c
 *
 *	filelist is a file containing the pathnames of all web pages
 *	prefix is a path prefix to remove from all the web page pathnames
 *	webrom.c is the resulting C source file to compile and link.
 */

/********************************* Includes ***********************************/
#include	"wsIntrn.h"

/**************************** Forward Declarations ****************************/
#if defined( __CONFIG_TENDA_MULTI__) && defined(__CONFIG_WEB_VERSION__)
static int compile0(char_t *fileList, char_t *prefix);
#endif
static int 	compile(char_t *fileList, char_t *prefix);
static void usage();

/*********************************** Code *************************************/
/*
 *	Main program for webpack test harness
 */

int gmain(int argc, char_t* argv[])
{
	char_t		*fileList, *prefix;

	fileList = NULL;

	if (argc != 3) {
		usage();
	}

	prefix = argv[1];
	fileList = argv[2];
#if defined( __CONFIG_TENDA_MULTI__) && defined(__CONFIG_WEB_VERSION__)
#if defined(__NEED_DO_COMPILE0__)
	if (compile0(fileList, prefix) < 0) {
		return -1;
	}
#endif	//__NEED_DO_COMPILE0__,define in tenda_httpd/Makefile
#endif	
	if (compile(fileList, prefix) < 0) {
		return -1;
	}
	return 0;
}

#if defined( __CONFIG_TENDA_MULTI__) && defined(__CONFIG_WEB_VERSION__)
static char web_files_tmp[] = "web_files_tmp";

static char *lang_en = "lang/en";
static char *lang_de = "lang/de",*lang_fr = "lang/fr",*lang_it = "lang/it"; //en_01
static char *lang_es = "lang/es",*lang_pt = "lang/pt";											//en_02
static char *lang_ru = "lang/ru",*lang_tr = "lang/tr";											//en_03

static int compile0(char_t *fileList, char_t *prefix)
{
	gstat_t			sbuf;
	FILE			*lp,*lp2;
	time_t			now;
	char_t			file[FNAMESIZE];
	char_t			*cp, *sl;
	char			buf[512];
	unsigned char	*p;
	int				j, i, len, fd, nFile;
	int en_need = 1;

	int en_01 = 0, en_02 = 0, en_03 = 0;
	
	if(strcmp(NORMAL_WEB_VERSION,"en_01") == 0){
		en_01 = 1;
	}
	else if(strcmp(NORMAL_WEB_VERSION,"en_02") == 0){
		en_02 = 1;
	}
	else if(strcmp(NORMAL_WEB_VERSION,"en_03") == 0){
		en_03 = 1;
	}
	
/*
 *	Open list of files
 */
	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}

	if ((lp2 = fopen(web_files_tmp, "w")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", web_files_tmp);
		fclose(lp);
		return -1;
	}

/*
 *	Open each input file and compile each web page
 */
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
	
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		} 
/*
 *		Remove the prefix and add a leading "/" when we print the path
 */
		if (strncmp(file, prefix, gstrlen(prefix)) == 0) {
			cp = &file[gstrlen(prefix)];
		} else {
			cp = file;
		}
		while((sl = strchr(file, '\\')) != NULL) {
			*sl = '/';
		}
		if (*cp == '/') {
			cp++;
		}

//filter what we don't need
		if(strncmp(cp, "lang", gstrlen("lang")) == 0){
			if(en_01 && (strncmp(cp,lang_en,7) == 0 || strncmp(cp,lang_de,7) == 0 || strncmp(cp,lang_fr,7) == 0 || strncmp(cp,lang_it,7) == 0 || strstr(cp,"b28n.js") != NULL)){
				//continue;
			}else if(en_02 && (strncmp(cp,lang_en,7) == 0 || strncmp(cp,lang_es,7) == 0 || strncmp(cp,lang_pt,7) == 0 || strstr(cp,"b28n.js") != NULL)){
				//continue;
			}else if(en_03 && (strncmp(cp,lang_en,7) == 0 || strncmp(cp,lang_ru,7) == 0 || strncmp(cp,lang_tr,7) == 0 || strstr(cp,"b28n.js") != NULL)){
				//continue;
			}else if(en_03 == 0 && en_02 == 0 && en_01 == 0){
				//continue;
			}else{
				continue;
			}
		}
//
		fprintf(lp2,"%s\n",file);
	}

	fclose(lp);
	fclose(lp2);
	unlink(fileList);
	rename(web_files_tmp, fileList);
	return 0;
}
#endif

/******************************************************************************/
/*
 *	Output usage message
 */

static void usage()
{
	fprintf(stderr, "usage: webcomp prefix filelist >output.c\n");
	exit(2);
}

/******************************************************************************/
/*
 *	Compile the web pages
 */
/*
tenda add next parameter for multi language
*/
static int lang;
static int de_lang, en_lang, es_lang, fr_lang, it_lang, ko_lang, pl_lang, pt_lang, ru_lang, tr_lang; 

static int compile(char_t *fileList, char_t *prefix)
{
	gstat_t			sbuf;
	FILE			*lp;
	time_t			now;
	char_t			file[FNAMESIZE];
	char_t			*cp, *sl;
	char			buf[512];
	unsigned char	*p;
	int				j, i, len, fd, nFile;

//tenda add	
	char_t *cp_lang;
	lang = de_lang =  en_lang = es_lang = fr_lang = it_lang = ko_lang = pl_lang = pt_lang = ru_lang = tr_lang = 0;
//end

/*
 *	Open list of files
 */
	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}

	time(&now);
	fprintf(stdout, "/*\n * webrom.c -- Compiled Web Pages\n *\n");
	fprintf(stdout, " * Compiled by GoAhead WebCompile: %s */\n\n", 
		gctime(&now));
	fprintf(stdout, "#include \"wsIntrn.h\"\n\n");
	fprintf(stdout, "#ifndef WEBS_PAGE_ROM\n");
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");
	fprintf(stdout, "    { 0, 0, 0 },\n};\n");
	fprintf(stdout, "#else\n");

/*
 *	Open each input file and compile each web page
 */
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
	
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		} 
		if ((fd = gopen(file, O_RDONLY | O_BINARY)) < 0) {
			fprintf(stderr, "Can't open file %s\n", file);
			return -1;
		}
		fprintf(stdout, "static unsigned char page_%d[] = {\n", nFile);

		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			p = buf;
			for (i = 0; i < len; ) {
				fprintf(stdout, "    ");
				for (j = 0; p < &buf[len] && j < 16; j++, p++) {
					fprintf(stdout, "%3d,", *p);
				}
				i += j;
				fprintf(stdout, "\n");
			}
		}
		fprintf(stdout, "    0 };\n\n");

		close(fd);
		nFile++;
	}
	fclose(lp);

/*
 *	Now output the page index
 */
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");

	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
/*
 *		Remove the prefix and add a leading "/" when we print the path
 */
		if (strncmp(file, prefix, gstrlen(prefix)) == 0) {
			cp = &file[gstrlen(prefix)];
		} else {
			cp = file;
		}
		while((sl = strchr(file, '\\')) != NULL) {
			*sl = '/';
		}
		if (*cp == '/') {
			cp++;
		}
/*tenda add for multi lang*/		
		if(lang == 0){
			if(strncmp(cp, "lang", gstrlen("lang")) == 0){
				lang = 1;
			}
		}
/*
next parse line like this:
lang/it/wireless_set.xml
lang/b28n.js
*/		
		if(lang == 1 && strncmp(cp, "lang", gstrlen("lang")) == 0){
			cp_lang = cp+gstrlen("lang");
			if(*cp_lang == '/'){
					cp_lang++;
					//de_lang =  en_lang = es_lang = fr_lang = it_lang = ko_lang = pl_lang = pt_lang = ru_lang = tr_lang
					if(de_lang == 0 && cp_lang && strncmp(cp_lang, "de", gstrlen("de")) == 0){
						de_lang = 1;
					}
					if(en_lang == 0 && cp_lang && strncmp(cp_lang, "en", gstrlen("en")) == 0){
						en_lang = 1;
					}
					if(es_lang == 0 && cp_lang && strncmp(cp_lang, "es", gstrlen("es")) == 0){
						es_lang = 1;
					}
					if(fr_lang == 0 && cp_lang && strncmp(cp_lang, "fr", gstrlen("fr")) == 0){
						fr_lang = 1;
					}
					if(it_lang == 0 && cp_lang && strncmp(cp_lang, "it", gstrlen("it")) == 0){
						it_lang = 1;
					}
					if(ko_lang == 0 && cp_lang && strncmp(cp_lang, "ko", gstrlen("ko")) == 0){
						ko_lang = 1;
					}
					if(pl_lang == 0 && cp_lang && strncmp(cp_lang, "pl", gstrlen("pl")) == 0){
						pl_lang = 1;
					}
					if(pt_lang == 0 && cp_lang && strncmp(cp_lang, "pt", gstrlen("pt")) == 0){
						pt_lang = 1;
					}
					if(ru_lang == 0 && cp_lang && strncmp(cp_lang, "ru", gstrlen("ru")) == 0){
						ru_lang = 1;
					}
					if(tr_lang == 0 && cp_lang && strncmp(cp_lang, "tr", gstrlen("tr")) == 0){
						tr_lang = 1;
					}
			}
		}
/*end*/
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			fprintf(stdout, "    { T(\"/%s\"), 0, 0 },\n", cp);
			continue;
		}
		fprintf(stdout, "    { T(\"/%s\"), page_%d, %d },\n", cp, nFile, 
			sbuf.st_size);
		nFile++;
	}
	fclose(lp); 
	
	fprintf(stdout, "    { 0, 0, 0 },\n");
	fprintf(stdout, "};\n");
//tenda add	
	fprintf(stdout, "#ifdef __CONFIG_TENDA_MULTI__ \n");
	fprintf(stdout, "int de_lang = %d, en_lang = %d, es_lang = %d, fr_lang = %d,"
									"it_lang = %d, ko_lang = %d, pl_lang = %d, pt_lang = %d, ru_lang = %d, tr_lang = %d;\n",
										de_lang,en_lang,es_lang,fr_lang,it_lang,ko_lang,pl_lang,pt_lang,ru_lang,tr_lang);
	fprintf(stdout, "#endif \n");
//end	
	fprintf(stdout, "#endif /* WEBS_PAGE_ROM */\n");

	//fclose(lp);
	fflush(stdout);
	return 0;
}

/******************************************************************************/

