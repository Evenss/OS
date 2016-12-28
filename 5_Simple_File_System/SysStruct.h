#ifndef  SYSSTRUCT_H
#define  SYSSTRUCT_H

#include "iostream"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;

#define BLOCKSIZE       1024        // 磁盘块大小
#define SIZE            1024000     // 虚拟磁盘空间大小
#define END             65535       // FAT中的文件结束标志
#define FREE            0           // FAT中盘块空闲标志
#define MAXOPENFILE     10          // 最多同时打开文件个数
#define MAX_TEXT_SIZE  10000		//最大输入文本大小

// 文件控制块
typedef struct FCB {
	char filename[8];           // 文件名
	char exname[5];             // 文件扩展名
	unsigned char attribute;    // 文件属性字段,值为0时表示目录文件，值为1时表示数据文件
	unsigned short time;        // 文件创建时间
	unsigned short date;        // 文件创建日期
	unsigned short first;       // 文件起始盘块号
	unsigned long length;       // 文件长度（字节数）
	char free;                  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配
} fcb;

// 文件分配表
typedef struct FAT {
	unsigned short id;
} fat;

// 用户打开文件表
typedef struct USEROPEN {
	char filename[8];           // 文件名
	char exname[5];             // 文件扩展名
	unsigned char attribute;    // 文件属性字段
	unsigned short time;        // 文件创建时间
	unsigned short date;        // 文件创建日期
	unsigned short first;       // 文件起始盘块号
	unsigned long length;       // 文件长度（对数据文件是字节数，对目录文件可以是目录项个数）
	char free;                  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配

	int dirno;                  // 相应打开文件的目录项在父目录文件中的盘块号
	int diroff;                 // 相应打开文件的目录项在父目录文件的dirno盘块中的目录项序号
	char dir[80];               // 相应打开文件所在的目录名，这样方便快速检查出指定文件是否已经打开
	int count;                  // 读写指针在文件中的位置
	char fcbstate;              // 是否修改了文件的FCB的内容，如果修改了置为1，否则为0
	char topenfile;             // 表示该用户打开表项是否为空，若值为0，表示为空，否则表示已被某打开文件占据
} useropen;

// 引导块 BLOCK0
typedef struct BLOCK0 {
	char magic_number[8];       // 文件系统的魔数
	char information[200];
	unsigned short root;        // 根目录文件的起始盘块号
	unsigned char *startblock;  // 虚拟磁盘上数据区开始位置
} block0;

unsigned char *myvhard;             // 指向虚拟磁盘的起始地址
useropen openfilelist[MAXOPENFILE]; // 用户打开文件表数组
int currfd;                         // 记录当前用户打开文件表项的下标
unsigned char *startp;              // 记录虚拟磁盘上数据区开始位置
const char *FileName = "FileSys.txt";
unsigned char buffer[SIZE];

void startSys();
void exitsys();
void my_format();
void my_mkdir(char *dirname);
void my_rmdir(char *dirname);
void my_ls();
void my_cd(char *dirname);
int  my_create(char *filename);
void my_rm(char *filename);
int  my_open(char *filename);
int  my_close(int fd);
int  my_write(int fd);
int  my_read(int fd,int len);
int  my_read_all(int fd);
int  do_write(int fd, char *text, int len, char wstyle);
int  do_read(int fd, int len, char *text);
void show_help();
int getFreeBLOCK();
int getFreeOpenfilelist();
int find_father_dir(int fd);
#endif