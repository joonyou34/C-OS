// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	// TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("create_semaphore is not implemented yet");
	cprintf("MEOWWWWWW IN CREATE :D\n");
	struct __semdata semdat;
	semdat.count = (int)value;
	semdat.lock = (uint32)0;
	sys_init_queue(&semdat.queue);
	strcpy(semdat.name, semaphoreName);
	struct semaphore *newSem;
	newSem = (struct semaphore *)smalloc(semaphoreName, sizeof(struct semaphore), 1);
	newSem->semdata = &semdat;
	return *newSem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char *semaphoreName)
{
	cprintf("MEOWWWWWW IN GET :D\n");
	// TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("get_semaphore is not implemented yet");
	// Your Code is Here...
	struct semaphore *sanfor = (struct semaphore *)sget(ownerEnvID, semaphoreName);
	return *sanfor;
}

void wait_semaphore(struct semaphore sem)
{
	cprintf("MEOWWWWWW IN WAIT :D\n");
	// TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	//  panic("wait_semaphore is not implemented yet");
	// sys_pushcli();
	while (xchg(&sem.semdata->lock, 1) != 0)
		;
	int32 procId = sys_getenvindex();
	if (--sem.semdata->count < 0)
	{
		volatile struct Env *volatileCurEnv = &envs[procId]; // Fetch the volatile struct pointer
		struct Env *curEnv = (struct Env *)volatileCurEnv;	 // Cast it to a normal struct pointer
		sys_enqueue(&sem.semdata->queue, curEnv);
		//sem.semdata->lock = 0;
		curEnv->env_status = ENV_BLOCKED;
	}
	sem.semdata->lock = 0;
	// sys_popcli();
}

void signal_semaphore(struct semaphore sem)
{
	cprintf("MEOWWWWWW IN SIGNAL :D\n");
	// TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	// COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("signal_semaphore is not implemented yet");
	// sys_pushcli();
	while (xchg(&sem.semdata->lock, 1) != 0)
		;

	int32 procId = sys_getenvindex();
	if (++sem.semdata->count <= 0)
	{
		volatile struct Env *volatileCurEnv = &envs[procId]; // Fetch the volatile struct pointer
		struct Env *curEnv = (struct Env *)volatileCurEnv;	 // Cast it to a normal struct pointer
		sys_remove_from_queue(&sem.semdata->queue, curEnv);
		curEnv->env_status = ENV_READY;
	}
	sem.semdata->lock = 0;
	// sys_popcli();
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
