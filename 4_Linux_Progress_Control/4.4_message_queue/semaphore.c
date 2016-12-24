#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>//pthread_create and so on
#include<errno.h>
#include<sys/msg.h>//msgsnd msgrcv and so on
#include<semaphore.h>

struct msg_st {
	long int msg_type;
	char text[BUFSIZ];
};
struct msg_st data;
sem_t sem1, sem2;
void clean() {
	int msgid = msgget((key_t)1235, 0666 | IPC_CREAT);
	msgctl(msgid, IPC_RMID, 0);//del
}
void receive() {
	int running = 1;
	int msgid = -1;
	msgid = msgget((key_t)1235, 0666 | IPC_CREAT);
	if(msgid == -1) {
		fprintf(stderr, "msgget failed with error: %d\n",errno);
		exit(EXIT_FAILURE);
	}
	while(running) {
		sem_wait(&sem2);//0p
		if(msgrcv(msgid, (void*)&data, BUFSIZ, 0, 0) == -1) {//0-early in line
			fprintf(stderr, "msgrcv failed with errno %d\n",errno);
			exit(EXIT_FAILURE);
		}
		if(strncmp(data.text, "end", 3) == 0) {
			strcpy(data.text, "over");
			data.msg_type = 2;
			msgsnd(msgid, (void*)&data, BUFSIZ, 0);
			running = 0;
		}
		if(running)
			printf("you text : %s\n", data.text);
		sem_post(&sem1);//1v
	}
}
void send() {
	int running = 1;
	int msgid = -1;
	msgid = msgget((key_t)1235, 0666 | IPC_CREAT);
	char buffer[BUFSIZ];
	if(msgid == -1) {
		fprintf(stderr, "msgget failed with errno: %d\n",errno);
		exit(EXIT_FAILURE);
	}
	while(running) {
		sem_wait(&sem1);//0p
		printf("Enter some text: \n");
		fgets(buffer, BUFSIZ, stdin);
		data.msg_type = 1;
		strcpy(data.text, buffer);
		if(strncmp(buffer, "exit", 4) == 0) {
			strcpy(data.text, "end");
			running = 0;
		}
		if(msgsnd(msgid, (void*)&data, BUFSIZ, 0) == -1) {
			printf("msgsnd failed.\n");
			exit(EXIT_FAILURE);
		}
		sem_post(&sem2);//1v
	}
	msgrcv(msgid, (void*)&data, BUFSIZ, 0, 0); //get msg
	printf("back massage: %s\n",data.text);
}

void* sender() {
	send();
	exit(EXIT_SUCCESS);
}
void* receiver() {
	receive();
	exit(EXIT_SUCCESS);
}
int main(){
	clean();
	sem_init(&sem1,0,1);
	sem_init(&sem2,0,0);
	pthread_t id1, id2;//pointer
	pthread_create(&id1,NULL,&sender,NULL);
	pthread_create(&id2,NULL,&receiver,NULL);
	pthread_join(id1,NULL);//wait process end syn
	pthread_join(id2,NULL);
	pthread_exit(&id1);
	pthread_exit(&id2);
	return 0;
}
