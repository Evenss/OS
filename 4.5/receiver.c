#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#define TEXT_SZ 2048

char SEM1[] = "vik1";
char SEM2[] = "vik2";
struct shared_use_st {
	int written;//0-w
	char text[TEXT_SZ];
};

int main(){
	sem_t *sem1;
	sem1 = sem_open(SEM1, O_CREAT, 0644, 1);
	sem_t *sem2;
	sem2 = sem_open(SEM2, O_CREAT, 0644, 0);


	int running = 1;
	void *shm = NULL;
	struct shared_use_st *shared = NULL;  
	int shmid;
	shmid = shmget((key_t)1235, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	
	if(shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shm = shmat(shmid, 0, 0);

	if(shm == (void*)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	shared = (struct shared_use_st*)shm;
	shared->written = 0;
	
	while(running) {
		sem_wait(sem2);
		printf("You wrote: %s\n", shared->text);
		shared->written = 0;
		if(strncmp(shared->text, "end", 3) == 0) {
			strncpy(shared->text,"over",TEXT_SZ);			
			printf("Now is %s\n",shared->text);			
			running = 0;
		}		
		sem_post(sem1);
	}
}
