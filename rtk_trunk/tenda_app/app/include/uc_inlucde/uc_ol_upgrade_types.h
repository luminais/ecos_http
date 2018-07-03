#ifndef __UC_OL_UPGRADE_TYPES_H__
#define __UC_OL_UPGRADE_TYPES_H__

#define MAX_PATH_SIZE		(256)

typedef struct memory_state_s {
	int  enough_memory;
} memory_state_t;

typedef struct saving_path_s {
		char file_name[MAX_PATH_SIZE];
} saving_path_t;

#endif
