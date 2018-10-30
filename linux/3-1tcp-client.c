﻿/*使用面向连接的套接口实现通信*/
/*套接口方式不仅可以实现单机内进程间通信，还可以实现不同计算机进程之间通信*/
/*先运行tcp-server程序，端口号作为参数，实现server程序的监听。*/
/*再运行tcp-client程序，以server所在的“ip地址127.0.0.1”或“主机名（localhost）”为第一参数，相同的端口号作为第二参数连接server*/

/*tcp-client.c */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	int sockfd;
	char buffer[1024];
	struct sockaddr_in server_addr; /*定义服务器端套接口数据结构server_addr */
	struct hostent *host;
	int portnumber,nbytes;
	if(argc!=3)
	{
		fprintf(stderr,"Usage:%s hostname portnumber\a\n",argv[0]);
		exit(1);
	}
	if((host=gethostbyname(argv[1]))==NULL)
	{  /*获得命令行的第二个参数-主机名*/
		fprintf(stderr,"Gethostname error\n");
		exit(1);
	}
	if((portnumber=atoi(argv[2]))<0)
	{  /*获得命令行的第三个参数--端口号，atoi()把字符串转换成整型数*/
		fprintf(stderr,"Usage:%s hostname portnumber\a\n",argv[0]);
		exit(1);
	}
	/* 客户程序开始建立 sockfd描述符 */
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
		exit(1); 
	} 
	/*客户程序填充服务端的资料*/
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(portnumber);
	server_addr.sin_addr=*((struct in_addr *)host->h_addr);
	/*客户程序发起连接请求*/
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
		exit(1);
	}
	/*连接成功，调用read读取服务器发送来的数据*/
	if((nbytes=read(sockfd,buffer,1024))==-1)
	{
		fprintf(stderr,"Read Error:%s\n",strerror(errno));
		exit(1);
	} 
	buffer[nbytes]='\0';
	printf("I have received:%s\n",buffer);   /*输出接收到的数据*/
	close(sockfd);   /*结束通信*/ 
	exit(0);
}

