/*CMSC 621 Advanced Operating systems 
Project 2
Ajinkya Wakhale- AN75014 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
//#include <ctime>
#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"

int main(int argc, char *argv[])
{
  int socket_sequencer, socket_multicast;
  struct sockaddr_in sequencer_st,multicast_st;
  struct ip_mreq mreq;
  //srand(time(NULL));
  //struct ip_mreq mreq;
   u_int yes = 1;

 //create sequencer socket
  socket_sequencer= socket(AF_INET, SOCK_DGRAM,0);   
  if(socket_sequencer<0){
    perror("Error in creating socket");
  }
//printf("Debug 16.0");
  sequencer_st.sin_family = AF_INET;
  sequencer_st.sin_addr.s_addr = INADDR_ANY;
  sequencer_st.sin_port = htons(3210);
//printf("Debug 16.0");
//bind sequencer socket
int b = bind(socket_sequencer,(struct sockaddr *)&sequencer_st,sizeof(sequencer_st));
if(b<0){
perror("Error in binding");

}  

//create multicast socket
  socket_multicast= socket(AF_INET, SOCK_DGRAM,0);   
  if(socket_sequencer<0){
    perror("Error in creating socket");
  }

//printf("Debug 16.1");
//Check reusability
  if (setsockopt(socket_multicast,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
   perror("Reusing ADDR failed");
   exit(1);
 }

 //bind sequener multicast socket
int b1 = bind(socket_multicast,(struct sockaddr *)&multicast_st,sizeof(multicast_st));

  memset(&multicast_st,0,sizeof(multicast_st));
  multicast_st.sin_family=AF_INET;
  multicast_st.sin_addr.s_addr=inet_addr(HELLO_GROUP);
  multicast_st.sin_port=htons(HELLO_PORT);


//to request setsockopt() to request that kernel join multicast group
mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
mreq.imr_interface.s_addr=htonl(INADDR_ANY);
if (setsockopt(socket_multicast,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
  perror("setsockopt");
  exit(1);
}

  
//printf("Debug 16.0\n");
  //sleep(36);
  char buffer1[256];
  socklen_t addrlen1 = sizeof(sequencer_st);
  int global_sequencer = 0;
  int seq_counter = 0;
  while(recvfrom(socket_sequencer,buffer1,sizeof(buffer1),0,
    (struct sockaddr *) &sequencer_st,&addrlen1)>=0) {
      //sleep(2);
    seq_counter++;
    //printf("M %s: \n",buffer1); 
    global_sequencer++;
    char sequencer_buffer[256];
    char comma [] = ",";
    sprintf(sequencer_buffer,"%d",global_sequencer);
    strcat(buffer1,comma);
    strcat(buffer1,sequencer_buffer);
    printf("[%d]: Message: %s\n",seq_counter,buffer1);
    //sleep(20);
  int s = sendto(socket_multicast,buffer1,sizeof(buffer1),0,(struct sockaddr *) &multicast_st,
    sizeof(multicast_st));
  //printf("Message Broadcasted from sequencer\n");
     bzero(buffer1,256);
      //sleep(15);
        //sleep(t);
        //i-- ;
}



  return 0;
}