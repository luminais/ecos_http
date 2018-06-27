/*
 * Modified by Akihiro Tominaga. (tomy@sfc.wide.ad.jp)
 */
/*
 * Generalized hash table ADT
 *
 * Provides multiple, dynamically-allocated, variable-sized hash tables on
 * various data and keys.
 *
 * This package attempts to follow some of the coding conventions suggested
 * by Bob Sidebotham and the AFS Clean Code Committee of the
 * Information Technology Center at Carnegie Mellon.
 *
 *
 *
 * Copyright (c) 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon.  Carnegie Mellon
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#include <stdlib.h>
#include "hash.h"

#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

static unsigned	  hash_func();

/*
 * Generic hash function to calculate a hash code from the given string.
 *
 * This function returns the sum of the squares of all the bytes.  It is
 * assumed that this result will be used as the "hashcode" parameter in
 * calls to other functions in this package.  These functions automatically
 * adjust the hashcode for the size of each hashtable.
 *
 * This algorithm probably works best when the hash table size is a prime
 * number.
 *
 * This may not be the world's best hash function.  I'm open to other
 * suggestions.  The programmer is more than welcome to supply his/her own
 * hash function as that is one of the design features of this package.
 */
static unsigned
hash_func(string, len)
  char *string;
  register unsigned len;
{
  register unsigned sum, value = 0;

  sum = 0;
#ifdef CONFIG_RTL_819X
  if((len>6) || (string==NULL))  //maybe can avoid coredump
  {
  	//diag_printf("%s:%d len=%d\n",__FUNCTION__,__LINE__,len);
  	return 390151;  // 6*255*255+1
  }  
#endif
  for (; len > 0; len--) {
    value = (unsigned) (*string++ & 0xFF);
    sum += value * value;
  }
  return(sum);
}

/*
 * Insert the data item "element" into the hash table using "hashcode"
 * to determine the bucket number, and "compare" and "key" to determine
 * its uniqueness.
 *
 * If the insertion is successful 0 is returned.  If a matching entry
 * already exists in the given bucket of the hash table, or some other error
 * occurs, -1 is returned and the insertion is not done.
 */
int hash_ins(struct hash_tbl *hashtable, char *str, unsigned length, int (*compare)(), 
		hash_datum *key, hash_datum *element)  
{
  unsigned hashcode;
  struct hash_member *memberptr = NULL, *temp = NULL;

  hashcode = hash_func(str, length);
#ifdef CONFIG_RTL_819X
  if(hashcode>=390151)
  {
  	//diag_printf("%s:%d ######\n",__FUNCTION__,__LINE__);
  	return -1;
  }
#endif
  hashcode %= HASHTBL_SIZE;
  if (hash_exst(hashtable, hashcode, compare, key)) {
    return(-1);	/* At least one entry already exists */
  }
  memberptr = (hashtable->head)[hashcode];
  if ((temp = (struct hash_member *) calloc(1, sizeof(struct hash_member)))) {
    temp->data = element;
    temp->next = memberptr;
    (hashtable->head)[hashcode] = temp;
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, sizeof(struct hash_member));
#endif

    return(0);	/* Success */
  } else {
    return(-1);	/* calloc failed! */
  }
}


/*
 * Returns TRUE if at least one entry for the given key exists; FALSE
 * otherwise.
 */
int hash_exst(struct hash_tbl *hashtable, unsigned hashcode,  
		int (*compare)(), hash_datum *key)
{
  register struct hash_member *memberptr = NULL;

  memberptr = (hashtable->head)[hashcode % HASHTBL_SIZE];
  while (memberptr) {
    if ((*compare)(key, memberptr->data)) {
      return TRUE;		/* Entry does exist */
    }
    memberptr = memberptr->next;
  }
  return FALSE;			/* Entry does not exist */
}


/*
 * Locate and return the data entry associated with the given key.
 *
 * If the data entry is found, a pointer to it is returned.  Otherwise,
 * NULL is returned.
 */
hash_datum *hash_find(struct hash_tbl *hashtable, char *str, unsigned length, 
			int (*compare)(), hash_datum *key)
{
  unsigned hashcode;
  struct hash_member *memberptr = NULL;

  hashcode = hash_func(str, length);
#ifdef CONFIG_RTL_819X
  if(hashcode>=390151)
  {
  	//diag_printf("%s:%d ######\n",__FUNCTION__,__LINE__);
  	return NULL;
  }
#endif
  memberptr = (hashtable->head)[hashcode % HASHTBL_SIZE];
  while (memberptr) {
    if ((*compare)(key, memberptr->data)) {
      return (memberptr->data);
    }
    memberptr = memberptr->next;
  }
  return NULL;
}


/*
 * find the data and remove it from list.
 * 
 * If the data entry is found, a pointer to it is returned.  Otherwise,
 * NULL is returned.
 */
hash_datum *hash_pickup(struct hash_tbl *hashtable, unsigned hashcode, 
		int (*compare)(), hash_datum *key)
{
  struct hash_member *memberptr = NULL, *previous = NULL;
  hash_datum *result = NULL;

  memberptr = (hashtable->head)[hashcode % HASHTBL_SIZE];
  while (memberptr) {
    if ((*compare)(key, memberptr->data)) {
      if (memberptr == *hashtable->head) {
	*hashtable->head = memberptr->next;
      } else {
      		if(previous)
			previous->next = memberptr->next;
      }
      result = memberptr->data;
      free(memberptr);
	  
#ifdef ECOS_DBG_STAT
      dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct hash_member));
#endif

      return(result);
    }
    previous = memberptr;
    memberptr = memberptr->next;
  }
  return (NULL);
}


/*
 * Delete all data elements which match the given key.  If at least one
 * element is found and the deletion is successful, 0 is returned.
 * If no matching elements can be found in the hash table, -1 is returned.
 */
int hash_del(struct hash_tbl *hashtable, char *str, unsigned length, 
		int (*compare)(), hash_datum *key, int (*free_data)())
{
  unsigned hashcode;
  struct hash_member *memberptr = NULL, *previous = NULL, *tempptr = NULL;
  int retval = -1;

  hashcode = hash_func(str, length);
  #ifdef CONFIG_RTL_819X
  if(hashcode>=390151)
  {
  	//diag_printf("%s:%d #########\n",__FUNCTION__,__LINE__);
  	return retval;
  }
  #endif
  hashcode %= HASHTBL_SIZE;

  memberptr = (hashtable->head)[hashcode];
  while (memberptr != NULL && (*compare)(key, memberptr->data)) {
    (hashtable->head)[hashcode] = memberptr->next;
    /*
     * Stop recursively deleting the whole list!
     */
    memberptr->next = NULL;
    if (free_data != NULL)
		free_data(memberptr);
    memberptr = (hashtable->head)[hashcode];
    retval = 0;
  }

  /*
   * Now traverse the rest of the list
   */
  previous = memberptr;
  if (memberptr) {
    memberptr = memberptr->next;
  }
  while (memberptr) {
    if ((*compare)(key, memberptr->data)) {
      tempptr = memberptr;
      previous->next = memberptr = memberptr->next;
      /*
       * Put the brakes
       */
      tempptr->next = NULL;
      if (free_data != NULL) 
	  	free_data(tempptr);
      retval = 0;
    } else {
      previous = memberptr;
      memberptr = memberptr->next;
    }
  }
  return retval;
}
