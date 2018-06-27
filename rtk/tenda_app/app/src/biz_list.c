#include "biz_list.h"

void init_list_node(list_node_t * list)
{
	list->next = list;
	list->prev = list;
}

void add_node_to_list(
    list_node_t * new,
    list_node_t * prev,
    list_node_t * next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

void add_node_to_list_head(
	list_node_t * new,
	list_node_t * head)
{
	add_node_to_list(new, head, head->next);
}

void add_node_to_list_tail(
	list_node_t * new,
	list_node_t * head)
{
	add_node_to_list(new, head->prev, head);
}

void detach_node_from_list(list_node_t * node)
{
	list_node_t * prev = node->prev;
	list_node_t * next = node->next;	
	next->prev = prev;
	prev->next = next;
}

int is_list_empty( list_node_t * head)
{
	return head->next == head;
}



