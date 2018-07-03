/*
 * Flash CGI macro function.
 *
 * Copyright (C) 2010, Tenda Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Tenda Corporation.
 *	
 * $Id: flash_cgi.h,v 1.0 2010/08/27   Exp $
 * $author: stanley 
 * $description: 
 *			  
 */
 #ifndef __FLASH_CGI_H__
 #define __FLASH_CGI_H__
#include <bcmnvram.h>


/*
 * extern char * nvram_get(const char *name);
 *
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */

#define _GET_VALUE(N,V)	V=nvram_safe_get(N)

#define _SAFE_GET_VALUE(N,V)  V=nvram_safe_get(N)



/*
 * extern int nvram_set(const char *name, const char *value);
 *
 * Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 */
 static inline char *wl02wl(char *name)
{
 	static char wl[64];
	char *p;

	if (strncmp(name, "wl0_", 4) != 0){
		return name;
	}else{
		p = &name[4];
		snprintf(wl,64,"wl_%s",p);
		return wl;
	}
}
#if 0
//下面这样做是为了备份/恢复设置时无线设置无法恢复功能
#define _SET_VALUE(N,V)	do{\
	if (strncmp(N,"wl0_", 4)!=0) \
	{\
		nvram_set(N,V); \
	}\
	else\
	{ \
		nvram_set(N,V); \
		nvram_set(wl02wl(N),V); \
	} \
}while(0)
#else
#define _SET_VALUE(N,V)	do{\
		nvram_set(N,V); \
}while(0)
#endif
/*
 * extern int nvram_unset(const char *name);
 *
 * Unset an NVRAM variable. Pointers to previously set values
 * remain valid until a set.
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
#define _DEL_VALUE(N)	nvram_unset(N)

/*
 * extern int nvram_commit(void);
 *
 * Commit NVRAM variables to permanent storage. All pointers to values
 * may be invalid after a commit.
 * NVRAM values are undefined after a commit.
 * @return	0 on success and errno on failure
 */
#define _COMMIT()		nvram_commit()


/*
 * extern char *nvram_default_get(const char *name);
 *
 * Get default value for an NVRAM variable
 */

#define _GET_DEFAULT(N,V)		V=nvram_default_get(N)


/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */

#define _INV_MATCH(N,IV)		nvram_invmatch(N,IV)



/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
#define _MATCH(N,V)				nvram_match(N,V)


/* 
 * Get the value of an NVRAM variable.
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
#define _SAFE_GET(N)			nvram_safe_get(N)

extern void sys_restart(void);

#define _RESTART()	sys_restart()

extern void reset_wl_pwr_percent(void);

#define _SET_WL_PWR_PENCENT() reset_wl_pwr_percent()

extern void sys_restart2(void);

#define _RESTART_ALL()	sys_restart2()

#define _REBOOT()	sys_reboot()
#endif

