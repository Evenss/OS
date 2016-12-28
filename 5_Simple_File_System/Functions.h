#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#include"SysStruct.h"

int getFreeBLOCK()
{
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	for (int i = 0;i < (int)(SIZE / BLOCKSIZE);i++) {
		if (fat1[i].id == FREE) {
			return i;
		}
	}
	return END;
};

int getFreeOpenfilelist()
{
	for (int i = 0;i < MAXOPENFILE;i++) {
		if (openfilelist[i].topenfile == 0) {
			openfilelist[i].topenfile = 1;
			return i;
		}
	}
	return -1;
};

int find_father_dir(int fd)
{
	for (int i = 0;i < MAXOPENFILE;i++) {
		if (openfilelist[i].first == openfilelist[fd].dirno) {
			return i;
		}
	}
	return -1;
};


void createSys() {
	cout << "There is no myfsys.Now create File System..." << endl;
	my_format();
	memcpy(buffer, myvhard, SIZE);
	cout << "File System has created!" << endl;
	return;
}

void startSys()
{
	FILE *file;
	myvhard = (unsigned char*)malloc(SIZE);
	if ((file = fopen(FileName, "r"))!=NULL) {
		fread(buffer,SIZE,1,file);
		fclose(file);
		if (memcmp(buffer, "10101010", 8) == 0) {//compare
			memcpy(myvhard, buffer, SIZE);//copy
			cout << "Read Success!" << endl;
		}
		else {
			createSys();
		}
	}
	else {
		createSys();
	}

	fcb *root;
	root = (fcb *)(myvhard + 5 * BLOCKSIZE);

	strcpy(openfilelist[0].filename, root->filename);
	strcpy(openfilelist[0].exname, root->exname);
	openfilelist[0].attribute = root->attribute;
	openfilelist[0].time = root->time;
	openfilelist[0].date = root->date;
	openfilelist[0].first = root->first;
	openfilelist[0].length = root->length;
	openfilelist[0].free = root->free;

	strcpy(openfilelist[0].dir, "root\\");
	openfilelist[0].dirno = 5;
	openfilelist[0].diroff = 0;
	openfilelist[0].count = 0;
	openfilelist[0].fcbstate = 0;
	openfilelist[0].topenfile = 1;

	currfd = 0;
	block0 *startBlock;
	startBlock = (block0*)myvhard;
	startp = startBlock->startblock;
	
	return;
};

void my_format()
{
	//startBlock setting
	block0 *startBlock;
	startBlock = (block0*)myvhard;
	strcpy(startBlock->magic_number, "10101010");
	strcpy(startBlock->information,"简单文件系统\n分配方式：FAT\n 目录结构：单用户多级目录\n");
	startBlock->root = 5;
	startBlock->startblock = myvhard + 5 * BLOCKSIZE;
	
	//fat1 fat2 setting
	fat *fat1 = (fat*)(myvhard + BLOCKSIZE);
	fat *fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	for (int i = 0;i < 6;i++) {
		fat1[i].id = END;//No.5 block is used by root
	}
	for (int i = 6;i < (SIZE / BLOCKSIZE);i++) {
		fat1[i].id = FREE;
	}
	memcpy(fat2, fat1, BLOCKSIZE);

	//root setting
	fcb *root1 = (fcb*)(myvhard + 5 * BLOCKSIZE);
	strcpy(root1->filename,".");
	strcpy(root1->exname, "d");
	root1->attribute = 0;
	root1->first = 5;
	root1->length = 2 * sizeof(fcb);
	root1->free = 1;
	time_t rawTime = time(NULL);//get second
	struct tm *time = localtime(&rawTime);//change to data
	root1->time = time->tm_hour * 2048 + time->tm_min * 32 + time->tm_sec / 2;
	root1->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
	cout << time->tm_year << endl;
	fcb *root2 = root1 + 1;
	memcpy(root2, root1, sizeof(fcb));
	strcpy(root2->filename, "..");

	fcb *restFCB = root2;
	for (int i = 2;i < (BLOCKSIZE / sizeof(fcb));i++) {
		restFCB++;
		strcpy(restFCB->filename, "");
		restFCB->free = 0;
	}

	FILE *file = fopen(FileName, "w");
	fwrite(myvhard, SIZE, 1, file);
	fclose(file);
};

void exitsys()
{
	while (currfd) {
		my_close(currfd);
	}
	FILE *file = fopen(FileName, "w");
	fwrite(myvhard, SIZE, 1, file);
	fclose(file);
};

