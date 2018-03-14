#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
void *create_thread(void *);

struct database{
	int account_number;
	int balance; 
};
struct database array_record[1000];

pthread_mutex_t mutex;
int main (int argc, char *argv[]){
	int i =0;
	for ( i = 0; i < 1000; i++)
	{
		array_record[i].account_number = 0;
	}
	int socket_desc, client_sock,clilen,read_size;
	int *new_sock;
	struct sockaddr_in server, client;
	
//create background server socket
	socket_desc = socket(AF_INET, SOCK_STREAM,0);
	if(socket_desc == -1){
		printf("Could not create socket");

	}
	puts("Back ground server Socket created");

//Prepare sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

//bind the socket
	if(bind(socket_desc,(struct sockaddr *) &server,sizeof(server)) <0){
		perror("Bind failed");
		return 1;
	}
	puts("Back end Server is up and running \n");
//listen to accept connections from front end server
	listen(socket_desc,100);
	puts("Waiting for incomming connections\n");

//Accept Connections

	clilen = sizeof(struct sockaddr_in);
	while(client_sock = accept(socket_desc,(struct sockaddr *) &client,(socklen_t *)&clilen)){
		//printf("Connection accepted \n");
		new_sock = malloc(1);	
		*new_sock = client_sock;
		pthread_t pid;
		if(pthread_create(&pid,NULL,create_thread,(void*)new_sock) < 0)	{
			perror("No thread created");
			return 1;
		}

		if(client_sock<0){
			perror("Accept failed");
			return 1;
		}
		
	}
	return 0;
}


void *create_thread(void *socketdesc){
	int client_sock = *(int *)socketdesc;
	//printf("In backgroung thread\n");

	while(1){

		//Recieve vote 
		char vote[256];	
		bzero(vote,256);
		int r1 = read(client_sock,vote,sizeof(vote));
		printf("%s\n",vote);
		
		if(r1>0){
			int s1 = send(client_sock,"COMMIT",256,0);
		}
		
		char global_commit_Status[256];
		bzero(global_commit_Status,256);
		int r3 = read(client_sock,global_commit_Status,sizeof(global_commit_Status));
		
		printf("%s\n",global_commit_Status);
		int cmpi= strcmp(global_commit_Status,"GLOBAL COMMIT");
		int cmpi2 = strcmp(global_commit_Status,"GLOBAL ABORT");

		char transaction[256];	
		bzero(transaction,256);
		char *split;
		char updated_transaction[256];
		bzero(updated_transaction,256);
		//Read transaction
		if(cmpi2!=0){
			int r4 = read(client_sock,transaction,sizeof(transaction));
			printf("Transaction is: %s\n",transaction);
		}else{
		//GOT ABORT	
			
		//Abort the transaction and send it to front server
			sprintf(updated_transaction,"Technical Problem: Please try again later");
			int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);
			printf("\n");
			bzero(vote,256);
			bzero(global_commit_Status,256);
			bzero(updated_transaction,256);
			bzero(transaction,256);
			//break;

		}
		if(cmpi==0&&cmpi2!=0){
			//printf("Debug 7.0");
			split = strtok (transaction," ,.-");


			while(split!= NULL){

				//what type of transaction
				strcpy(transaction,split);

				
				int char_count = 0;
				while(transaction[char_count]){
					transaction[char_count] = toupper(transaction[char_count]);
					char_count++;
				}
				split = strtok (NULL, " ,.-");
				if(strcmp(transaction,"CREATE")==0){
				
					pthread_mutex_lock(&mutex);
					int index = 1;
					while(array_record[index].account_number!=0){
						index++;
					}		
					array_record[index].account_number = index;
					array_record[index].balance = atoi(split);
					split = strtok (NULL, " ,.-");
					sprintf(updated_transaction,"OK %d",array_record[index].account_number);
					int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);
					pthread_mutex_unlock(&mutex);
					
				}

				else if(strcmp(transaction,"QUERY")==0){
					
					int acc = atoi(split);
					
					if(array_record[acc].account_number==0){
						sprintf(updated_transaction,"ERR: Account %d does not exist",acc);
						split = strtok (NULL, " ,.-");
						int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);
					}
					else{
						sprintf(updated_transaction,"OK %d",array_record[acc].balance);
						split = strtok (NULL, " ,.-");
						int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);
					}
				}

				else if(strcmp(transaction,"UPDATE")==0){
					//Update the balance from account number and update it into structure array		
					pthread_mutex_lock(&mutex);
					int acc = atoi(split);
					split = strtok (NULL, " ,.-");
					int value = atoi(split);
					if(array_record[acc].account_number==0){
						sprintf(updated_transaction,"Err Account %d does not exist",acc);
						split = strtok (NULL, " ,.-");
						int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);
					}

					else{
						array_record[acc].balance = (value);
						sprintf(updated_transaction,"OK %d",array_record[acc].balance);	
						split = strtok (NULL, " ,.-");
						int s1 = send(client_sock,updated_transaction,sizeof(updated_transaction),0);

					}
					pthread_mutex_unlock(&mutex);
				}
				
				else{
					sprintf(updated_transaction,"Err Invalid Transaction");
					split = strtok (NULL, " ,.-");
					int s1= send(client_sock, updated_transaction, sizeof(updated_transaction),0);

				}

			}

		}

		bzero(updated_transaction,256);

	}	

	close(client_sock);

}
