#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void *create_thread(void *);

struct records{
	int account_number;
	char name[60];
	int balance; 
};
struct records array_record[100];

pthread_mutex_t mutex;
int main (int argc, char *argv[]){

	int socket_desc, client_sock,clilen,read_size;
	int *new_sock;
	struct sockaddr_in server, client;
	
//create socket
	socket_desc = socket(AF_INET, SOCK_STREAM,0);
	if(socket_desc == -1){
		printf("Could not create socket");

	}
	puts("Socket created");

//Prepare sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

//bind the socket
	if(bind(socket_desc,(struct sockaddr *) &server,sizeof(server)) <0){
		perror("Bind failed");
		return 1;
	}
	puts("Server is up and running \n");
//listen
	listen(socket_desc,100);
	puts("Waiting for incomming connections\n");

//Accept Connections
	clilen = sizeof(struct sockaddr_in);

	while(client_sock = accept(socket_desc,(struct sockaddr *) &client,(socklen_t *)&clilen)){
		printf("Connection accepted \n");
		new_sock = malloc(1);	
		*new_sock = client_sock;
		pthread_t pid;
		if(pthread_create(&pid,NULL,create_thread,(void*)new_sock) < 0)	{
			perror("No thread created");
			return 1;
		}
		
	}
	if(client_sock<0){
		perror("Accept failed");
		return 1;
	}
	return 0;
}

void *create_thread(void *socketdesc){
	int sock = *(int *)socketdesc ;
	int read_size;
	char buffer[256],buffer1[256];

//Opening records file
	FILE *fp = fopen("Records.txt","r");
	int record_size = 0;
	char records_line[256];
	char *split;


//Filling up the structure array from records.txt
	while(fgets(records_line,sizeof(records_line),fp)){
		split = strtok (records_line," ,.-");

		while(split!= NULL){
//printf("%s\n",split );
			array_record[record_size].account_number = atoi(split);
			split = strtok (NULL, " ,.-");
// /printf("%s\n",split );	
			strcpy(array_record[record_size].name,split);
			split = strtok (NULL, " ,.-");
//printf("%s\n",split );	
			array_record[record_size].balance = atoi(split);		
			split = strtok (NULL, " ,.-");
		}
		record_size++;
	}

	printf("Num of line of records %d\n",record_size);

//Printing record structure
	for (int b = 0; b < record_size; b++)
	{
		printf("%d \t",array_record[b].account_number);
		printf("%s \t",array_record[b].name);
		printf("%d\n",array_record[b].balance);
	}
//Recieve a message from the client
	
	int size_of_transaction = recv(sock,buffer1,256,0);
	size_of_transaction = write(sock,"Server recievd total number of instruction",256);
	int instruction_size = atoi(buffer1);
	
	/*if(size_of_transaction >0){
		printf("Number of transactions %d\n",instruction_size);

	}*/

	char transasctions[atoi(buffer1)];
	int timestamp,acnt_t, amount_transaction;
	char t_type;
	
	
	int len =0;
	char *t_split;
	char status_old[256],status_new[256];
	printf("Reading transasctions one by one \n");
	
	do{
		len = recv(sock,buffer,256,0)>0;

		len = write(sock,"Processing your transaction one by one \n",256);
		printf("*********************************************\n");
		printf("%s\n",buffer);

		t_split = strtok (buffer," ,.-");
		timestamp = atoi(t_split);
		t_split = strtok (NULL, " ,.-");
		acnt_t = atoi(t_split);
		t_split = strtok (NULL, " ,.-");
		t_type = *t_split;
		t_split = strtok (NULL, " ,.-");
		amount_transaction = atoi(t_split);
		t_split = strtok (NULL, " ,.-");
		
		for (int i = 0; i <record_size; i++)
		{
			//printf("Value of i %d\n",i);
			if(acnt_t == array_record[i].account_number){
				
				//printf("Mathched condition");
				if(t_type == 'd'){
					pthread_mutex_lock(&mutex);
					sprintf(status_old,"Name :%s\nOld balance is: %d",array_record[i].name,array_record[i].balance);
					printf("%s\n", status_old);
					write(sock,status_old,256);
					array_record[i].balance = array_record[i].balance+amount_transaction;
					printf("New balance after depositing %d is : %d\n",amount_transaction,array_record[i].balance);
					printf("*********************************************\n");
					sprintf(status_new,"New balance after depositing amount USD %d is: %d\n",amount_transaction,array_record[i].balance);
					write(sock,status_new,256);
					pthread_mutex_unlock(&mutex);
					break;
				}
				else if(t_type == 'w'){
					if(amount_transaction>array_record[i].balance){
						pthread_mutex_lock(&mutex);
						sprintf(status_old,"Name: %s\nOld balance is: %d",array_record[i].name,array_record[i].balance);
						printf("%s\n", status_old);
						write(sock,status_old,256);
						printf("Transaction declined due to insufficient balance\n");
						printf("*********************************************\n");
						sprintf(status_new,"Transaction declined: Due to insufficient balance. Because Your balance is %d and you are trying to withdraw: %d",array_record[i].balance,amount_transaction);
						write(sock,status_new,256);
						pthread_mutex_unlock(&mutex);
						break;
					}
					else{
						pthread_mutex_lock(&mutex);
						sprintf(status_old,"Name: %s\nOld balance is: %d",array_record[i].name,array_record[i].balance);
						printf("%s\n", status_old);
						write(sock,status_old,256);
						array_record[i].balance = array_record[i].balance - amount_transaction;	
						printf("New balance: %d\n", array_record[i].balance);
						printf("*********************************************\n");
						sprintf(status_new,"New balance after withdrawing amount USD %d is: %d",amount_transaction,array_record[i].balance);
						write(sock,status_new,256);
						pthread_mutex_unlock(&mutex);
						break;

					}
					
				}
				
			}
			else if (acnt_t!= array_record[i].account_number && (i == (record_size-1))){
				printf("Wrong account number, Transaction declined\n");
				write(sock,"Wrong Account number",256);
				write(sock,"Transaction declined",256);
				break;
			}
		

		}
			
		instruction_size--;
	}while(instruction_size>0);

close(sock);
}




