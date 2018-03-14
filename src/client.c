#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]){

// Creating client socket
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

//create socket
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	server = gethostbyname(argv[1]); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);

//Connecting to server
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0){
		perror("Connect failed");	
		return 1;
	}
	puts("Connected");
	// Opening Transaction file	
	FILE *fp = fopen("Transactions.txt","r");
	FILE *fp1 = fopen("Transactions.txt","r");
	char ch;
	int num_line = 1;
	char line_transaction[256],buffer[256],buffer1[256],buffer2[256],buffer4[256];
	if(fp==NULL){
		printf("Error in opening file");
		return 1;
	}

	while(!feof(fp1)){
		ch = fgetc(fp1);
		if(ch=='\n'){
			num_line++;	
		}
	}
	printf("Number of Transactions : %d\n",num_line);

	//To send number of instruction via send, converting to char array
	char num_line_char[1];


	sprintf(num_line_char,"%d",num_line);
	//printf("%s\n",num_line_char);
	int q=0, n=0;
	float time = atof(argv[3]);
	printf("Wait time for each transaction is %f\n",time );
//Sending data to server

	int p = write(sockfd,num_line_char,strlen(num_line_char));
	if(p<0){
		perror("Error writing number of transaction");
		return 1;
	}
	p = read(sockfd,buffer2,256);
	//printf("%s\n",buffer2 );
	int timer =0,counter = 0;
	char * split;
	char buffer3[256];
	printf("*****************************************************************************************\n");
	while(fgets(line_transaction,sizeof(line_transaction),fp)!=NULL){
		strcpy(buffer3,line_transaction);
		split = strtok (buffer3," ,.-");
		timer = atoi(split);
		while(timer > counter){
			counter++;
			usleep(1000000*time);
		}
		//sending each transaction line
		n = write(sockfd,line_transaction,strlen(line_transaction));
		//Recieveing acknowledgement of recieving each transaction
		q = read(sockfd,buffer,256);
		if(q<0){
		perror("acknowledgement of transaction line recieved by server is not recieved on client");	
		return 1;
		}
		//Printing transaction one by one line
		printf("%s\n",buffer);
		//Printing each transaction
		printf("%s\n", line_transaction);
		//Recieving old Status
		n = read(sockfd,buffer1,256);
		if(n<0){
			perror("Old status before transaction is not recieved to client");
			return 1;
		}
		//Recieving new status
		n = read(sockfd,buffer4,256);
		if(n<0){
			perror("New status acknowledgement is not recieved on server");
			return 1;
		}
		//Printing old status
		printf("%s\n", buffer1);
		//Printing new status
		printf("%s\n",buffer4);
		printf("*****************************************************************************************\n");	
		
	}
	printf("All Transactions completed. Closing the connection\n");
	close(sockfd);
	return 0;

}