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
#include <thread>         // std::thread
/*
*
*/
using namespace std;
/*
*   Used constants and variables
*/
const int REQ_ARGC = 3;
const int MAX_CLIENTS = 9;
const int MAX_BUFF_SIZE = 1024;
/*
* Prototypes of functions
*/
void err_print(const char *msg);
bool args_err(int argc, const char** argv);

void handle_request(int sock);
void send_data(int socket);
void recv_data(int socket);
/*
*   Main
*/
int main(int argc, char const *argv[])
{   
    if (args_err(argc,argv)) 
    {
        perror("Wrong parameters, usage: ./server -p [port_number]");
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
        perror("ERROR opening socket");
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
        perror("ERROR on binding, make sure port is available");
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

        cout << client_addr.sin_addr.s_addr<< endl;

        //thread 
        /* Create child process */
        handle_request(newsockfd);
        close(newsockfd);
        
        /* 
        thread first (foo);     // spawn new thread that calls foo()
        std::thread second (bar,0);  // spawn new thread that calls bar(0)

        std::cout << "main, foo and bar now execute concurrently...\n";

        // synchronize threads:
        first.join();                // pauses until first finishes
        second.join();               // pauses until second finishes

        std::cout << "foo and bar completed.\n";

        return 0;
        */
    }
    close(sockfd);
    return EXIT_SUCCESS;
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

void handle_request(int sock)
{
    int code;
    char buffer[MAX_BUFF_SIZE];
    bzero(buffer,MAX_BUFF_SIZE);
    n = read(sock,buffer,MAX_BUFF_SIZE);
   
    if (code < 0) 
    {
        perror("ERROR reading from socket");
        exit(1);
    }
   
    printf("Here is the message: %s\n",buffer);
    code = write(sock,"I got your message",18);
   
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void send_data(int socket)
{
    cout<<socket;
}

void recv_data(int socket)
{
    cout<<socket;   
}

