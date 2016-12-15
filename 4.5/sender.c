#include<stdlib.h>
#include<stdio.h>
#include<sys/shm.h>
#include<unistd.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<sys/stat.h>
#define TEXT_SZ 2048

struct shared_use_st {
	int written;//0-able w
	char text[TEXT_SZ];
};
char SEM1[] = "vik1";
char SEM2[] = "vik2";
int main() {
	sem_t *sem1;
	sem1 = sem_open(SEM1, O_CREAT, 0644, 1);
	sem_t *sem2;
	sem2 = sem_open(SEM2, O_CREAT, 0644, 0);

	int running = 1;
	void *shm = NULL;
	struct shared_use_st *shared = NULL;//shm->shared
	char buffer[BUFSIZ+1];
	
	int shmid;
	shmid = shmget((key_t)1235, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	if(shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shm = shmat(shmid, (void*)0, 0);//start shm
	if(shm == (void*)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	shared = (struct shared_use_st*)shm;
	while(running) {		
		sem_wait(sem1);
		if(strncmp(shared->text, "over", 4) == 0) break;
		printf("Enter some text: \n");
		fgets(buffer, BUFSIZ, stdin);
		strncpy(shared->text, buffer, TEXT_SZ);
		sem_post(sem2);
	}
	if(shmdt(shm) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	if(shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
} 