void my_mkdir(char *dirname)
{
	char* fname = strtok(dirname, ".");
	char* exname = strtok(NULL, ".");

	//input check
	if (exname) {
		cout << "Input is not valid!" << endl;
		return;
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	int readLen = do_read(currfd, openfilelist[currfd].length, buf);

	//check same name
	fcb* fcbPtr = (fcb*)buf;
	for (int i = 0;i < (int)(readLen / sizeof(fcb));i++) {
		if (strcmp(dirname, fcbPtr->filename) == 0 && fcbPtr->attribute == 0) {
			cout << "There is The Same File!" << endl;
			return;
		}
	}

	//apply openfilelist
	int fd = getFreeOpenfilelist();
	if (fd == -1) {
		cout << "File Room not enough!" << endl;
		return;
	}

	//apply block
	int blockNum = getFreeBLOCK();
	if (blockNum == END) {
		cout << "Block not enough!" << endl;
		openfilelist[fd].topenfile = 0;
		return;
	}

	//update fat
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	fat* fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	fat1[blockNum].id = END;
	fat2[blockNum].id = END;

	//find free block
	int i = 0;
	for (;i < (int)(readLen / sizeof(fcb));i++) {
		if (fcbPtr[i].free == 0) {
			break;
		}
	}
	openfilelist[currfd].count = i*sizeof(fcb);
	openfilelist[currfd].fcbstate = 1;
	
	//write to fcbTmp then to file
	fcb* fcbTmp = (fcb*)malloc(sizeof(fcb));
	strcpy(fcbTmp->filename, dirname);
	strcpy(fcbTmp->exname, "d");
	fcbTmp->attribute = 0;
	fcbTmp->first = blockNum;
	fcbTmp->length = 2*sizeof(fcb);
	fcbTmp->free = 1;
	time_t rawtime = time(NULL);
	struct tm* time = localtime(&rawtime);
	fcbTmp->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
	fcbTmp->time = (time->tm_hour) * 2048 + (time->tm_min) * 32 + (time->tm_sec) / 2;
	do_write(currfd, (char*)fcbTmp, sizeof(fcb), 1);

	//set openfilelist
	strcpy(openfilelist[fd].filename, dirname);
	strcpy(openfilelist[fd].exname, "d");
	openfilelist[fd].attribute = 0;
	openfilelist[fd].time = fcbTmp->time;
	openfilelist[fd].date = fcbTmp->date;
	openfilelist[fd].first = fcbTmp->first;
	openfilelist[fd].length = fcbTmp->length;
	openfilelist[fd].free = fcbTmp->free;
	openfilelist[fd].dirno = openfilelist[currfd].first;
	openfilelist[fd].diroff = 0;
	strcpy(openfilelist[fd].dir, (char*)(string(openfilelist[currfd].dir) + string(dirname) + string("\\")).c_str());
	openfilelist[fd].count = 0;
	openfilelist[fd].fcbstate = 0;
	openfilelist[fd].topenfile = 1;

	//add . & ..
	strcpy(fcbTmp->filename, ".");//noly filename change
	do_write(fd, (char*)fcbTmp, sizeof(fcb), 1);

	fcb* fcbTmp2 = (fcb*)malloc(sizeof(fcb));
	memcpy(fcbTmp2, fcbTmp, sizeof(fcb));
	strcpy(fcbTmp2->filename, "..");
	strcpy(fcbTmp2->exname, "d");
	fcbTmp2->first = openfilelist[currfd].first;
	fcbTmp2->length = openfilelist[currfd].length;
	fcbTmp2->date = openfilelist[currfd].date;
	fcbTmp2->time = openfilelist[currfd].time;
	do_write(fd, (char*)fcbTmp2, sizeof(fcb), 1);
	
	//back father directory
	my_close(fd);
	
	//update currfd's fcb  
	fcbPtr = (fcb*)buf;
	fcbPtr->length = openfilelist[currfd].length;
	openfilelist[currfd].count = 0;
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].fcbstate = 1;

	free(fcbTmp);
	free(fcbTmp2);
};

