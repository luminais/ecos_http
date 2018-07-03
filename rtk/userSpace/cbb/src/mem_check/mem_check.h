#ifndef __MEM_CHECK_H__
#define __MEM_CHECK_H__

typedef struct _ptr_node
{
	void  *ptr;
	size_t block;
	struct _ptr_node *next;
}ptr_node;

typedef struct _mem_node  
{   
    ptr_node *ptr_head;
    char *fun_name;
    struct _mem_node *next;
} mem_node;  

#define MAX_MEM_NODE    (20*1024)
void *tenda_malloc(size_t elem_size, char *func);  
void *tenda_calloc(size_t count, size_t elem_size, char *func);  
void tenda_free(void *ptr);  
void show_block(int argc,char **argv);  
#endif
