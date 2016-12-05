#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>//?
#include<signal.h>//exit
#include<sys/wait.h>
#define MAXVALUE 100

int main()
{
	int fd[2];//0-read 1-write
	int pid1,pid2,pid3;
	char cache[MAXVALUE];
	if(pipe(fd)<0){
		printf("Create pipe failed\n");
	}

	pid1=fork();
	pid2=fork();
	pid3=fork();
	if(pid1==0)
	{
		printf("In child1 process\n");
		close(fd[0]);//close read
		write(fd[1],"Child1 ",7);
		close(fd[1]);//close write
		exit(0);
		//wait(NULL);//
	}
	else if(pid2==0)
	{
		printf("In child2 process\n");
		close(fd[0]);//close read
		write(fd[1],"Child2 ",7);
		close(fd[1]);//close write
		exit(0);
	}
	else if(pid3==0)
	{
		printf("In child3 process\n");
		close(fd[0]);//close read
		write(fd[1],"Child3 ",7);
		close(fd[1]);//close write
		exit(0);
	}
	else if(pid1>0 && pid2>0 && pid3>0)
	{
		printf("In parent process\n");
		close(fd[1]);//close write
		sleep(1);//wait for writing content
		int num=read(fd[0],cache,100);
		printf("%d types %s\n",num,cache);
		close(fd[0]);//close read
		wait(NULL);
	}
	else
	{
		printf("Create chile process failed\n");
	}
	return 0;
}