void my_rmdir(char *dirname)
{
	char* fname = strtok(dirname, ".");
	char* exname = strtok(NULL, ".");

	//input check
	if (strcmp(dirname, ".") == 0 && strcmp(dirname, "..")==0) {
		cout << "Can't remove system file!" << endl;
		return;
	}
	if (exname) {
		cout << "Input is not valid!" << endl;
		return;
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	int readLen = do_read(currfd, openfilelist[currfd].length, buf);
	
	//find this d_file
	int i;
	int length = (int)(openfilelist[currfd].length / sizeof(fcb));
	fcb* fcbPtr = (fcb*)buf;
	for (i = 0;i < length;i++, fcbPtr++) {
		if (strcmp(fcbPtr->filename, fname) == 0 && fcbPtr->attribute == 0) {
			break;
		}
	}
	if (i == length) {
		cout << "No '" << dirname << "' File!" << endl;
		return;
	}

	//check other files in this file
	if (fcbPtr->length > 2 * sizeof(fcb)) {
		cout << "Please remove all other files in this file!" << endl;
		return;
	}

	//clear using fcb
	int blockNum = fcbPtr->first;
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	int next = 0;
	while (true) {
		next = fat1[blockNum].id;
		fat1[blockNum].id = FREE;
		if (next != END) {
			blockNum = next;
		}
		else {
			break;
		}
	}
	fat* fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	memcpy(fat2, fat1, BLOCKSIZE);
	
	//clear fcb in father
	fcbPtr->date = 0;
	fcbPtr->time = 0;
	fcbPtr->exname[0] = '\0';
	fcbPtr->filename[0] = '\0';
	fcbPtr->first = 0;
	fcbPtr->free = 0;
	fcbPtr->length = 0;
	openfilelist[currfd].count = i*sizeof(fcb);
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].length -= sizeof(fcb);

	//update currfd's fcb  
	fcbPtr = (fcb*)buf;
	fcbPtr->length = openfilelist[currfd].length;
	openfilelist[currfd].count = 0;
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].fcbstate = 1;
	
};

