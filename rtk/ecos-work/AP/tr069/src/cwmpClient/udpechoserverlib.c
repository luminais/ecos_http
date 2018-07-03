#include <sys/types.h>
//#include <sys/ipc.h>
#include <stdio.h>
#include "udpechoserverlib.h"

#define SHM_PROJ_ID	'S'

#if defined(ECOS)
char shmem[64] = {0};
#endif

int initShmem( void **t, int s, char *name )
{
#if !defined(ECOS)
	key_t key;
	int shm_id;

	if(t==NULL || s<=0 || name==NULL)
		return -1;
		
	key = ftok(name, SHM_PROJ_ID);
	if(key==-1)
	{
		//perror("ftok");
		return -1;
	}
	
	shm_id=shmget(key, s, IPC_CREAT);
	if(shm_id==-1)
	{
		//perror("shmget");
		return -1;		
	}
	
	*t=shmat(shm_id,NULL,0);
	if( *t==(void *)-1 )
	{
		//perror("shmget");
		return -1;		
	}
	
	memset( *t, 0, s );
	
#else
	if(t==NULL || s<=0 || name==NULL)
		return -1;

	*t = shmem;
	memset(shmem, 0, sizeof(shmem));
#endif	

	return 0;
}

int getShmem( void **t, int s, char *name )
{
#if !defined(ECOS)
	key_t key;
	int shm_id;

	if(t==NULL || s<=0 || name==NULL)
		return -1;

	key = ftok(name, SHM_PROJ_ID);
	if(key==-1)
	{
		//perror("ftok");
		return -1;
	}
	
	shm_id=shmget(key, s , 0);
	if(shm_id==-1)
	{
		//perror("shmget");
		return -1;		
	}
	
	*t=shmat(shm_id,NULL,0);
	if( *t==(void *)-1 )
	{
		//perror("shmget");
		return -1;		
	}
	
#else
	if(t==NULL || s<=0 || name==NULL)
		return -1;

	*t = shmem;
#endif

	return 0;
}

int detachShmem( void *t )
{
#if !defined(ECOS)
	if( shmdt(t)==-1 )
	{
		//perror( "shmdt" );
		return -1;
	}

#else
	//memset(shmem, 0, sizeof(shmem));
#endif

	return 0;
}
