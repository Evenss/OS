#include"Functions.h"

const int cmdListNum = 12;
char *cmdList[cmdListNum] = { "mkdir", "rmdir", "ls", "cd", "create", 
					"rm", "open", "close", "write", "read", "exit", "help" };
int main()
{
	char input[30],*command;
	char *content;
	cout << "-------------------File System---------------------"<<endl;
	cout << "Please input 'help' command for getting information" << endl;
	startSys();
	while (true)
	{
		int cmdNum = -1;
		cout <<openfilelist[currfd].dir<< ">" ;
		gets_s(input);
		if (strcmp(input,"")) {//check 
			command = strtok(input, " ");//get command
			content = strtok(NULL, " ");//get content
			for (int i = 0;i < cmdListNum;i++) {
				if (strcmp(command, cmdList[i]) == 0) {
					cmdNum = i;
					break;
				}
			}
		}

		switch (cmdNum)
		{
			case 0:
				if (content != NULL) {
					my_mkdir(content);
				}
				break;
			case 1:
				if (content != NULL) {
					my_rmdir(content);
				}
				break;
			case 2:
				my_ls();
				break;
			case 3:
				if (content != NULL) {
					my_cd(content);
				}
				break;
			case 4:
				if (content != NULL) {
					my_create(content);
				}
				break;
			case 5:
				if (content != NULL) {
					my_rm(content);
				}
				break;
			case 6:
				if (content != NULL) {
					my_open(content);
				}
				break;
			case 7:
				if (openfilelist[currfd].attribute == 1)
					my_close(currfd);
				else
					cout << "No file open" << endl;
				break;
			case 8:
				if (openfilelist[currfd].attribute == 1)
					my_write(currfd);
				else
					cout << "Please open file then write" << endl;
				break;
			case 9:
				if (openfilelist[currfd].attribute == 1)
					my_read_all(currfd);
				else
					cout << "Please open file then read" << endl;
				break;
			case 10:
				exitsys();
				cout << "Exit My File System..." << endl;
				cout << "... see you again" << endl;
				return 0;
				break;
			case 11:
				show_help();
				break;
			default:
				cout << "Command Not Find!Please input 'help' for help.";
				break;
		}
		cout << endl;
	}
	return 0;
}