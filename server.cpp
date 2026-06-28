#include<iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string.h>
#include <sys/epoll.h>
#include<csignal>
#include "logging.hpp"

#define PORT 8080

using namespace std;

int main()
{
    int fd=socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1)
    {
        perror("Failed to open socket!");
        exit(1);
    }
    signal(SIGINT, signal_handler);
    sockaddr_in server_addr{};
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_family=AF_INET;
    int bindReturn=bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bindReturn<0)
    {
        perror("Could not bind:");
        exit(1);
    }
    int ret=listen(fd, 5);
    if(ret==-1)
    {
        perror("Could not listen!");
        exit(1);
    }
    cout<<"Server listening on port: "<<PORT<<endl;
    sockaddr_in client_addr{};
    int client_len=sizeof(client_addr);
    int clientSocketFd=accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_len);
    if(clientSocketFd<0)
    {
        perror("Connection with client failed!");
        exit(1);
    }
    epoll_event ev;
    int epfd=epoll_create1(0);
    ev.events=EPOLLIN;
    ev.data.fd=STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    ev.data.fd=clientSocketFd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, clientSocketFd, &ev);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    cout<<"Connection established with "<< client_ip<<":"<< ntohs(client_addr.sin_port)<<endl;
    logEvent("Connected to client "+(string)client_ip+":"+to_string(ntohs(client_addr.sin_port)));
    epoll_event events[2];
    bool canLoop=true;
    while(canLoop)
    {
        int n=epoll_wait(epfd, events, 2, -1);
        for(int i=0;i<n;i++)
        {
            int fd=events[i].data.fd;
            if(fd==STDIN_FILENO)
            {
                char msg[1024]={0};
                cin.getline(msg, 1024);
                send(clientSocketFd, (char *)msg, sizeof(msg), 0);
                logEvent("Server sent:"+(string)msg);
            }
            else if(fd==clientSocketFd)
            {
                char buffer[1024]={0};
                int n=read(clientSocketFd, buffer, sizeof(buffer));
                if(strlen(buffer)==0)
                {
                    cout<<"Connection closing"<<endl;
                    logEvent("Connection with client "+(string)client_ip+" terminates");
                    canLoop=false;
                    break;
                }
                cout<<"Message received from client: "<<buffer<<endl;
                logEvent("Message received from client "+(string)client_ip+":"+(string)buffer);
            }
        }
    }
    return 0;
}