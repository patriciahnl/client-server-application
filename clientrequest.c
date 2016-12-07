//IPv6 Client
//Florea Adina, Szasz Patricia
//TST Engleza gr. 2341


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//The client will be connecting on destination port 80 (default port for http)
#define PORT "80"

#define MAXDATASIZE 10000

//function that returns the address of sockaddr_in6
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main()
{
	int sockfd, numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char hostname[] = "www.4or6.";
	FILE *htmlfile = fopen("index.html", "w+");
	
	
	//init hints structure
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	
	//get the internet address
	
	if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	//loop through all the results returned by getaddrinfo and connect to the first possible
	
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		     p->ai_protocol)) == -1) {
			perror("Client: socket error\n");
			continue;
		}
		
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("Client: socket error\n");
			continue;
		}
		
		break;
	}
	
	if (p == NULL) {
		fprintf(stderr, "Client: failed to connect\n");
		return 2;
	}

	//address in human readable format	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		s, sizeof s);
	printf("Client: connecting to %s\n", s);
	
	freeaddrinfo(servinfo);

	//Sending http request
	char *request="Get / HTTP/1.0\r\n\r\n";
	char buffer[MAXDATASIZE];
	
	printf("\n%s", request);
	
	if (send(sockfd, request, strlen(request), 0) < 0)
		printf("Error sending request.");
	else
		printf("Sent request.");

	//while loop for recv - recieve until EOF is reached
	
	do {	
		if (htmlfile == NULL){
			printf("Could not open html file");
			exit(-1);
		}
		
		fprintf(htmlfile, buffer);
		bzero(buffer,10000);

	}while ( recv(sockfd, buffer, MAXDATASIZE-1, 0) != 0 );
	 
	
	//when finished recieving data, close the file and the socket
	
	fclose(htmlfile);
	close(sockfd);
	return 0;
 
	
	
}
