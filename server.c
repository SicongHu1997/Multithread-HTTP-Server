#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/io.h>
#include "server.h"

int count = 0;
pthread_t threads[5];

int main(int argc, char **argv)
{
	int sockfd;
	int portno;
	struct sockaddr_in serv_addr;
	
	//socket()
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	// bind()
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0) 
	{
		perror("ERROR on binding");
		exit(1);
	}
	// listen()
	listen(sockfd,5);	
	while(1){
		struct sockaddr_in cli_addr;
		socklen_t clilen;
		int newsockfd;
		struct arg_struct args;
		// accept()
		clilen = sizeof(cli_addr);
		newsockfd = accept(	sockfd, (struct sockaddr *) &cli_addr, 
						&clilen);
		args.arg1 = newsockfd;
		stpcpy(args.arg2,argv[2]);
		// multithread
		pthread_create(threads+(count++), NULL, (void*)handle_request, (void *)&args);
	}
	close(sockfd);
	return 0; 
}

void handle_request(void *arguments){
	char buffer[1024];
	char request[1024];
	char uri[1024];
	char type[1024];
	char dir_copy[2048];
	char get[]="GET";
	char html[]="html";
	char js[]="js";
	char jpeg[]="jpg";
	char css[]="css";
	char space[]=" ";
	char dot[]=".";
	struct arg_struct *args = arguments;
	int socket = args->arg1;
	char failheader[] = "HTTP/1.1 404\r\n";
	char htmlheader[] = "HTTP/1.1 200 Ok\r\n"
	"Content-Type: text/html\r\n\r\n";
	char jpegheader[] = "HTTP/1.1 200 Ok\r\n"
	"Content-Type: image/jpeg\r\n\r\n";
	char jsheader[] = "HTTP/1.1 200 Ok\r\n"
	"Content-Type: application/javascript\r\n\r\n";
	char cssheader[] = "HTTP/1.1 200 Ok\r\n"
	"Content-Type: text/css\r\n\r\n";
	bzero(buffer,1024);
	bzero(request,1024);
	bzero(uri,1024);
	bzero(dir_copy,2048);
	//read the request
	read(socket,buffer,1024);
	//check if it's a get request
	strncpy(request,buffer,3);
	if(!strcmp(request,get)){
		//get the uri
		int i = strcspn(buffer+4,space);
		strncpy(uri,buffer+4,i);
		i=0;
		//get the filetype
		i = strcspn(uri,dot);
		strncpy(type,uri+i+1,5);
		i=0;
		//combine path to web root with uri
		strcpy(dir_copy,args->arg2);
		strcat(dir_copy,uri);
		if(!access(dir_copy,0)){
			//handle different file types
			if(!strcmp(type,html)){
				write(socket, htmlheader, sizeof(htmlheader) - 1);
			}
			else if(!strcmp(type,js)){
				write(socket, jsheader, sizeof(jsheader) - 1);
			}
			else if(!strcmp(type,css)){
				write(socket, cssheader, sizeof(cssheader) - 1);
			}
			else if(!strcmp(type,jpeg)){
				write(socket, jpegheader, sizeof(jpegheader) - 1);
			}
			int f = open(dir_copy, O_RDONLY);
			sendfile(socket, f, NULL, 4294836225);
			close(f);
		}
		else{
			//if file not exist, return 404
			write(socket, failheader, sizeof(failheader) - 1);
		}
	}
	bzero(get,4);
	bzero(css,4);
	bzero(js,4);
	bzero(html,4);
	bzero(jpeg,4);
	bzero(space,2);
	bzero(dot,2);
	close(socket);
}