void my_ls()
{
	if (openfilelist[currfd].attribute == 1) {
		cout << "Data File Can't use 'my_ls'" << endl;
		return;
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	do_read(currfd, openfilelist[currfd].length, buf);

	//output info
	fcb* fcbPtr = (fcb*)buf;
	for (int i = 0;i < (int)(openfilelist[currfd].length / sizeof(fcb));i++) {
		if (fcbPtr->attribute == 0) {//d_file info
			printf("%s\\\t<DIR>\t%d/%d/%d\t%d:%d:%d\n",
				fcbPtr->filename,
				(fcbPtr->date >> 9) + 2000,
				(fcbPtr->date >> 5) & 0x000f,
				(fcbPtr->date) & 0x001f,
				(fcbPtr->time >> 11),
				(fcbPtr->time >> 5) & 0x003f,
				(fcbPtr->time) & 0x001f * 2);
		}
		else {
			unsigned int length = fcbPtr->length;
			if (length != 0) {
				length -= 2;//delete \0 \n
			}
			printf("%s.%s\t%dB\t%d/%d/%d\t%02d:%02d:%02d\n",
				fcbPtr->filename,
				fcbPtr->exname,
				length,
				(fcbPtr->date >> 9) + 2000,
				(fcbPtr->date >> 5) & 0x000f,
				(fcbPtr->date) & 0x001f,
				(fcbPtr->time >> 11),
				(fcbPtr->time >> 5) & 0x003f,
				(fcbPtr->time) & 0x001f * 2);
		}
		fcbPtr++;
	}
};

void my_cd(char *dirname)
{
	if (openfilelist[currfd].attribute == 1) {
		cout << "Data File Can't Use 'my_cd'" << endl;
		return ;
	}
	else {
		//read to buf
		char *buf = (char *)malloc(MAX_TEXT_SIZE);
		openfilelist[currfd].count = 0;
		do_read(currfd, openfilelist[currfd].length, buf);

		char tmp[20];
		int tot = 0;
		bool flag = false;
		for (int i = 0;dirname[i];i++) {
			if (flag) {
				tmp[tot++] = dirname[i];
			}
			else if (tmp[i] == '\\') {
				flag = true;
				tmp[i] = 0;
			}
		}
		tmp[tot] = 0;

		//find d_file by name
		fcb* fcbPtr = (fcb*)buf;
		int i = 0;
		for (;i<int(openfilelist[currfd].length / sizeof(fcb));i++, fcbPtr++) {
			if (strcmp(fcbPtr->filename, dirname) == 0 && fcbPtr->attribute == 0) {
				break;
			}
		}

		//cd file

		if (strcmp(fcbPtr->exname, "d") != 0) {
			cout << "Only class 'd' file  can use 'my_cd'" << endl;
		}
		else{
			if (strcmp(fcbPtr->filename, ".") == 0) {
				return;
			}
			else if (strcmp(fcbPtr->filename, "..") == 0) {
				if (currfd == 0) {
					return;
				}
				else {
					currfd = my_close(currfd);//////////////////////////这里文档要求my_close不返回值
					return;
				}
			}
			else {
				int fd = getFreeOpenfilelist();
				if (fd == -1) {
					cout << "File Room not enough!Can't use 'my_cd'" << endl;
					return;
				}
				else {
					openfilelist[fd].attribute = fcbPtr->attribute;
					openfilelist[fd].count = 0;
					openfilelist[fd].date = fcbPtr->date;
					openfilelist[fd].time = fcbPtr->time;
					strcpy(openfilelist[fd].filename, fcbPtr->filename);
					strcpy(openfilelist[fd].exname, fcbPtr->exname);
					openfilelist[fd].first = fcbPtr->first;
					openfilelist[fd].free = fcbPtr->free;

					openfilelist[fd].fcbstate = 0;
					openfilelist[fd].length = fcbPtr->length;
					strcpy(openfilelist[fd].dir, (char*)(string(openfilelist[currfd].dir) + string(dirname) + string("\\")).c_str());
					openfilelist[fd].topenfile = 1;
					openfilelist[fd].dirno = openfilelist[currfd].first;
					openfilelist[fd].diroff = i;
					
					currfd = fd;
				}
			}
		}
	}
};

int  my_create(char *filename)
{
	char* fname = strtok(filename, ".");
	char* exname = strtok(NULL, ".");

	//check input 
	if (strcmp(fname, "") == 0 || !(exname)) {
		cout << "Input is not valid!" << endl;
		return -1;
	}
	if (openfilelist[currfd].attribute == 1) {
		cout << "Data File can't use 'my_create'!" << endl;
		return -1;
	}

	//apply openfilelist
	int fd = getFreeOpenfilelist();
	if (fd == -1) {
		cout << "File Room not enough!" << endl;
		return -1;
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	do_read(currfd, openfilelist[currfd].length, buf);

	//check same name
	int i;
	fcb* fcbPtr = (fcb*)buf;
	for (i = 0;i < (int)(openfilelist[currfd].length / sizeof(fcb));i++,fcbPtr++) {/////////////////////
		if (strcmp(fcbPtr->filename, filename) == 0 && strcmp(fcbPtr->exname,exname)==0) {
			cout << "There is The Same File!" << endl;
			openfilelist[fd].topenfile = 0;
			return -1;
		}
	}

	//apply block
	int blockNum = getFreeBLOCK();
	if (blockNum == END) {
		cout << "Block not enough!" << endl;
		openfilelist[fd].topenfile = 0;
		return -1;
	}

	//update fat
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	fat* fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	fat1[blockNum].id = END;
	fat2[blockNum].id = END;

	//find free block
	fcbPtr = (fcb*)buf;
	for (i = 0;i < (int)(openfilelist[currfd].length / sizeof(fcb));i++, fcbPtr++) {
		if (fcbPtr->free == 0) {
			break;
		}
	}
	openfilelist[currfd].count = i*sizeof(fcb);
	
	//write to fcbPtr then to file
	strcpy(fcbPtr->filename, filename);
	strcpy(fcbPtr->exname, exname);
	fcbPtr->attribute = 1;
	fcbPtr->first = blockNum;
	fcbPtr->length = 0;
	fcbPtr->free = 1;
	time_t rawtime = time(NULL);
	struct tm* time = localtime(&rawtime);
	fcbPtr->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
	fcbPtr->time = (time->tm_hour) * 2048 + (time->tm_min) * 32 + (time->tm_sec) / 2;
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);

	//update currfd's fcb  
	fcbPtr = (fcb*)buf;
	fcbPtr->length = openfilelist[currfd].length;
	openfilelist[currfd].count = 0;
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].fcbstate = 1;

	return fd;
};

