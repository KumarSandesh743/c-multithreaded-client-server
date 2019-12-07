/*get_usna.c*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#define Buffer 4096

static volatile int sock = -1;

void * Read_Args(void * args)
{
    int number;
    char readLine[Buffer + 1];

    number = read(sock, readLine, Buffer);
    if ( strcmp(readLine, "clash") == 0)
    {
        printf("A Client already exists with the mentioned name\n");
        exit(1);
    }
    else if ( strcmp(readLine, "accept") != 0 )
    {
        printf("Error in connecting\n");
        exit(1);
    }
    else if ( number < 0 )
    {
        perror("read ");
        exit(1);
    }
    else
    {
        printf("Connection Successful.\n");
        printf("Available commands are /quit /list /msg clientname message\n");
    }


    while( (number = read(sock, readLine, Buffer)) > 0)
    {
        readLine[number] = '\n';
        //write readLine to stdout
        if(write(1, readLine, number + 1) < 0)
        {
            perror("write");
            exit(1);
        }
    }

    //read the readLine until EOF

    if (number<0)
    {
        perror("read");
        exit(1);
    }

}
int Check_Message(char * string)
{
    int lengthOfString = strlen(string);
    if (lengthOfString < 6)
    {
        return 0;
    }
    char array[] = "/msg ";
    char array2[lengthOfString + 1];
    strcpy(array2, string);
    array2[5] = 0;
    if ( (strcmp(array2, array) != 0) )
    {
        return 0;
    }
    strcpy(array2, string);
    array2[lengthOfString] = 0;

    char * token;
    int count = 0;

    token = strtok(array2, " ");
    while ( token != 0)
    {
        token = strtok(NULL, " ");
        count++;
    }

    if(count < 3)
    {
        return 0;
    }

    return count;
}

int main(int argc, char * argv[])
{
    char * hostname = argv[1];
    short port = atoi(argv[2]);
    struct addrinfo *result;       //to store results
    struct addrinfo hints;         //to indicate information we want
    struct sockaddr_in *saddr_in;  //socket interent address

    int check,n;                       //for error checking
    pthread_t p;
    //setup our hints
    memset(&hints,0,sizeof(struct addrinfo));  //zero out hints
    hints.ai_family = AF_INET; //we only want IPv4 addresses

    //Convert the hostname to an address
    if(argc != 4)
    {
        printf("Sorry, Too less number of inputs\n");
        exit(1);
    }

    if( (check = getaddrinfo(hostname, NULL, &hints, &result)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s check\n",gai_strerror(check));
        exit(1);
    }

    //convert generic socket address to inet socket address
    saddr_in = (struct sockaddr_in *) result->ai_addr;

    //set the port in network byte order
    saddr_in->sin_port = htons(port);

    //open a socket
    if( (sock = socket(AF_INET, SOCK_STREAM, 0))  < 0)
    {
        perror("socket");
        exit(1);
    }

    //connect to the server
    if(connect(sock, (struct sockaddr *) saddr_in, sizeof(*saddr_in)) < 0)
    {
        perror("connect");
        exit(1);
    }

    char * arrayCheck;
    size_t s_message;
    int transmit = 0;
    int terminate = 0;
    //while(1){
    //transmit the arrayCheck
    //size_t s_message;

    //int quit = 0;
    //int transmit = 0;

    if (pthread_create(&p, NULL, Read_Args, NULL) < 0 )
    {
        perror("thread ");
        exit(1);
    }


    if(write(sock,argv[3],strlen(argv[3])) < 0)
    {
        perror("transmit");
    }

    while(terminate == 0)
    {
        arrayCheck = 0;
        transmit = 0;
        s_message = getline(&arrayCheck, &s_message, stdin);
        arrayCheck[s_message - 1] = 0;
        if ( strcmp (arrayCheck, "/list") == 0)
        {
            transmit = 1;
            arrayCheck[0] = '1';
            arrayCheck[1] = 0;
            if(write(sock,arrayCheck,strlen(arrayCheck)) < 0)
            {
                perror("transmit");
            }
        }
        else if (strcmp(arrayCheck, "/quit") == 0)
        {
            terminate = 1;
            transmit = 1;
            arrayCheck[0] = '0';
            arrayCheck[1] = 0;
            if(write(sock,arrayCheck,strlen(arrayCheck)) < 0)
            {
                perror("transmit");
            }
        }
        else if ( Check_Message(arrayCheck) == 1)
        {
            transmit = 1;
            arrayCheck[0] = '2';
            int i = 5;
            while( arrayCheck [i] != 0)
            {
                arrayCheck[i - 4] = arrayCheck[i];
                i++;
            }
            arrayCheck[i - 4] = 0;
            //transmit arrayCheck
            if(write(sock,arrayCheck,strlen(arrayCheck)) < 0)
            {
                perror("transmit");
            }
        }
        if ( transmit != 1)
        {
            printf("Invalid command \n");
        }
        
        free(arrayCheck);
    }
    //close the socket
    close(sock);

    return 0; //success
}

