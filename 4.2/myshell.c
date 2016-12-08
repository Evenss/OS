#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int getNumber(char cmdStr[])
{
	int num=-1;//error
	if(cmdStr[0]=='c'&&cmdStr[1]=='m'&&cmdStr[2]=='d'){
		if(cmdStr[3]=='1')
			num=1;
		else if(cmdStr[3]=='2')
			num=2;
		else
			num=3;
	}
	else if(cmdStr[0]=='e'&&cmdStr[1]=='x'&&cmdStr[2]=='i'&&cmdStr[3]=='t')
		num=0;//exit
	return num;
}

void runCmd(int num)
{
	pid_t pid;
	int execl_status=-1;
	if((pid=fork())<0)//必须在这里fork
		printf("fork error\n");
	else if(pid==0){
		switch(num){
			case 1:
				execl("./cmd1","cmd1",NULL);
				break;
			case 2:
				execl("./cmd2","cmd1",NULL);
				break;
			case 3:
				execl("./cmd3","cmd1",NULL);
				break;
			case -1:
				printf("Command not found\n");
				break;
				
		}
	}
	
}

int main()
{	
	char cmdStr[20];
	int num,x;
	pid_t pid;
	memset(cmdStr,0,sizeof(cmdStr));
	while(1){
		scanf("%s",cmdStr);
		num=getNumber(cmdStr);
		if(num!=0)
			runCmd(num);
		else
		{
			exit(0);
		}
	}
}
