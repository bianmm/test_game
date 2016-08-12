#include <string.h>

#include "shm_util.h"

int shm_attach(int shm_id, void** shm_address)
{
	if (shm_id < 0)
	{
		return -1;
	}

	if (shm_address == NULL)
	{
		return -1;
	}

	*shm_address = shmat(shm_id, NULL, SHM_RDONLY);
	if (*shm_address == (void*)-1)
	{
		return -1;
	}

	return 0;
}


int shm_detach(const void* shm_address)
{
	if (shm_address != NULL)
	{
		return shmdt(shm_address);
	}
	else
	{
		return 0;
	}
}

int shm_destroy(int shm_id)
{
	return shmctl(shm_id, IPC_RMID, NULL);
}

