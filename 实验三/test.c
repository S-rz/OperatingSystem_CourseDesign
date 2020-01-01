#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#define BUFFER_SIZE 102400
int main(void)
{
	int dev,i=0;
	char c;
 	char source[BUFFER_SIZE];//写入MyDev设备的内容
 	char goal[BUFFER_SIZE]; //MyDev设备的内容读入到该goal中
	printf("input the string you want to write in your device:\n");
	while((c=getchar())!='\n')
	{
		source[i++]=c;
	}
	printf("\n");
 	if((dev=open("/dev/MyDev",O_RDWR))==-1) //打开MyDev设备失败
  		printf("FAIL to open MyDev!\n");
	else//成功
  		printf("SUCCESS to open MyDev!\n");
 	printf("Write:\n%s\n\n",source);
 	write(dev,source,sizeof(source)); //把source中的内容写入MyDev设备
 	lseek(dev,0,SEEK_SET); //把文件指针定位到文件开始的位置
 	read(dev,goal,sizeof(source)); //把MyDev设备中的内容读入到goal中
 	printf("Read:\n%s\n\n",goal);
 	return 0;
}
