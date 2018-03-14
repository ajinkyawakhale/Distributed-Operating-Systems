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

//Handler definitions
void *sender_handler(void *);
void *listener_handler(void *);
int socket_process_multicast, socket_process_point;
struct sockaddr_in multicast_st,multicast_st_d,point_sock,st_sockname;
int rank;
char * mesg_bcast1;
char mesg_bcast[256];

struct message_st{
  char chck_mesg[256];
  int flag; 
};
struct message_st message_array[10000];

struct message_seq_st{
  char chck_mesg_seq[256];
  int global_sequence; 
};
struct message_seq_st message_array_sequencer[10000];

int main(int argc, char *argv[])
{
  pthread_mutex_t mutex;
  FILE *fp = fopen("counter.txt","r+");
  char buffer_lock[256];
  fgets(buffer_lock,sizeof(buffer_lock),fp);
  int count_lock = atoi(buffer_lock);
  //printf("Cu counter  is %d\n",count_lock);
  count_lock++;
  printf("Updated value before writing %d\n",count_lock);
  //fprintf(fp, "%d\n",count_lock );
  fclose(fp);
  FILE *fp1 = fopen("counter.txt","w+");
  fprintf(fp1, "%d\n",count_lock );
  printf("Value after writing is %d\n",count_lock);
  fclose(fp1);
  pthread_mutex_unlock(&mutex);
  
  srand(time(NULL));
  struct ip_mreq mreq;
  char msgbuf[256],msgbuf1[256];
  char message[256];
  u_int yes=1; 
  int logical_counter;
  sleep(5);    
  int master_port_slave;
  int master_clock_slave;
  pthread_t sender_t, listener_t;

  //assigning message to be sent
  mesg_bcast1 = argv[2];
   //checking which is rank
  rank = atoi(argv[1]);
  srand(time(NULL));
  logical_counter = rand()%100;


 //create multicast socket
  socket_process_multicast= socket(AF_INET, SOCK_DGRAM,0);   
  if(socket_process_multicast<0){
    perror("Error in creating socket");
  }
    //create point_point socket socket
  socket_process_point = socket(AF_INET, SOCK_DGRAM,0);   
  if(socket_process_point<0){
    perror("Error in creating socket");
  }


 //Setup destination address on Multicast
  memset(&multicast_st,0,sizeof(multicast_st));
  multicast_st.sin_family = AF_INET;
  multicast_st.sin_addr.s_addr = inet_addr(HELLO_GROUP);
  multicast_st.sin_port = htons(HELLO_PORT);

 // Fill Recieving structure   
  memset(&multicast_st_d,0,sizeof(multicast_st_d));
    //printf("Debug 0.0 \n");
  multicast_st_d.sin_family = AF_INET;
  multicast_st_d.sin_addr.s_addr = htonl(INADDR_ANY);
    //printf("Debug 0.1 \n");
  multicast_st_d.sin_port = htons(HELLO_PORT);

//Check reusability
  if (setsockopt(socket_process_multicast,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
   perror("Reusing ADDR failed");
   exit(1);
 }

    //Bind multicast socket
 int b = bind(socket_process_multicast,(struct sockaddr *)&multicast_st_d,sizeof(multicast_st_d));
   //printf("Debug 0.6 \n");
 if(b<0){
  perror("Error in binding multicast socket");
}

//to request setsockopt() to request that kernel join multicast group
mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
mreq.imr_interface.s_addr=htonl(INADDR_ANY);
if (setsockopt(socket_process_multicast,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
  perror("setsockopt");
  exit(1);
}

//Bind point to point socket
point_sock.sin_family = AF_INET;
point_sock.sin_addr.s_addr = INADDR_ANY;
point_sock.sin_port = htons(INADDR_ANY);

//Check reusability
if (setsockopt(socket_process_point,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
 perror("Reusing ADDR of socket point  failed");
 exit(1);
}


//Bind point to point socket
//printf("Debug 0.7 \n");
int b1 = bind(socket_process_point,(struct sockaddr *)&point_sock,sizeof(point_sock));
//printf("Debug 0.8 \n");
if(b1<0){
  perror("Error in binding Point to point socket");
}

sleep(5);

int sockname,logical_counter_master;
socklen_t sa_len;
int num_process = 0;
char st_mesg_send[256];
socklen_t addrlen=sizeof(multicast_st_d);
int nbytes;

//To calculate number of processess and sending master clock and master port
if(rank==0){
    //logical_counter_master = 50;
  printf("Local clock of Master is %d\n", logical_counter);
  sa_len = sizeof(st_sockname);
  sockname = getsockname(socket_process_point,(struct sockaddr *)&st_sockname,&sa_len);
    //printf("Local port is %d\n",(int)ntohs(st_sockname.sin_port));
  char master_counter[256];
  sprintf(master_counter,"%d",logical_counter);

    //Send master clock to all processes
  int s = sendto(socket_process_multicast,master_counter,sizeof(master_counter),0,(struct sockaddr *) &multicast_st,
    sizeof(multicast_st));
  int master_port_int = (int)ntohs(st_sockname.sin_port);
  char master_port[256];
  sprintf(master_port,"%d",master_port_int);

//Sending master port number to all other process 
  s = sendto(socket_process_multicast,master_port,sizeof(master_port),0,(struct sockaddr *) &multicast_st,
    sizeof(multicast_st));
  char message4[256];

}

//Recieving Master clock value 
if ((nbytes=recvfrom(socket_process_multicast,msgbuf,256,0,
 (struct sockaddr *) &multicast_st_d,&addrlen))) {
  if(rank!=0){
    printf("Local clock value for process %d is  %d\n",rank,logical_counter);
    printf("Logical counter of master is %s\n",msgbuf );
  }
}

//Recieving master port number
if ((nbytes=recvfrom(socket_process_multicast,msgbuf1,256,0,
 (struct sockaddr *) &multicast_st_d,&addrlen))) {
 // printf("Master port number is %s\n", msgbuf1);
}

//Send drift to master process
int local_drift;
if(rank!=0){
 master_clock_slave = atoi(msgbuf);
//Assign port number to master
 point_sock.sin_port = htons(atoi(msgbuf1));
   //printf("master port %s\n", msgbuf1);
 char message3[256];
 local_drift = logical_counter-master_clock_slave;
 sprintf(message3,"%d",local_drift);
 int s1 = sendto(socket_process_point,message3,sizeof(message3),0,(struct sockaddr *) &point_sock,
  sizeof(point_sock));
}
char message4[256],message5[256];
int s2;
struct timeval timeout;
timeout.tv_sec = 5;
timeout.tv_usec = 0;
if(setsockopt(socket_process_point,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))<0){
  perror("timeout eror");  
}

int j, average=0,sum=0;
if(rank==0){
   // for( j = 0; j<2; j++)
    //{
  while(recv(socket_process_point,message4,256, 0)>=0){
    //printf("Drift recieved is %s\n",message4 );
        //printf("Debug 16.0 \n");    
    sum = (sum +atoi(message4));
    //printf("Value of num_process %d\n",num_process );
    num_process++;
  }
    //}
  average = sum/(num_process+1);
    //logical_counter= logical_counter+average;
    //printf("Average is %d\n",average );
  printf("Total number of process is %d\n",num_process+1 );
    //sending average to Multicast
  sprintf(message5,"%d",average);
  s2 = sendto(socket_process_multicast,message5,sizeof(message5),0,(struct sockaddr *) &multicast_st,
    sizeof(multicast_st));
  //printf("Total number of process is %d\n",num_process );
}

char msgbuf2[256];
int offset,drift;
if ((nbytes=recvfrom(socket_process_multicast,msgbuf2,256,0,
 (struct sockaddr *) &multicast_st_d,&addrlen))) {
    //printf("Master clock at end %d\n",atoi(msgbuf) );
  drift = logical_counter-atoi(msgbuf);
offset = drift-atoi(msgbuf2);
//printf("Offset for process %d is %d\n",rank,offset );
logical_counter = logical_counter-offset;  
printf("Clocks synchronized for all the processes \n");
printf("Logical clock for process rank %d is %d\n",rank,logical_counter );
}
//End of Clock synchronized

//Create threads for sender and reciever
if(pthread_create(&sender_t,NULL,sender_handler,(void *)NULL)<0){
  perror("Error in creating sender thread");  
  return 1;
}
if(pthread_create(&listener_t,NULL,listener_handler,(void *)NULL)<0){
  perror("Error in creating Reciever  thread");  
  return 1;
}
pthread_join(sender_t,NULL);

pthread_join(listener_t,NULL);
return 0;
}


//Handler 
void *sender_handler(void * me) {
  int s5=0;
  int t = 10;
  for(int i =0 ; i<3; i++){
   sprintf(mesg_bcast,"%s%d",mesg_bcast1,i);
   //sleep(rand()% t);
   //sleep(rand()%t);
   s5 = sendto(socket_process_multicast,mesg_bcast,sizeof(mesg_bcast),0,(struct sockaddr *) &multicast_st,
    sizeof(multicast_st));
   //sleep(rand()%i);
   point_sock.sin_port = htons(3210);
   s5 = sendto(socket_process_point,mesg_bcast,sizeof(mesg_bcast),0,(struct sockaddr *) &point_sock,
    sizeof(point_sock)); 
    //sleep(20);
   
   //sleep(rand()%t);

 }
  //sleep(3);

 

   //sleep(38);
  //}
//}
}
void *listener_handler(void *mesg){
  char msgbuf3[256];
  //printf("In reciever thread\n"); 
  socklen_t  addrlen1=sizeof(multicast_st_d);
  int nbytes1;
  int recv_seq = 0;
  int count = 0;
  int local_seq = 0;
  int message_array_sequencer_counter;
  int messsage_Counter = 0;
  //sleep(rand()%50);
  //Recieve message from Broadcast and sequencer
  while(recvfrom(socket_process_multicast,msgbuf3,sizeof(msgbuf3),0,
    (struct sockaddr *) &multicast_st_d,&addrlen1)>=0) {
    int q = 0, seq_mesg = 0;
  count ++;//Total number of messages recieved
  char *split;

  printf("Before total ordering [%d]. message %s \n",count,msgbuf3); 
  printf("\n"); 
  //printf("Total number of messages recieved is %d\n",count);
  //printf("Debug 1.0\n");

  //Checking whether the message recieved is from sequencer or it is a Multicast
  while(msgbuf3[q]!= '\0'){
   // printf("Debug 2.0\n");
    if(msgbuf3[q]==','){
     // printf("Debug 3.0\n");
      seq_mesg=1 ;
      split = strtok(msgbuf3," ,.-");
      while(split!=NULL){
       // printf("Debug 4.0\n");
        strcpy(msgbuf3,split);
        //printf("Recievded split message %s\n",msgbuf3);
        split = strtok (NULL, " ,.-");
        recv_seq = atoi(split);
        split = strtok (NULL, " ,.-"); 
      }
    } 
    q++;
  }
  //printf("Seq message is %d\n", seq_mesg);
 //If it is from multicast group- Putting it in Structure- local buffer which stores all messages.  
  if(seq_mesg ==0){
    strcpy(message_array[count-1].chck_mesg,msgbuf3);
    message_array[count-1].flag = 0;
    for (int i = 0; i < message_array_sequencer_counter; i++)
    {
     // printf("value of i is %d\n",i);
     // printf("Message in sequence array is %s\n",message_array_sequencer[i].chck_mesg_seq );
      //printf("Value of its sequence is %d\n", message_array_sequencer[i].global_sequence);
      //printf("Message in global array is %s\n", message_array[i].chck_mesg);
      int b1 = strcmp(message_array_sequencer[i].chck_mesg_seq,msgbuf3);
     // printf("Value of b1 is %d\n", b1);
      if(b1==0){
        if(message_array_sequencer[i].global_sequence == local_seq+1){
          //printf("*******************Message sent**********************************\n");
          messsage_Counter++;
          printf("[%d].Message %s sent to application\n",messsage_Counter,message_array_sequencer[i].chck_mesg_seq);
          local_seq++;
          message_array[count-1].flag =1;
          break;
        }
      }
    }
  }
  if(seq_mesg==1){
    strcpy(message_array_sequencer[message_array_sequencer_counter].chck_mesg_seq,msgbuf3);
    message_array_sequencer[message_array_sequencer_counter].global_sequence = recv_seq;
    message_array_sequencer_counter++;
    for(int i = 0; i<count; i++){
      int b= strcmp(msgbuf3,message_array[i].chck_mesg);
      if(b==0){
       if(message_array[i].flag==0 && ((local_seq+1)==recv_seq)) {
        //printf("*******************Message sent**********************************\n");
        messsage_Counter++;
        printf("[%d].Message %s sent to application\n",messsage_Counter,msgbuf3);
        //printf("*****************************************************\n");
        printf("\n");
        message_array[i].flag = 1;
        local_seq++;
        break;
      }
    } 

  }
}

bzero(msgbuf3,256);
      //sleep(15);
        //sleep(t);
        //i-- ;
}


}