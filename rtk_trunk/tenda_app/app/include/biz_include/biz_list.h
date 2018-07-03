#ifndef BIZ_LIST_H
#define BIZ_LIST_H

/** 初始化list_node_t节点 */
#define INIT_LIST_NODE(node) {&(node), &(node)}

typedef struct list_node {
	struct list_node * next, * prev;
} list_node_t;

/** 得到member成员的偏移量 */
#define member_offset(type, member)	\
	((size_t)&(((type*)0)->member))
	
/** 从member_ptr指针计算出其所在type类型的首地址 */
#define type_list_entry(member_ptr, type, member) \
	(type*)((char*)member_ptr - member_offset(type, member))

/** 循环遍历head->next */
#define list_for_each_entry_next(pos, head, list) \
	for (pos = type_list_entry((head)->next, typeof(*pos), list); \
		 pos && (&pos->list != (head)); \
		 pos = type_list_entry(pos->list.next, typeof(*pos), list))

/** 循环遍历head->pre */
#define list_for_each_entry_prev(pos, head, list) \
	for (pos = type_list_entry((head)->prev, typeof(*pos), list); \
		 pos && (&pos->list != (head)); \
		 pos = type_list_entry(pos->list.prev, typeof(*pos), list))

/** 循环遍历head->next, 删除节点时使用 */
#define list_for_each_entry_safe(pos, n, head, list) \
	for (pos = type_list_entry((head)->next, typeof(*pos), list), \
		 n = type_list_entry((pos->list).next, typeof(*pos), list); \
		 pos && (&pos->list != (head)); \
		 pos = n, n = type_list_entry(n->list.next, typeof(*pos), list)) 


/** 初始化list_node节点 */
void init_list_node(list_node_t * list);
/** 将新节点new插入到prev和next之间 */
void add_node_to_list(list_node_t * new, list_node_t * prev, list_node_t * next);
/** 将新节点插入到head前边，即链表最前边 */
void add_node_to_list_head(list_node_t * new, list_node_t * head);
/** 将新节点插入到head后边，即链表结尾 */
void add_node_to_list_tail(list_node_t * new, list_node_t * head);
/** 将节点和链表中分离出来 */
void detach_node_from_list(list_node_t * node);
/** 链表是否为空 */
int is_list_empty(list_node_t * head);

#endif
