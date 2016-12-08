#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>//pipe
#include<signal.h>//exit
#include<sys/wait.h>
#define MAXVALUE 100

void output(int num,char cache[])
{
	int i;
	printf("%d types ",num);
	if(num!=0){
		for(i=0;i<num;i++)
		{
			printf("%c",cache[i]);
		}
	}
	printf("\n");
}
 
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
		printf("In child1 process...Write 'Child1'\n");
		close(fd[0]);
		write(fd[1],"Child1 ",7);
		sleep(3);//3 s
		exit(0);
	}
	else if(pid1>0){
		while((pid2=fork())==-1);
		if(pid2==0){
			printf("In child2 process...Write 'Child2'\n");
			close(fd[0]);
			write(fd[1],"Child2 ",7);
			sleep(3);
			exit(0);
		}else if(pid2>0){
			while((pid3=fork())==-1);
			if(pid3==0){
				printf("In child3 process...Write 'Child3'\n");
				close(fd[0]);
				write(fd[1],"Child3 ",7);
				sleep(3);
				exit(0);
			}
			else{
				printf("In parent process\n");
				close(fd[1]);
				wait(0);//wait for child end
				printf("First read 14 type\n");
				num =read(fd[0],cache,14);
				output(num,cache);
				
				printf("Second read 7 type\n");
				num =read(fd[0],cache,7);
				output(num,cache);
				
				printf("Third read 7 type\n");
				num =read(fd[0],cache,7);
				output(num,cache);
				exit(0);
			}
		}
	}
	return 0;
}
