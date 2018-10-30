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
//�������̺߳���
void * pth_recv(void * arg)
{
	int i;
	int cli_pid = *((int *)arg);
	char buf[1024];
	
	while(1)
	{
		memset(buf,0x00,strlen(buf));
		//��ȡĳһ�ͻ��˷���������,�޸��¶�ȡ�����ݣ�
		//�������źŸ����������ͻ��˵����ڴ��������ȴ�״̬��д�̣߳�ʹ����������״̬������ִ��
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
//д�����̺߳���
void * pth_snd(void * arg)
{
	int cli_pid = *((int *)arg);
	char buf[1024];

	if(cli_pid == end_cli_sid)
	{
		return;
	}
	//�������������ݷ��������ͻ���
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

	pthread_mutex_init(&mutex,NULL); //�̻߳������ĳ�ʼ��
	pthread_cond_init(&cond,NULL); //���������ĳ�ʼ��

  //������������socket�ļ�������������������
	ser_sid = socket_set_server(port,listen_num);

	while(1)
	{
		pthread_t recv_pid,snd_pid;

		//���տͻ��˵���������
		cli_sid = socket_accept(ser_sid);
		cli_num++;
		printf("the connected client pid:%d\n",cli_sid);
		
		//�����������߳�
		if(pthread_create(&recv_pid,NULL,pth_recv,(void *)&cli_sid) != 0)
			perror("pthread_create recv_pid error.");
		//����д�����߳�
		if(pthread_create(&snd_pid,NULL,pth_snd,(void *)&cli_sid) != 0)
			perror("pthread_create snd_pid error.");		
	}

	//�رշ�������socket�ļ�������
	socket_close(ser_sid);
	return 0;
}

