/*Front end server which will accept connections from client
and will do all the necessary steps for transactions to backend servers */

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
#define server_main_1 7868
#define server_main_2 7869
#define server_main_3 7870

void *create_thread(void *);

pthread_mutex_t mutex;

int main (int argc, char *argv[]){

	int socket_desc, client_sock,clilen,read_size;
	int *new_sock;
	struct sockaddr_in server_front, client,server_main;
	
//create front end socket
	socket_desc = socket(AF_INET, SOCK_STREAM,0);
	if(socket_desc == -1){
		printf("Could not create socket");

	}
	puts("Front end Socket created");

//Prepare sockaddr_in structure, get port from command line argument
	server_front.sin_family = AF_INET;
	server_front.sin_addr.s_addr = INADDR_ANY;
	server_front.sin_port = htons(atoi(argv[2]));

//bind the front end  socket
	if(bind(socket_desc,(struct sockaddr *) &server_front,sizeof(server_front)) <0){
		perror("Bind failed");
		return 1;
	}
	printf("Server is up and running\n");
	
//listen
	listen(socket_desc,100);

//Accept Connections
	clilen = sizeof(struct sockaddr_in);

	//For each client, create a new thread and assigning a thread handle for each client
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
	int client_sock1 = *(int *)socketdesc;
	int front_as_client[3];
	int array_port[3];	
	array_port[0] = htons(server_main_1);
	array_port[1] = htons(server_main_2);
	array_port[2] = htons(server_main_3);
	int i=0;
	int total_servers=0;
	int servers_connected[3];

	for ( i = 0; i < 3; i++)
	{
		struct hostent *server;
		struct sockaddr_in server_addr;
		server = gethostbyname("localhost");
		server_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
		server_addr.sin_port = ((array_port[i]));	

			//array of sockets to connect to multiple servers one by one 

		front_as_client[i] = socket(AF_INET, SOCK_STREAM,0);
		if(front_as_client[i]<0){
			printf("Could not create socket");

		}	
		
		
		
			//Connect to servers one by one 
		if(connect(front_as_client[i], (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		{
			//fprintf(stderr, "Error connecting\n");
			servers_connected[i] = 0;
		}
		else{
			servers_connected[i] = 1;
			total_servers++;
		}
	}

	
	while(1){
		//printf("Total servers are %d\n",total_servers );
		char transaction[256];
		char updated_trans_status[256];
		//char vote[256];
		char global_abort_status[256];
		//Read from client 
		int r = read(client_sock1,transaction,sizeof(transaction));
		//pthread_mutex_lock(&mutex);	
		int char_count = 0;
		while(transaction[char_count]){
			transaction[char_count] = toupper(transaction[char_count]);
			char_count++;
		}

		printf("transaction is %s\n",transaction );
		//int cmpi= strcmp(transaction,"QUIT");
		
		//printf("Valuye of q1 %d\n",cmpi);
		if(strcmp(transaction,"QUIT")==10){
			int s3 = send(client_sock1,"OK",sizeof(2),0);	
			//printf("In quit\n" );
			close(client_sock1);
			break;
		}
		else{

			int vote_count = 0;
			//printf("Before voting \n");
			sleep(5);

		//Send vote
			int k1=0;	
			char test1[256];

			for(k1=0;k1<3;k1++){
				if(servers_connected[k1]==1){
					int n= send(front_as_client[k1],"VOTE",256,0);
				}

			}
			int z = 0;
			char recv_vote[256];
			bzero(recv_vote,256);
		//Recieve Vote
			struct timeval timeout;
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
			servers_connected[0] = 0;
			servers_connected[1] = 0;
			servers_connected[2] = 0;
			
			for(int h = 0; h<3;h++){
				if(setsockopt(front_as_client[h],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
				{
					printf("\nTimeout error");
				//exit(1);
				}
				while(recv(front_as_client[h],recv_vote,256,0)>0)
				{
					vote_count++;
					servers_connected[h] = 1;
				}
			}
			printf("Vote count is %d \n",vote_count);
			printf("Total servers after vote count is %d\n", total_servers);
			
			sleep(3);

		//All the servers are available	
			int j=0;
			if(vote_count==0){
				sprintf(updated_trans_status,"ERR: Try again later");
				int s3 = send(client_sock1,updated_trans_status,sizeof(updated_trans_status),0);
			}
			else{
				if(vote_count==total_servers){
					char test[256];

					total_servers = 0;
					for ( j = 0; j < 3; j++)
					{
						if(recv(front_as_client[j],test,256,0)!=0)
						{
							if(servers_connected[j]==1){
								int s3= send(front_as_client[j],"GLOBAL COMMIT",256,0);

								total_servers++;
							}
						}
						else{
							servers_connected[j] = 0;
						}

					}	
					int x=0;
					total_servers=0;
					char test3[256];
					for(x=0;x<3;x++){
						
						
						if(recv(front_as_client[x],test3,256,0)!=0)
						{
							if(servers_connected[x]==1){
								int n2 = send(front_as_client[x],transaction,sizeof(transaction),0);
								total_servers++;
							}
						}
						else{
							servers_connected[x] = 0;
						}
					}
					int y = 0;
					total_servers = 0;
					printf("Sleep\n");
					sleep(5);
			//Recieve updated transaction
					char test4[256];
					bzero(updated_trans_status,256);
					for(y=0;y<3;y++){
						

						if(recv(front_as_client[y],updated_trans_status,256,0)!=0)
						{
							if(servers_connected[y]==1){
								
								total_servers++;
							}
						}
						else{
							servers_connected[x] = 0;
						}
						

					}

					int s3 = send(client_sock1,updated_trans_status,sizeof(updated_trans_status),0);	


				}
				else{
					int sock_closed;
					int a= 0;
					total_servers = 0;
					for(a=0;a<3;a++){
						if(setsockopt(front_as_client[j],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
						{
							//printf("\nTimeout error");

						}
						if(servers_connected[a]==1){
							send(front_as_client[a],"GLOBAL ABORT",256,0);		
							total_servers++;	
						}
					}
					for(a=0;a<3;a++){
						if(setsockopt(front_as_client[j],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
						{
							//printf("\nTimeout error");
				//exit(1);
						}
						if(servers_connected[a]==1){
							int n2 = read(front_as_client[a],updated_trans_status,sizeof(updated_trans_status));
						}
					}

					int s3 = send(client_sock1,updated_trans_status,sizeof(updated_trans_status),0);
				}
			}	

			bzero(transaction,256);
			bzero(global_abort_status,256);
			bzero(recv_vote,6);
			

		}	
	}	
	close(client_sock1);

}

//}