void my_rm(char *filename)
{
	char* fname = strtok(filename, ".");
	char* exname = strtok(NULL, ".");

	//check input
	if (!exname) {
		cout << "Please input exname!" << endl;
		return ;
	}
	if (strcmp(exname, "d") == 0) {
		cout << "Can't remove directory file!" << endl;
		return;
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	int readLen = do_read(currfd, openfilelist[currfd].length, buf);

	//find this d_file
	int i;
	int length = (int)(openfilelist[currfd].length / sizeof(fcb));
	fcb* fcbPtr = (fcb*)buf;
	for (i = 0;i < length;i++, fcbPtr++) {
		if (strcmp(fcbPtr->filename, fname) == 0 && strcmp(fcbPtr->exname, exname) == 0) {
			break;
		}
	}
	if (i == length) {
		cout << "No '" << filename << "' File!" << endl;
		return;
	}

	//clear using fcb
	int blockNum = fcbPtr->first;
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	int next = 0;
	while (true) {
		next = fat1[blockNum].id;
		fat1[blockNum].id = FREE;
		if (next != END) {
			blockNum = next;
		}
		else {
			break;
		}
	}
	fat* fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	memcpy(fat2, fat1, BLOCKSIZE);

	//clear fcb in father
	fcbPtr->date = 0;
	fcbPtr->time = 0;
	fcbPtr->exname[0] = '\0';
	fcbPtr->filename[0] = '\0';
	fcbPtr->first = 0;
	fcbPtr->free = 0;
	fcbPtr->length = 0;
	openfilelist[currfd].count = i*sizeof(fcb);
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].length -= sizeof(fcb);

	//update currfd's fcb  
	fcbPtr = (fcb*)buf;
	fcbPtr->length = openfilelist[currfd].length;
	openfilelist[currfd].count = 0;
	do_write(currfd, (char*)fcbPtr, sizeof(fcb), 1);
	openfilelist[currfd].fcbstate = 1;
};

int  my_open(char *filename)
{
	//break filename
	char *fname = strtok(filename, ".");
	char *exname = strtok(NULL, ".");
	
	//check open
	int i;
	for (i = 0;i < MAXOPENFILE;i++) {
		if (strcmp(openfilelist[i].filename,fname)==0 && strcmp(openfilelist[i].exname,exname)==0) {
			cout << "This File has opened.Please Not Open Again!" << endl;
			return -1;
		}
	}

	//read to buf
	char buf[MAX_TEXT_SIZE];
	openfilelist[currfd].count = 0;
	do_read(currfd, openfilelist[currfd].length, buf);

	//find this file
	fcb* fcbPtr = (fcb*)buf;
	int length = (int)(openfilelist[currfd].length / sizeof(fcb));
	for (i = 0;i < length;i++,fcbPtr++) {
		if (strcmp(fcbPtr->filename, fname)==0 && strcmp(fcbPtr->exname, exname)==0 && fcbPtr->attribute == 1) {//must be data file
			break;
		}
	}
	if (i == length) {
		cout << "This file not exist" << endl;
		return -1;
	}

	//put in openfilelist
	int fd = getFreeOpenfilelist();
	if (fd == -1) {
		cout << "File Room not enough!" << endl;
		return -1;
	}
	openfilelist[fd].attribute = 1;
	openfilelist[fd].count = 0;
	openfilelist[fd].date = fcbPtr->date;
	openfilelist[fd].time = fcbPtr->time;
	strcpy(openfilelist[fd].filename, fname);
	strcpy(openfilelist[fd].exname, exname);
	openfilelist[fd].first = fcbPtr->first;
	openfilelist[fd].length = fcbPtr->length;

	strcpy(openfilelist[fd].dir, (string(openfilelist[currfd].dir) + string(filename)).c_str());
	openfilelist[fd].dirno = openfilelist[currfd].first;
	openfilelist[fd].diroff = i;
	openfilelist[fd].free = 1;
	openfilelist[fd].topenfile = 1;
	openfilelist[fd].fcbstate = 0;

	currfd = fd;
	return currfd;
};

