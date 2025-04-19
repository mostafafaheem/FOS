// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("dakhal create_semaphore\n");
	//struct semaphore* temp_sem=(struct semaphore*)malloc(sizeof(struct semaphore));
	struct semaphore temp_sem;
	temp_sem.semdata=(struct __semdata*)smalloc(semaphoreName,sizeof(struct __semdata),1);


	if(temp_sem.semdata==NULL)
	{
		panic("can not malloc\n");
	}

//	if(temp_sem==NULL)
//	{
//		panic("can not allocate semaphore\n");
//	}
	//cprintf("create_semaphore baad if\n");

	//struct semaphore created_sem=temp_sem;
	temp_sem.semdata->count=value;
	temp_sem.semdata->lock=0;
	//strncpy(created_sem.semdata->name,semaphoreName, strlen(semaphoreName));
	strcpy(temp_sem.semdata->name,semaphoreName);
	sys_initQueue(&(temp_sem.semdata->queue));
	return temp_sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...

	//struct semaphore *temp_sem=(struct semaphore *)malloc(sizeof(struct semaphore));
	struct semaphore temp_sem;
	temp_sem.semdata=(struct __semdata *)sget(ownerEnvID,semaphoreName);
	//struct semaphore get_sem=temp_sem;
	return temp_sem;
}

void wait_semaphore(struct semaphore sem)
{
//	cprintf("free frame list %d\n",sys_calculate_free_frames());
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	int key=1;
	struct Env envv;

	while(xchg(&(sem.semdata->lock),1) != 0);
	sem.semdata->count--;
	if(sem.semdata->count<0)
	{
		//get_cpu_proc()->env_status =ENV_BLOCKED;
		sys_enqueue(&(sem.semdata->queue),&sem,&envv);

	}
	sem.semdata->lock=0;
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...

	int key=1;
	struct Env* env;
		//sys_pushcli();
//		do
//		{
//			xchg(key,sem.semdata->lock);
//		}while(key != 0);
	while(xchg(&(sem.semdata->lock),1) != 0);

		sem.semdata->count++;
		if(sem.semdata->count<=0)
		{
			sys_dequeue(&(sem.semdata->queue),&sem);
			//sys_sched_insert_ready0(env,&sem);
		}

			sem.semdata->lock=0;


}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
