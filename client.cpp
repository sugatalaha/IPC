#include<iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string.h>
#include<sys/epoll.h>

using namespace std;

int main(int argc, char *argv[])
{   
    if(argc<2)
    {
        cout<<"Provide IP address of server to connect.";
        return 0;
    }
    
    char *ip_addr=argv[1];
    int clientSocketFd=socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocketFd<0)
    {
        perror("Could not open client socket!");
        exit(1);
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    if(inet_pton(AF_INET, ip_addr, &(serverAddress.sin_addr))<=0)
    {
        perror("IP address cant be converted to network byte order:");
        exit(1);
    }
    int ret=connect(clientSocketFd, (sockaddr *)&serverAddress, sizeof(serverAddress));
    if(ret<0)
    {
        perror("Could not connect to server !");
        exit(1);
    }
    bool canLoop=true;
    while(canLoop)
    {
        int epfd=epoll_create1(0);
        epoll_event ev;
        ev.events=EPOLLIN;
        ev.data.fd=STDIN_FILENO;
        epoll_ctl(epfd, EPOLL_CTL_ADD,STDIN_FILENO, &ev);
        ev.data.fd=clientSocketFd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, clientSocketFd, &ev);
        epoll_event events[2];
        int n=epoll_wait(epfd, events, 2, -1);
        for(int i=0;i<n;i++)
        {
            int fd=events[i].data.fd;
            if(fd==clientSocketFd)
            {
                char receivedMsg[1024]={0};
                read(clientSocketFd, (char *)receivedMsg, sizeof(receivedMsg) );
                if(strlen(receivedMsg)==0)
                {
                    canLoop=false;
                    break;
                }
                cout<<"Received from server: "<<receivedMsg<<endl;
            }
            else if(fd==STDIN_FILENO)
            {
                char msg[1024]={0};
                cin.getline(msg, 1024);
                send(clientSocketFd, (char *)msg, sizeof(msg), 0);
            }
        }
    }
}