int  my_close(int fd)
{
	if (fd > MAXOPENFILE || fd < 0) {
		cout << "File not exist" << endl;
		return -1;
	}

	int fatherFd = find_father_dir(fd);
	if (fatherFd == -1) {
		cout << "No father directory.Can't close!" << endl;
		return -1;
	}

	if (openfilelist[fd].fcbstate == 1) {
		//father d_file read to buf
		char buf[MAX_TEXT_SIZE];
		do_read(fatherFd, openfilelist[fatherFd].length, buf);

		//update fcb
		fcb* fcbPtr = (fcb*)(buf + sizeof(fcb)*openfilelist[fd].diroff);//get fcb in father's location
		strcpy(fcbPtr->filename, openfilelist[fd].filename);
		strcpy(fcbPtr->exname, openfilelist[fd].exname);
		fcbPtr->attribute = openfilelist[fd].attribute;
		fcbPtr->time = openfilelist[fd].time;
		fcbPtr->date = openfilelist[fd].date;
		fcbPtr->first = openfilelist[fd].first;
		fcbPtr->length = openfilelist[fd].length;
		fcbPtr->free = openfilelist[fd].free;

		openfilelist[fatherFd].count = openfilelist[fd].diroff*sizeof(fcb);//??????????????????
		do_write(fatherFd, (char*)fcbPtr, sizeof(fcb), 1);
	}

	//recycle openfilelist
	memset(&openfilelist[fd], 0, sizeof(USEROPEN));
	currfd = fatherFd;
	return fatherFd;
};

int  my_write(int fd)
{
	if (fd > MAXOPENFILE || fd < 0) {//cross check
		cout << "File not exist" << endl;
		return -1;
	}

	int wstyle;
	while (true) {
		cout << "Please input :0 = Cut,1 = Cover, 2 = Append" << endl;
		cin >> wstyle;
		if (wstyle < 0 || wstyle>2) {
			cout << "Input Error! Please input again..." << endl;
		}
		else{
			break;
		}
	}

	char text[MAX_TEXT_SIZE] = "\0";
	char textTmp[MAX_TEXT_SIZE] = "\0";
	cout << "Please input content ending with 'end' ：)" << endl;
	while (cin>>textTmp) {
		if (strcmp(textTmp, "end")==0) {
			break;
		}
		textTmp[strlen(textTmp)] = '\n';
		strcat(text, textTmp);//append content to text
	}

	int writeLen;
	text[strlen(text)] = '\0';
	writeLen= do_write(fd, text, strlen(text) + 1, wstyle);//strlen doesn't  include \0
	openfilelist[fd].fcbstate = 1;
	return writeLen;
};

int my_read_all(int fd)
{
	return (my_read(fd, openfilelist[fd].length));//////////////////////
};

int  my_read(int fd,int len)
{
	if (fd > MAXOPENFILE || fd < 0) {//cross check
		cout << "File not exist" << endl;
		return -1;
	}

	char text[MAX_TEXT_SIZE] = "\0";
	openfilelist[fd].count = 0;
	int readLen = do_read(fd, len, text);
	if ( readLen < 0)
	{
		cout << "Can't Read this File" << endl;
		return -1;
	}
	else {
		cout << text;
	}
	return readLen;
};

int  do_write(int fd, char *text, int len, char wstyle)
{
	//0 = Cut,1 = Cover, 2 = Append
	if (wstyle == 0) {
		openfilelist[fd].count = 0;
		openfilelist[fd].length = 0;
	}
	else if (wstyle == 1) {
		if (openfilelist[fd].attribute == 1 && openfilelist[fd].length != 0) {
			openfilelist[fd].count -= 1;//delete \0
		}
	}
	else if (wstyle == 2) {
		if (openfilelist[fd].attribute==0) {
			openfilelist[fd].count = openfilelist[fd].length;
		}
		else if (openfilelist[fd].attribute == 1 && openfilelist[fd].length != 0) {
			openfilelist[fd].count = openfilelist[fd].length - 1;
		}
	}

	unsigned char* buf = (unsigned char*)malloc(BLOCKSIZE);
	if (buf == NULL) {
		cout << "do_write apply memory failed" << endl;
		return -1;
	}

	//get off & block num
	int blockNum = openfilelist[fd].first;//current block num
	fat * fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;
	int off = openfilelist[fd].count;
	while (off >= BLOCKSIZE) {
		blockNum = fatPtr->id;
		if (blockNum == END) {
			blockNum = getFreeBLOCK();
			if (blockNum == END) {
				cout << "Block not enough" << endl;
				return -1;
			}
		}
		else {
			fatPtr->id = blockNum;//store next block num
			fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;
			fatPtr->id = END;
		}
		off -= BLOCKSIZE;
		fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;
	}

	//writing
	unsigned char * blockPtr = (unsigned char *)(myvhard + BLOCKSIZE*blockNum);
	char *textPtr = text;
	int lenTmp = 0;
	while (len > lenTmp) {
		memcpy(buf, blockPtr, BLOCKSIZE);//block to buf
		for (;off < BLOCKSIZE;off++) {
			*(buf + off) = *textPtr;//text to buf
			textPtr++;
			lenTmp++;
			if (len == lenTmp) {
				break;
			}
		}

		memcpy(blockPtr, buf, BLOCKSIZE);

		if (off == BLOCKSIZE || lenTmp != len) {//have remaining data to write
			off = 0;
			blockNum = fatPtr->id;
			if (blockNum == END) {
				blockNum = getFreeBLOCK();
				if (blockNum == END) {
					cout << "Block not enough!" << endl;
					return -1;
				}
				else {
					fatPtr->id = blockNum;
					fatPtr = (fat *)(myvhard + BLOCKSIZE) + blockNum;//update
					fatPtr->id = END;
					blockPtr = (unsigned char *)(myvhard + BLOCKSIZE*blockNum);
				}
			}
			else {
				fatPtr = (fat *)(myvhard + BLOCKSIZE) + blockNum;//update
				blockPtr = (unsigned char *)(myvhard + BLOCKSIZE*blockNum);
			}
		}
	}
	free(buf);
	openfilelist[fd].count += lenTmp;//////////////////////////
	if (openfilelist[fd].count > openfilelist[fd].length) {
		openfilelist[fd].length = openfilelist[fd].count;
	}
	
	//update fat
	fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
	int i = blockNum;
	fat* fat2 = (fat*)(myvhard + 3 * BLOCKSIZE);
	memcpy(fat2, fat1, BLOCKSIZE);/////////////////

	return lenTmp;
};

