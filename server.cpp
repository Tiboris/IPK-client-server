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
const int MAX_CLIENTS = 10;
/*
* Prototypes of functions
*/
void err_print(const char *msg);
bool args_err(int argc, const char** argv);

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
    int sockfd, newsockfd, clilen, n;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
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
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    /*
        Binding socket
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
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
    clilen = sizeof(cli_addr);
    /*
        Getting thing work
    */
    while(true)
    {
        cout << "Port No: " << port << endl;
        break;
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
    if ((garbage[0]!='\0') || (port < 0)) 
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
