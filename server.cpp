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
const int RECV_BUFF = 256;
const int MAX_BUFF_SIZE = 1024;
/*
* Prototypes of functions
*/
void err_print(const char *msg);
bool args_err(int argc, const char** argv);
int create_socket(int port);
bool begin_listen(int socket, bool interrupt);
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

    int socket;
    if ((socket = create_socket(strtoul(argv[2],NULL,0))) == -1)
    {
        return EXIT_FAILURE;
    }

    bool interrupt=false; //TODO

    return (begin_listen(socket, interrupt)) ? EXIT_FAILURE : EXIT_SUCCESS;
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

void handle_request(int cli_socket)
{
    int code;
    char buffer[RECV_BUFF];
    bzero(buffer,RECV_BUFF);
    code = read(cli_socket,buffer,RECV_BUFF);
   
    if (code < 0) 
    {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }
   
    printf("Here is the message: %s\n",buffer);
    code = write(cli_socket,"I got your message",18);
   
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    close(cli_socket);

}

void send_data(int socket)
{
    cout<<socket;
}

void recv_data(int socket)
{
    cout<<socket;   
}

int create_socket(int port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    /* 
        First call socket() function 
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return -1;
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
        return -1;
    }
    
    return sockfd;
}

bool begin_listen(int socket, bool interrupt)
{
    struct sockaddr_in client_addr;
    /* 
        Now start listening for the clients, here
        process will go in sleep mode and will wait
        for the incoming connection
    */
    listen(socket,MAX_CLIENTS);
    socklen_t cli_len = sizeof(client_addr);
    while(true)
    {
        if (interrupt)
        {
            break;
        }

        int cli_socket = accept(socket, (struct sockaddr *) &client_addr, &cli_len);
        
        if (cli_socket < 0) 
        {
            perror("ERROR on accept");
            return EXIT_FAILURE;
        }

        handle_request(cli_socket);
        //thread first (foo);     // spawn new thread that calls foo()
        //std::thread second (bar,0);  // spawn new thread that calls bar(0)
    }
    close(socket);

    return EXIT_SUCCESS;
}
