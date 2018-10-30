#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "c_socket.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int cli_snd_flag_id;
char changed_buf[1024];
int cli_num = 0;
FILE * fp;

int end_cli_sid;
//读数据线程函数
void * pth_recv(void * arg)
{
	int i;
	int cli_pid = *((int *)arg);
	char buf[1024];
	
	while(1)
	{
		memset(buf,0x00,strlen(buf));
		//读取某一客户端发来的数据,修改下读取的数据，
		//并发送信号给连接其他客户端的正在处于阻塞等待状态的写线程，使其脱离阻塞状态，继续执行
		if(socket_read(cli_pid, buf, 900) > 0)
		{
			if((strncmp(buf,"end",3) == 0))
			{
				cli_num--;
				end_cli_sid = cli_pid;
				break;
			}
			sprintf(changed_buf,"receive from the client(%d)--->:%s\n",cli_pid,buf);

			fp = fopen("./chat_record.txt","a+");
			fprintf(fp,"%s",changed_buf);

			fclose(fp);
			changed_buf[strlen(changed_buf)-1] = '\0';
			strcpy(buf,changed_buf);
			cli_snd_flag_id = cli_pid;

			for(i = 0;i < cli_num;i++)
				pthread_cond_signal(&cond);
		}
	}
	
	return;
}
//写数据线程函数
void * pth_snd(void * arg)
{
	int cli_pid = *((int *)arg);
	char buf[1024];

	if(cli_pid == end_cli_sid)
	{
		return;
	}
	//将将读到的数据发往其他客户端
	while(1)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex);

		if(cli_pid != cli_snd_flag_id)
		{
			socket_write(cli_pid, changed_buf, strlen(changed_buf)+1);
		}
		
		pthread_mutex_unlock(&mutex);

	}
	
	return;	
}


int main(int argc,char *argv[])
{
	int ser_sid;
	int cli_sid;

	unsigned short port = 5001;
	int listen_num = 20;

	pthread_mutex_init(&mutex,NULL); //线程互斥锁的初始化
	pthread_cond_init(&cond,NULL); //条件变量的初始化

  //创建服务器的socket文件描述符，并进行配置
	ser_sid = socket_set_server(port,listen_num);

	while(1)
	{
		pthread_t recv_pid,snd_pid;

		//接收客户端的连接请求
		cli_sid = socket_accept(ser_sid);
		cli_num++;
		printf("the connected client pid:%d\n",cli_sid);
		
		//创建读数据线程
		if(pthread_create(&recv_pid,NULL,pth_recv,(void *)&cli_sid) != 0)
			perror("pthread_create recv_pid error.");
		//创建写数据线程
		if(pthread_create(&snd_pid,NULL,pth_snd,(void *)&cli_sid) != 0)
			perror("pthread_create snd_pid error.");		
	}

	//关闭服务器的socket文件描述符
	socket_close(ser_sid);
	return 0;
}

