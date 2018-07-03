#ifndef __VERSION_H__
#define __VERSION_H__

//???????
#define COMPANY_NAME "Tenda"

/*sdk version*/
#define W311R_ECOS_BV	"V5.100.27.21"

/*???????*/
#ifdef __CONFIG_HW_VER__
#define W311R_ECOS_HV	__CONFIG_HW_VER__
#else
#define W311R_ECOS_HV	""
#endif


#if (8 == __CONFIG_MEMORY_SIZE__)
	#define W311R_ECOS_SV "V12.02.01.41"
#elif (16 == __CONFIG_MEMORY_SIZE__)
	#define W311R_ECOS_SV "V12.01.01.38"
#elif (32 == __CONFIG_MEMORY_SIZE__)
	#define W311R_ECOS_SV "V02.03.01.22"
#else
	#define W311R_ECOS_SV "V12.02.01.37"
#endif


/*???????e?? multi*/
#define NORMAL_WEB_VERSION	__CONFIG_WEB_VERSION__

#endif/*__VERSION_H__*/
