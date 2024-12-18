// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	// TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("create_semaphore is not implemented yet");

	cprintf("MEOWWWWWW IN CREATE :D\n");

	struct semaphore newSem;

	newSem.semdata = (struct __semdata *)smalloc(semaphoreName, sizeof(struct __semdata), 1);
	if (newSem.semdata == NULL)
	{
		cprintf("couldn't allocate semdata:(\n");
	}

	newSem.semdata->count = (int)value;
	newSem.semdata->lock = (uint32)0;
	sys_init_queue(&newSem.semdata->queue);
	strcpy(newSem.semdata->name, semaphoreName);

	return newSem;
}

struct semaphore get_semaphore(int32 ownerEnvID, char *semaphoreName)
{
	cprintf("MEOWWWWWW IN GET :D\n");
	// TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("get_semaphore is not implemented yet");
	// Your Code is Here...
	struct semaphore sanfor;
	sanfor.semdata = (struct __semdata *)sget(ownerEnvID, semaphoreName);
	return sanfor;
}

void wait_semaphore(struct semaphore sem)
{
	cprintf("MEOWWWWWW IN WAIT :D\n");
	// TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//  panic("wait_semaphore is not implemented yet");
	uint32 key = 1;
	while (xchg(&sem.semdata->lock, key) != 0)
		;
	struct Env *curEnv = sys_get_cpu_proc();
	if (--sem.semdata->count < 0)
	{
		sys_enqueue(&sem.semdata->queue, curEnv);
		sem.semdata->lock = 0;
		curEnv->env_status = ENV_BLOCKED;
		sys_sched();
	}
	sem.semdata->lock = 0;
}

void signal_semaphore(struct semaphore sem)
{
	cprintf("MEOWWWWWW IN SIGNAL :D\n");
	// TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("signal_semaphore is not implemented yet");
	uint32 key = 1;
	while (xchg(&sem.semdata->lock, key) != 0)
		;

	struct Env *curEnv = sys_get_cpu_proc();
	if (++sem.semdata->count <= 0)
	{
		sys_remove_from_queue(&sem.semdata->queue, curEnv);
		sem.semdata->lock = 0;
		curEnv->env_status = ENV_READY;
		sys_sched();
	}
	sem.semdata->lock = 0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
