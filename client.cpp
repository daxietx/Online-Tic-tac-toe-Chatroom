#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include "user.h"
using namespace std;

#define MAXONLINE 10 
#define MAXBUFLEN 1024

int SERVERPORT;
char SERVERIP[40]; //127.0.0.1
int highestsock = 0;

int listener = -1;

fd_set master;
fd_set readfds;

void read_from_socket()
{
	char sendbuf[MAXBUFLEN];
    char recvbuf[MAXBUFLEN];
    
	int n;
	
	if(FD_ISSET(fileno(stdin),&readfds))
	{
        string cmd;
        memset(sendbuf,'\0',MAXBUFLEN);  
        fgets(sendbuf,MAXBUFLEN,stdin);
		if(sendbuf[strlen(sendbuf)-1]=='\n')
			sendbuf[strlen(sendbuf)-1] = '\0';       
        cmd = string(sendbuf);            
        cmd = cmd + "\n"+"\n";
        send(listener,(void*)cmd.c_str(),cmd.size(),0);
        fflush(stdin);
    }
	if(FD_ISSET(listener,&readfds))
	{	
		n = recv(listener,recvbuf,MAXBUFLEN,0);
		if(n<=0)
		{
			if(n==0)
			{
				printf("Connection closed by foreign host.\n");
				close(listener);
				FD_CLR(listener,&master);
				listener = -1;
				exit(0);
			}
			if(n<0)
			{
				printf("Receive error.\n");
				close(listener);
				FD_CLR(listener,&master);
				listener = -1;				
				exit(-1);
			}			
		}
		else
		{
            string print = string(recvbuf);
            print = print.substr(0,n);
            cout<<print;
            fflush(stdout);           
		}	
	}
}

void client_run()
{ 
	struct sockaddr_in server_addr;

	FD_ZERO(&master);
	FD_ZERO(&readfds);
	
	//--------------------connect to server--------------------
	if((listener=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("Fail to create connect socket.\n");
		exit(-1);
	}
	// set listener address to be reused immediately after release
	int a=1;
	if(setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int))==-1)
	{
		perror("Fail to set socket option.\n");
		exit(-1);
	}	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	inet_aton(SERVERIP,(struct in_addr*)&server_addr.sin_addr.s_addr);
	memset(&(server_addr.sin_zero),'\0',8);
	if(connect(listener,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
	{
		printf("Fail to connect to server at port %d.\n",SERVERPORT);
		close(listener);
		exit(-1);
	}
	//--------------------connect to server--------------------
		
	FD_SET(listener,&master);
	if(listener>highestsock)
	{
		highestsock = listener;
	}
	
	FD_SET(fileno(stdin),&master);
	if(fileno(stdin)>highestsock)
	{
		highestsock = fileno(stdin);
	}	
	
	while(1)  // keep listen the connection request
	{
		readfds = master;
		if(select(highestsock+1,&readfds,NULL,NULL,NULL) == -1)
		{
			perror("Fail to select.\n");
			exit(-1);
		}	
		read_from_socket();	
	}	
	close(listener);
}

int main(int argc, char **argv)
{
	if(argc<2)
	{
		perror("Insufficient parameter.\n");
		return 1;
	}
	else 
	{
		memcpy(SERVERIP,argv[1],strlen(argv[1])); //get server IP
		SERVERPORT = atoi(argv[2]); //get server port
	}
	
	client_run();
	
	return 0;
}