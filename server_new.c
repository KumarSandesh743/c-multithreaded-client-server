/*hello_server.c*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#define BUF_SIZE 4096

pthread_mutex_t lock_of_list;

typedef struct __node_t
{
  char * name;//name[1024];
  //struct sockaddr_in client_addr_in;
  int client_fd; 
  struct __node_t * next;

} client_node;

//static volatile client_node * head = 0;
client_node * head = NULL;

int Find_Client(char * n){

  printf("Entering  find\n");

  client_node * temp = head;

  printf("Find: declare temp\n");

  //pthread_mutex_lock(&lock_of_list);
  if (temp->next == NULL){ //pehla node hai
    printf("Loop 1\n");
    if ( strcmp(temp->name,n) == 0){
      printf("Found it!\n");      
      return temp->client_fd; //client found
    }
    printf("Compare?\n");
    return 0;
  }


  while(temp->next != NULL){
    printf("n: %s, temp_name: %s \n",n,temp->name);

    if (strcmp(temp->name,n) == 0){
      printf("Found it!\n");      
      return temp->client_fd; //client found

    }
    temp=temp->next;
  }
  return -1; //client not found
  //pthread_mutex_unlock(&lock_of_list);
}

int Find_Client_name(int fd){

  //pthread_mutex_lock(&lock_of_list);


  client_node * temp = head;

  if (temp->next == NULL){ //pehla node hai
    printf("Loop 2\n");
    if ( temp->client_fd == fd){
      printf("Found it!\n");      
      return 0; //client found
    }
    printf("Compare?\n");
    return -1;
  }


  while(temp->next != NULL){
    if ( temp->client_fd == fd){
      printf("Found it!\n");
      pthread_mutex_unlock(&lock_of_list);
      return 1; //client found

    }
    temp=temp->next;
  }
  return -1; //client not found
  //pthread_mutex_unlock(&lock_of_list);
}



void Add_Client(char * n,int fd){

  
  

  client_node * temp = head;

  while(temp->next != NULL){
    temp=temp->next;
  }
  int ishead = 0;
  if (temp->next == NULL){
    //yani pehla node
    ishead = 1;
  }
  
  client_node * new_client = malloc(sizeof(client_node)); //dynamically allocate memory for node
  temp->next = new_client;
  new_client->next = NULL;
  //new_client->client_addr_in = add;
  char * temp_name = malloc(strlen(n)+1);
  strcpy(temp_name,n);
  temp_name[strlen(n)] = '\0';
  new_client->name = temp_name;
  new_client->client_fd = fd;
  if(ishead == 1){
    head = new_client;

  }
  printf("client name: %s\n and f.d: %d\n\n",new_client->name,new_client->client_fd);

  pthread_mutex_unlock(&lock_of_list);

}

void Delete_Client(int fd){
  

  client_node * temp = head;
  int isFind = Find_Client_name(fd);
  pthread_mutex_lock(&lock_of_list);

  if(isFind==-1){
    printf("error: no client of this name exists!\n");
    exit(1);
  }

  client_node * temp_prev = head;

  while(temp->next != NULL){
    if(isFind>0){
      break;
    }

    temp=temp->next;
  }


  while(temp_prev->next != temp){   

    temp_prev=temp_prev->next;
  }

  temp_prev->next = temp->next;
  
  
  close(temp->client_fd);
  free(temp->name);
  free(temp);

  pthread_mutex_unlock(&lock_of_list);

}

void Show_Clients(int fd){
  //pthread_mutex_lock(&lock_of_list);
  printf("Enter show clients!\n");

  client_node * temp = head;
 
  if(temp->next == NULL){
    printf("List: %s\n",temp->name);
    write(fd,temp->name,strlen(temp->name)+1);
  }

  while(temp->next != NULL){
    write(fd,temp->name,strlen(temp->name)+1);//printf("%d.    %s\n",count,temp->name);
    temp=temp->next;
  }
  printf("Leave show clients\n");
  //pthread_mutex_unlock(&lock_of_list);
}

void * Read_Client(void * client_s)
{

  printf("entering read client!\n");
  int client_sock = (int )client_s;
  char response[BUF_SIZE];           //what to send to the client
  int n;        
  response[0]=0;
  n=read(client_sock,response,BUF_SIZE-1);                     //length measure


  if (n < 0)
  {
    perror("read ");
    close(client_sock);
    pthread_exit(1);
  }  
  char name[strlen(response) + 1];
  strcpy(name, response);
  name[strlen(response)] = 0;
 

  printf("name of new client: %s\n",name);
  int x = Find_Client_name(client_sock);
  printf("allocated x\n");

  if ( x > -1)
  {
    printf("Client name already exists!\n");
    strcpy(response, "error");
    if ( write(client_sock, response, strlen(response)) < 0 )
    {
      perror("write ");
    }
    close(client_sock);
    pthread_exit(1);
  }
  else
  {
    printf("Client name new hai!\n");
    strcpy(response, "accept");
  }

  printf("Writng response on client socket\n");

  if ( write(client_sock, response, strlen(response)) < 0 )
  {
    perror("write ");
    close(client_sock);
    pthread_exit(1);
  }

  printf("Written response on client socket\n");

  Add_Client(response,client_sock);
  printf("client added\n");

  int send;

  while(1){

    send = 0;

    if((n = read(client_sock,response, BUF_SIZE-1)) < 0){
      perror("read");
      Delete_Client(client_sock);
      close(client_sock);
      pthread_exit(1);
    }

    if ( response[0] == '0') //quit
    {
      Delete_Client(client_sock);
      close(client_sock);
      pthread_exit(1);
    }
    else if ( response[0] == '1') //list
    {
      //populateClient(response);
      Show_Clients(client_sock);
      //send = 1;
    }
    else if ( response[0] == '2') //msg
    {
      response[n] = '\0';
      char reciever_name[BUF_SIZE];
      int i =1;
      int ln = 1;
      while(reciever_name[i] != " "){
        reciever_name[i] = response[i];
        ln++; 
      }

      int r = 0;//messageClient(response, name, client_sock);
      if ( r == 1 )
      {
        char back_rsp[] = "Client name not found ";
        strcpy(response, back_rsp);
       response[strlen(back_rsp)] = 0;
        send = 1;
      }
    }

    //send response
    if ( send  == 1)
    {
      if(write(client_sock, response, strlen(response)) < 0){
        Delete_Client(client_sock);
        close(client_sock);
        pthread_exit(1);
      }     
    }
  }
}






int main(int argc,char * argv[]){


  if (argc != 2){
    printf("error: You have not specified a port to listen on!\n");
    exit(1);
  }
  head = malloc(sizeof(client_node));
  head->next = NULL;
  head->name = '\0';
  head->client_fd = -1;

  
  char hostname[]="127.0.0.1";//"192.168.58.130";   //localhost ip address to bind to
  short port=atoi(argv[1]);//5330;               //the port we are to bind to


  struct sockaddr_in saddr_in;  //socket interent address of server
  struct sockaddr_in client_saddr_in;  //socket interent address of client

  socklen_t saddr_len = sizeof(struct sockaddr_in); //length of address
  
  int isQuit = 0;
  int server_sock, client_sock;         //socket file descriptor


  char response[BUF_SIZE];           //what to send to the client
  
  char quit[6] = "/quit";
  quit[5] = '\0';
  int n;                             //length measure

  //set up the address information
  saddr_in.sin_family = AF_INET;
  inet_aton(hostname, &saddr_in.sin_addr);
  saddr_in.sin_port = htons(port);

  //open a socket
  if( (server_sock = socket(AF_INET, SOCK_STREAM, 0))  < 0){
    perror("socket");
    exit(1);
  }
 
  //bind the socket
  if(bind(server_sock, (struct sockaddr *) &saddr_in, saddr_len) < 0){
    perror("bind");
    exit(1);
  }
   
  //ready to listen, queue up to 5 pending connectinos
  if(listen(server_sock, 5)  < 0){
    perror("listen");
    exit(1);
  }


  saddr_len = sizeof(struct sockaddr_in); //length of address

  printf("Listening On: %s:%d\n", inet_ntoa(saddr_in.sin_addr), ntohs(saddr_in.sin_port));

  //accept incoming connections
  pthread_t p;
  while (1 == 1)
  {
    if((client_sock = accept(server_sock, (struct sockaddr *) &client_saddr_in, &saddr_len)) < 0){
      perror("accept");
      exit(1);
    }

    if ( pthread_create(&p, NULL, Read_Client, (void *)client_sock) < 0 )
    {
      perror("thread ");
    }
  }

  printf("Closing socket\n\n");


  //close client_sock
  close(client_sock);

  //close the socket
  close(server_sock);

  return 0; //success
}
