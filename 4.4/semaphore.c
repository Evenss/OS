#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>//pthread_create and so on
#include<errno.h>//记录错误代码 errno
#include<sys/msg.h>//msgsnd msgrcv and so on
#include<semaphore.h>

struct msg_st {
	long int msg_type;
	char text[BUFSIZ];
};
struct msg_st data;
sem_t sem1, sem2;//sem_t是信号量的数据类型
void clean() {//清空消息队列
	int msgid  = msgid = msgget((key_t)1235, 0666 | IPC_CREAT);//
	msgctl(msgid, IPC_RMID, 0);//删除消息队列
}
void receive() {
	int running = 1;
	int msgid = -1;//表示失败
	msgid = msgget((key_t)1235, 0666 | IPC_CREAT);//创建队列
	long int msgtype = 0;
	if(msgid == -1) {
		fprintf(stderr, "msgget failed with error: %d\n",errno);//创建队列失败
		exit(EXIT_FAILURE);
	}
	while(running) {
		sem_wait(&sem2);//0
		if(msgrcv(msgid, (void*)&data, BUFSIZ, msgtype, 0) == -1) {//从队列中获取消息
			fprintf(stderr, "msgrcv failed with errno %d\n",errno);
			exit(EXIT_FAILURE);
		}
		if(strncmp(data.text, "end", 3) == 0) {
			strcpy(data.text, "over");//结尾添加over
			data.msg_type = 1;//?
			msgsnd(msgid, (void*)&data, BUFSIZ, 0);//向队列发送数据
			running = 0;
		}
		if(running)
			printf("you text : %s\n", data.text);
		sem_post(&sem1);//1
	}
}
void send() {
	int running = 1;
	int msgid = -1;//msgid是由msgget函数返回的消息队列标识符。
	msgid = msgget((key_t)1235, 0666 | IPC_CREAT);
	char buffer[BUFSIZ];
	if(msgid == -1) {
		fprintf(stderr, "msgget failed with errno: %d\n",errno);
		exit(EXIT_FAILURE);
	}
	while(running) {
		sem_wait(&sem1);//0
		printf("Enter some text: \n");
		fgets(buffer, BUFSIZ, stdin);
		data.msg_type = 1;
		strcpy(data.text, buffer);
		if(strncmp(buffer, "exit", 4) == 0) {
			strcpy(data.text, "end");
			running = 0;
		}
		if(msgsnd(msgid, (void*)&data, BUFSIZ, 0) == -1) {//向队列发送数据
			printf("msgsnd failed.\n");
			exit(EXIT_FAILURE);
		}
		sem_post(&sem2);//1
	}
	msgrcv(msgid, (void*)&data, BUFSIZ, 0, 0); //从队列中获取消息
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
	pthread_t id1, id2;//申明线程id，即线程标识符的指针
	pthread_create(&id1,NULL,&sender,NULL);
	pthread_create(&id2,NULL,&receiver,NULL);
	pthread_join(id1,NULL);//等待一个线程的结束,线程间的同步
	pthread_join(id2,NULL);
	pthread_exit(&id1);
	pthread_exit(&id2);
	return 0;
}