int  do_read(int fd, int len, char *text)
{
	int lenTmp = len;//record reading length

	unsigned char* buf = (unsigned char*)malloc(BLOCKSIZE);
	if (buf==NULL) {
		cout << "do_read apply memory failed" << endl;
		return -1;
	}

	int off = openfilelist[fd].count;
	int blockNum = openfilelist[fd].first;//current block num
	fat *fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;//point current fat
	while (off > BLOCKSIZE) {
		off -= BLOCKSIZE;
		blockNum = fatPtr->id;//next block num
		if (blockNum == END) {
			cout << "do_read's Block Isn't Exist" << endl;
			return -1;
		}
		fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;//confirm block num
	}

	unsigned char* blockPtr = myvhard + blockNum*BLOCKSIZE;//confirm block
	memcpy(buf, blockPtr, BLOCKSIZE);

	char* textPtr = text;
	int restRoom;
	while (len > 0) {
		restRoom = BLOCKSIZE - off;
		if (restRoom >= len) {
			memcpy(textPtr, buf + off, len);
			textPtr += len;
			off += len;
			openfilelist[fd].count += len;
			len = 0;
		}
		else {
			memcpy(textPtr, buf + off, restRoom);
			textPtr += restRoom;
			off = 0;
			len -= BLOCKSIZE - restRoom;
			
			blockNum = fatPtr->id;
			if (blockNum == END) {
				cout << "length is out of range" << endl;
				break;
			}

			fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;//update
			blockPtr = myvhard + blockNum*BLOCKSIZE;
			memcpy(buf,blockPtr, BLOCKSIZE);
		}
	}
	free(buf);
	return lenTmp - len;
};

void show_help()
{
	cout << "命令\t\t参数\t\t\t命令介绍" << endl;
	cout << "cd\t\t目录名(路径名)\t\t切换当前目录到指定目录" << endl;
	cout << "mkdir\t\t目录名\t\t\t在当前目录创建新目录" << endl;
	cout << "rmdir\t\t目录名\t\t\t在当前目录删除指定目录" << endl;
	cout << "ls\t\t无\t\t\t显示当前目录下的目录和文件" << endl;
	cout << "create\t\t文件名\t\t\t在当前目录下创建指定文件" << endl;
	cout << "rm\t\t文件名\t\t\t在当前目录下删除指定文件" << endl;
	cout << "open\t\t文件名\t\t\t在当前目录下打开指定文件" << endl;
	cout << "write\t\t无\t\t\t在打开文件状态下，写该文件" << endl;
	cout << "read\t\t无\t\t\t在打开文件状态下，读取该文件" << endl;
	cout << "close\t\t无\t\t\t在打开文件状态下，关闭该文件" << endl;
	cout << "exit\t\t无\t\t\t退出系统\n" << endl;
};


#endif
