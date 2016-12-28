#ifndef  SYSSTRUCT_H
#define  SYSSTRUCT_H

#include "iostream"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;

#define BLOCKSIZE       1024        // ���̿��С
#define SIZE            1024000     // ������̿ռ��С
#define END             65535       // FAT�е��ļ�������־
#define FREE            0           // FAT���̿���б�־
#define MAXOPENFILE     10          // ���ͬʱ���ļ�����
#define MAX_TEXT_SIZE  10000		//��������ı���С

// �ļ����ƿ�
typedef struct FCB {
	char filename[8];           // �ļ���
	char exname[5];             // �ļ���չ��
	unsigned char attribute;    // �ļ������ֶ�,ֵΪ0ʱ��ʾĿ¼�ļ���ֵΪ1ʱ��ʾ�����ļ�
	unsigned short time;        // �ļ�����ʱ��
	unsigned short date;        // �ļ���������
	unsigned short first;       // �ļ���ʼ�̿��
	unsigned long length;       // �ļ����ȣ��ֽ�����
	char free;                  // ��ʾĿ¼���Ƿ�Ϊ�գ���ֵΪ0����ʾ�գ�ֵΪ1����ʾ�ѷ���
} fcb;

// �ļ������
typedef struct FAT {
	unsigned short id;
} fat;

// �û����ļ���
typedef struct USEROPEN {
	char filename[8];           // �ļ���
	char exname[5];             // �ļ���չ��
	unsigned char attribute;    // �ļ������ֶ�
	unsigned short time;        // �ļ�����ʱ��
	unsigned short date;        // �ļ���������
	unsigned short first;       // �ļ���ʼ�̿��
	unsigned long length;       // �ļ����ȣ��������ļ����ֽ�������Ŀ¼�ļ�������Ŀ¼�������
	char free;                  // ��ʾĿ¼���Ƿ�Ϊ�գ���ֵΪ0����ʾ�գ�ֵΪ1����ʾ�ѷ���

	int dirno;                  // ��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ��е��̿��
	int diroff;                 // ��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ���dirno�̿��е�Ŀ¼�����
	char dir[80];               // ��Ӧ���ļ����ڵ�Ŀ¼��������������ټ���ָ���ļ��Ƿ��Ѿ���
	int count;                  // ��дָ�����ļ��е�λ��
	char fcbstate;              // �Ƿ��޸����ļ���FCB�����ݣ�����޸�����Ϊ1������Ϊ0
	char topenfile;             // ��ʾ���û��򿪱����Ƿ�Ϊ�գ���ֵΪ0����ʾΪ�գ������ʾ�ѱ�ĳ���ļ�ռ��
} useropen;

// ������ BLOCK0
typedef struct BLOCK0 {
	char magic_number[8];       // �ļ�ϵͳ��ħ��
	char information[200];
	unsigned short root;        // ��Ŀ¼�ļ�����ʼ�̿��
	unsigned char *startblock;  // �����������������ʼλ��
} block0;

unsigned char *myvhard;             // ָ��������̵���ʼ��ַ
useropen openfilelist[MAXOPENFILE]; // �û����ļ�������
int currfd;                         // ��¼��ǰ�û����ļ�������±�
unsigned char *startp;              // ��¼�����������������ʼλ��
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