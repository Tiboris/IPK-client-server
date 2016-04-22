#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
/*
*
*/
using namespace std;
/*
*   Used constants and variables
*/
const int REQ_ARGC = 3;
const int MAX_CLIENTS = 9;
/*
* Prototypes of functions
*/
void err_print(const char *msg);
bool args_err(int argc, const char** argv);

void doprocessing (int sock);
/*
*   Main
*/
int main(int argc, char const *argv[])
{   
    if (args_err(argc,argv)) 
    {
        err_print("Wrong parameters, usage: ./server -p [port_number]");
        return EXIT_FAILURE;
    }
    int port = strtoul(argv[2],NULL,0);
    /*
        vars
    */
    int sockfd, newsockfd;

    struct sockaddr_in server_addr, client_addr;
    /* 
        First call socket() function 
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        err_print("ERROR opening socket");
        return EXIT_FAILURE;
    }
    /* 
        Initialize socket structure 
    */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    /*
        Binding socket
    */
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
    {
        err_print("ERROR on binding, make sure port is available");
        return EXIT_FAILURE;
    }
    /* 
        Now start listening for the clients, here
        process will go in sleep mode and will wait
        for the incoming connection
    */
    listen(sockfd,MAX_CLIENTS);
    socklen_t cli_len = sizeof(client_addr);
    /*
        Getting things work
    */
    while(true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
        
        if (newsockfd < 0) 
        {
            perror("ERROR on accept");
            exit(1);
        }
      
        /* Create child process */
        doprocessing(newsockfd);
        close(newsockfd);
        break;
       /* switch(fork())
        {
            case -1:
            {
                perror("ERROR on fork");
                exit(1);
                break;
            }
            case 0:
            {
                // This is the client process 
                close(sockfd);
                //doprocessing(newsockfd);
                exit(0);
            }
            default:
            {
                close(newsockfd);
            }
        }*/
    }
    return EXIT_SUCCESS;
}
/*
*   For printing errors
*/
void err_print(const char *msg)
{
    cerr << msg << endl;    
}
/*
*   For checking argument
*/
bool args_err(int argc, const char** argv)
{
    if ( ( argc != REQ_ARGC ) || (strcmp(argv[1],"-p") != 0)) 
    {
        return EXIT_FAILURE;
    }
    char* garbage = '\0';
    int port = strtoul(argv[2],&garbage,0);
    if ((garbage[0]!='\0') || (port < 1024) || (port > 65535)) 
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void doprocessing (int sock) {
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(sock,buffer,255);
   
    if (n < 0) 
    {
        perror("ERROR reading from socket");
        exit(1);
    }
   
    printf("Here is the message: %s\n",buffer);
    n = write(sock,"I got your message",18);
   
    if (n < 0) 
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}
