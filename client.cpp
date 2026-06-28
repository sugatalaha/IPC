#include<iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string.h>
#include<sys/epoll.h>

using namespace std;

int main()
{
    int clientSocketFd=socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocketFd<0)
    {
        perror("Could not open client socket!");
        exit(1);
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    int ret=connect(clientSocketFd, (sockaddr *)&serverAddress, sizeof(serverAddress));
    if(ret<0)
    {
        perror("Could not connect to server !");
        exit(1);
    }
    while(true)
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
                char receivedMsg[1024];
                read(clientSocketFd, (char *)receivedMsg, sizeof(receivedMsg) );
                cout<<"Received from server: "<<receivedMsg<<endl;
            }
            else if(fd==STDIN_FILENO)
            {
                char msg[1024];
                cin>>msg;
                send(clientSocketFd, (char *)msg, sizeof(msg), 0);
            }
        }
    }
}