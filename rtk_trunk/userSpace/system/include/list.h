#ifndef __LIST_H__
#define __LIST_H__
/*!\file list.h
 * \brief Header file for doubly linked list manipulation functions.
 *
 * These functions implement doubly linked list.
 *
 */

/*! \brief structure that must be placed at the begining of any structure
 *         that is to be put into the linked list.
 */
typedef struct list_node {
	struct list_node *next;   /**< next pointer */
	struct list_node *prev;   /**< previous pointer */
} DlistNode;


/** Initialize a field in a structure that is used as the head of a list */
#define DLIST_HEAD_IN_STRUCT_INIT(field) do {\
      (field).next = &(field);               \
      (field).prev = &(field);               \
   } while (0)

/** Initialize a standalone variable that is the head of a list */
#define DLIST_HEAD_INIT(name) { &(name), &(name) }

/** Declare a standalone variable that is the head of the list */
#define DLIST_HEAD(name) \
	struct list_node name = DLIST_HEAD_INIT(name)


static inline void INIT_LIST_HEAD(struct list_node *list)
{
	list->next = list;
	list->prev = list;
}

/** Return true if the list is empty.
 *
 * @param head pointer to the head of the list.
 */
static inline int list_empty(const struct list_node *head)
{
	return ((head->next == head) && (head->prev == head));
}

static inline void list_expand(struct list_node *prev_head, struct list_node *next_head)
{
	prev_head->next = next_head->next;
	
	next_head->next = prev_head;
	next_head->prev = next_head->prev;
	
	prev_head->prev = next_head;
}

/** add a new entry after an existing list element
 *
 * @param new       new entry to be added
 * @param existing  list element to add the new entry after.  This could
 *                  be the list head or it can be any element in the list.
 *
 */
static inline void list_append(struct list_node *new_node, struct list_node *existing)
{
   existing->next->prev = new_node;

   new_node->next = existing->next;
   new_node->prev = existing;

   existing->next = new_node;
}


/** add a new entry in front of an existing list element
 *
 * @param new       new entry to be added
 * @param existing  list element to add the new entry in front of.  This could
 *                  be the list head or it can be any element in the list.
 *
 */
static inline void list_prepend(struct list_node *new_node, struct list_node *existing)
{
   existing->prev->next = new_node;

   new_node->next = existing;
   new_node->prev = existing->prev;

   existing->prev = new_node;
}


/** Unlink the specified entry from the list.
 *  This function does not free the entry.  Caller is responsible for
 *  doing that if applicable.
 *
 * @param entry existing list entry to be unlinked from the list.
 */
static inline void list_unlink(struct list_node *entry)
{
   entry->next->prev = entry->prev;
   entry->prev->next = entry->next;

	entry->next = 0;
	entry->prev = 0;
}

static inline void __list_del(struct list_node *prev,struct list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_node *list)
{
	__list_del(list->prev,list->next);
}
/** Return byte offset of the specified member.
 *
 * This is defined in stddef.h for MIPS, but not defined
 * on LINUX desktop systems.  Play it safe and just define
 * it here for all build types.
 */
#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


/** cast a member of a structure out to the containing structure
 *
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})


#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)


/** Create a for loop over all entries in the list.
 *
 * @param pos A variable that is the type of the structure which
 *            contains the DlistNode.
 * @param head Pointer to the head of the list.
 * @param member The field name of the DlistNode field in the
 *               containing structure.
 *
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))



#endif  /*__DLIST_H__ */
