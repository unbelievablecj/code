
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>
int main(int argc ,char* argv[]){
    
    int sockfd;
    char buffer[1024];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int portnumber,nbytes;
    if(argc!=3)
    {
        fprintf(stdout,"usage:%s hostname portnumber\a\n",argv[0]);
        exit(1);
    }
    if((host=gethostbyname(argv[1]))==NULL)
    {
        fprintf(stdout,"Gethostname error\n");
        exit(1);
    }
    if((portnumber=atoi(argv[2]))<0)
    {
        fprintf(stdout,"Usage:%s hostname portnumber\a\n",argv[0]);
        exit(1);
    }
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stdout,"Socket Error:%s\a\n",strerror(errno));
        exit(1);
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(portnumber);
    server_addr.sin_addr=*((struct in_addr *)host->h_addr);
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
    {
        fprintf(stdout,"Connect Error\n");
        exit(1);
    }
    if((nbytes=read(sockfd,buffer,1024))==-1)
    {
        fprintf(stdout,"Read Error");
        exit(1);
    }
    buffer[nbytes]='\0';
    printf("I hava received:%s\n",buffer);
    close(sockfd);
    return 0;
}
