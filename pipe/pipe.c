#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>//pipe
#include<signal.h>//exit
#include<sys/wait.h>
#define MAXVALUE 100

int main()
{
	int fd[2];//0--read 1--write
	int pid1,pid2,pid3,num;
	char cache[MAXVALUE];//array store info
	if(pipe(fd)<0){
		printf("Create pipe failed\n");
	}
	while((pid1=fork())==-1);//fork success
	if(pid1==0){//child process
		printf("In child1 process\n");
		close(fd[0]);
		lockf(fd[1],F_LOCK,0);//lock write
		write(fd[1],"Child1 ",7);
		sleep(3);//5 s
		lockf(fd[1],F_ULOCK,0);//unlock
		exit(0);
	}
	else if(pid1>0){
		while((pid2=fork())==-1);
		if(pid2==0){
			printf("In child2 process\n");
			close(fd[0]);
			//lockf(fd[1],1,0);
			write(fd[1],"Child2 ",7);
			sleep(3);
			//lockf(fd[1],0,0);
			exit(0);
		}else if(pid2>0){
			while((pid3=fork())==-1);
			if(pid3==0){
				printf("In child3 process\n");
				close(fd[0]);
				write(fd[1],"Child3 ",7);
				sleep(3);
				exit(0);
			}
			else{
				printf("In parent process\n");
				close(fd[1]);
				wait(0);//wait for child end
				num =read(fd[0],cache,7);
				printf("%d type %s\n",num,cache);
				//wait(0);
				num =read(fd[0],cache,7);
				printf("%d type %s\n",num,cache);
				//wait(0);
				num =read(fd[0],cache,7);
				printf("%d type %s\n",num,cache);
				exit(0);
			}
		}
	}
	return 0;
}
