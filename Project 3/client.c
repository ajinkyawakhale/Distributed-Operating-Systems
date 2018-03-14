/*Client which will connect to front-end server */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{

	if(argc < 3)
	{
		fprintf(stderr, "Usage client  <port number> <hostname>\n");
		exit(1);
	}

	struct hostent *server;
	int portno;
	portno = atoi(argv[1]);
	server = gethostbyname(argv[2]);
	
	if(server == NULL)
	{
		fprintf(stderr, "No such host exists\n");
		exit(1);
	}

	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
	server_addr.sin_port = htons(portno);	


	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		fprintf(stderr, "Client Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "Error connecting\n");
		return 1;
	}
	else{
		printf("OK\n");
	}
	char transaction[256], updated_transaction[256];
	while(1){
		
		printf("Please enter Transaction \n");
		fgets(transaction,256,stdin);
			//printf("%s\n",transaction);

		//Send transaction to front end server
		int n = send(socketfd, transaction, sizeof(transaction),0);


		//Recieve transaction status
		int r = read(socketfd, updated_transaction, sizeof(updated_transaction));

		if(strcmp(updated_transaction,"OK")==0){
			printf("%s\n",updated_transaction);	
			break;
		}
		else{

			printf("%s\n",updated_transaction);
			bzero(updated_transaction,256);
			bzero(transaction,256);
		}
	}

	close(socketfd);
	return 0;



}