#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "22016"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

//function that does something we don't know

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

int main(void)
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage addr_of_connector;
    socklen_t addr_size;
    struct sigaction sa;
    int value=1;
    char s[INET_ADDRSTRLEN];
    int rv;
    int read_size,sent;
    long numbytes;
    char *buff;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
    //loop through all the results and bind to the first
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "failed to bind\n");
        return 2;
    }


    freeaddrinfo(servinfo);
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    //sa.sa_handler = sigchld_handler;
    //sigemptyset(&sa.sa_mask);
    //sa.sa_flags = SA_RESTART;
    //if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    //    perror("sigaction");
    //    exit(1);
    //}

    printf("server: waiting for connections...\n");
    
    //accept an incoming connection
   while(1){
    
   	addr_size = sizeof addr_of_connector;
    new_fd = accept(sockfd, (struct sockaddr *)&addr_of_connector, &addr_size);
    
    
    //ready to connect on the socket f desc new_fd
    inet_ntop(addr_of_connector.ss_family,
            get_in_addr((struct sockaddr *)&addr_of_connector),
            s, sizeof s);
     printf("\nserver got connection from %s\n", s);
     
     char buffer[2000];
     //Recieving message from client
     while( (read_size = recv(new_fd, buffer, 2000, 0)) > 0 ) {
         printf("recieved from client %s\n", buffer);
         if (strncmp(buffer,"10#",3) == 0 ){
		sent=send(new_fd, "Method implemented\n", 18,0);
		if(system("./client") != 0){
			perror("error executing client");
			return 1;
		}
	        printf("\nexecuted client. now reading the file \n");
		
    		FILE *data = fopen("index.html", "r");
		fseek(data,0L, SEEK_END);
		numbytes=ftell(data);
		
		fseek(data, 0L, SEEK_SET);
		
		buff=(char*)calloc(numbytes,sizeof(char));
		
		if(buff == NULL)
			return 1;

		fread(buff,sizeof(char),numbytes, data);
		fclose(data);
		
		sent=send(new_fd, buff, numbytes, 0);
		free(buff);	
            	bzero(buffer, 2000); 
         }
         else
             sent=send(new_fd, "Method not implemented", 22,0);
         if (sent == -1)
	     perror("error sending message to client");
      }
      if(read_size == 0)
	{
	   printf("\nClient disconnected. closing the socket");
           close(new_fd);
	}
      if(read_size == -1)
      {
          printf("\nrecv failed. closing the socket");
          close(new_fd);
      }	  
   }
    close(sockfd); 
    return 0;
}

 























	
   

