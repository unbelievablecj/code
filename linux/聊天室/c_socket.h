/*对socket相关函数的简单封装后的函数的声明
*用ifndef防止头文件被重复包含
*/
#ifndef C_SOCKET_H
#define C_SOCKET_H


/* 套接字操作集 */
// 设置服务器套接字
int socket_set_server(unsigned short port, int listen_num);
// 接受请求
int socket_accept(int socket_server);
// 发送
int socket_write(int socket_fd, const void * buf, int len);
// 读
int socket_read(int socket_fd, void * buf, int len);
// 关闭套接字
void socket_close(int socket_fd);
// 连接服务器
int socket_connect(const char * ip, unsigned short port);


#endif
