/*��socket��غ����ļ򵥷�װ��ĺ���������
*��ifndef��ֹͷ�ļ����ظ�����
*/
#ifndef C_SOCKET_H
#define C_SOCKET_H


/* �׽��ֲ����� */
// ���÷������׽���
int socket_set_server(unsigned short port, int listen_num);
// ��������
int socket_accept(int socket_server);
// ����
int socket_write(int socket_fd, const void * buf, int len);
// ��
int socket_read(int socket_fd, void * buf, int len);
// �ر��׽���
void socket_close(int socket_fd);
// ���ӷ�����
int socket_connect(const char * ip, unsigned short port);


#endif
