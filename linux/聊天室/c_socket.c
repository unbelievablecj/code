/*对socket相关函数的简单封装的具体实现*/
#include "c_socket.h"
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <strings.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
/*对服务器的socket,bind,listen的简单封装
*参数port为要绑定的端口号，listen_num为同一时间监听的客户端的最大数目
*返回值：创建并配置好的服务器socket文件描述符
*/
int socket_set_server(unsigned short port, int listen_num)
{
    int ret;
	struct sockaddr_in ser_addr;
	int ser_sid;
	int flag,len = sizeof(len);
	
	if( ( ser_sid = socket(AF_INET,SOCK_STREAM,0)) == -1 )
	{
		perror("socket error.");
		exit(-1);
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(port);
	ser_addr.sin_addr.s_addr = htons(INADDR_ANY);

	// 防止提示端口占用
	if( setsockopt(ser_sid, SOL_SOCKET, SO_REUSEADDR, &flag, len) == -1)
    {
        perror("setsockopt");
        exit(-1);
    }
	
	ret = bind(ser_sid,(struct sockaddr *)&ser_addr,sizeof(struct sockaddr));
	if(ret == -1)
	{
		perror("bind error.");
		exit(-1);
	}

	ret = listen(ser_sid,listen_num);
	if(ret == -1)
	{
		perror("listen error.");
		exit(-1);
	}

	return ser_sid;
}

/*对accept函数的简单封装
*参数socket_server为要连接的服务器的socket文件描述符
*返回值：发连接的客户端的的socket文件描述符
*/
int socket_accept(int socket_server)
{
    int cli_sid;
  	struct sockaddr client_address;
	socklen_t client_len;
		
	cli_sid = accept(socket_server,&client_address, &client_len);	
    
	return cli_sid;
}
/*对write函数的简单封装，防止数据的丢失
*参数socket_fd为要连接的服务器的socket文件描述符
*参数buf为存储数据的缓冲区的地址
*参数len为要接收数据的字节数
*返回值：0，以后可以定义其他值，以便进行错误检索
*/
int socket_write(int socket_fd, const void * buf, int len)
{
    int ret = 0;
    int send_len = 0;
    fd_set fds;

    ret = send(socket_fd, buf, len, 0);
    if (ret < 0)
    {
        ret = -errno;
        printf("Send error, error code is %d\n", -errno);
        goto exit;
    }
    send_len += ret;

		FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);

    while (send_len < len)
    {
        
        ret = select(socket_fd + 1, NULL, &fds, NULL, NULL);
        if (ret <= 0)
        {
            ret = -errno;
            printf("Select error, error code is %d", ret);
            goto exit;
        }

        ret = send(socket_fd, ((char *)buf) + send_len, len - send_len, 0);
        if (ret < 0)
        {
            ret = -errno;
            printf("Send error, error code is %d\n", -errno);
            goto exit;
        }
        send_len += ret;
    }
    ret = 0;

exit:
    return ret;
}

int socket_read(int socket_fd, void * buf, int len)
{
    int ret = 0;
    int recv_len = 0;
		recv_len = recv(socket_fd, (char *)buf, len, 0);
    ret = recv_len;
exit:
    return ret;
}

void socket_close(int socket_fd)
{
    close(socket_fd);
    sleep(2);
}

/*对connect函数的简单封装
*参数ip为要连接的服务器的ip地址
*参数port为要连接的服务器的端口号
*返回值：连接服务器成功后的socket文件描述符
*/
int socket_connect(const char * ip, unsigned short port)
{
    int ret = 0;
    int err = 0;

    int socket_fd = 0;
    int tmp_flag = 0;

    socklen_t socket_len = sizeof(err);
    struct sockaddr_in server_addr;

    fd_set fds;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socket_fd)
    {
        ret = -errno;
        printf("Get socket error: %d\n", -errno);
        goto exit;
    }
    
    // nonblock
    tmp_flag = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, tmp_flag | O_NONBLOCK);

    bzero(&(server_addr.sin_zero), 8);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        if (EINPROGRESS != errno)
        {
            ret = -errno;
            printf("Enter connect progress unsuccessfully, error code is : %d.\n", ret);
            goto exit;
        }
        
        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);
        ret = select(socket_fd + 1, NULL, &fds, NULL, NULL);
        if (ret < 0)
        {
            ret = -errno;
            printf("Select error, error code is : %d.\n", ret);
            goto exit;
        }

        if (0 == ret)
        {
            printf("Select timeout.\n", ret);
        }

        getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &err, &socket_len);
        if (err != 0)
        {
            ret = -errno;
            printf("Connect error, error code is : %d.\n", ret);
            goto exit;
        }
    }

    ret = socket_fd;

exit:
    if (ret < 0 && socket_fd != -1)
    {
        close(socket_fd);
    }

    return ret;
}



