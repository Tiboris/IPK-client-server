#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

using namespace std;
/*
*   Structure for options
*/
typedef struct Options
{
    char* hostname;
    char* file_name;
    int port;
    bool download;
} options;

/*
*   Used constants and variables
*/
const int REQ_ARGC = 7;
const int REQ_COMB = 20;
const int MAX_ATTEMPTS = 5;
const size_t BUFF_SIZE = 1024;
string file_name = "";

int attempts = 0; 
/*
* Used regular expressions
*/

/*
* Prototypes of functions
*/
void err_print(const char *msg);

bool args_err(int argc, char** argv, options* args);

bool connect_to(char* hostname, int port, bool download, char* file_name);

string generate_message(bool download,char* file_name);

/*
*   Main
*/
int main(int argc, char **argv)
{   
    options args;
    if ( args_err(argc,argv,&args) ) 
    {
        perror("Wrong parameters, usage: ./client -h hostname -p port [-­d|u] file_name");
        return EXIT_FAILURE;
    }

    if ( connect_to(args.hostname, args.port, args.download, args.file_name) ) 
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
/*
*   For checking arguments
*/
bool args_err(int argc, char** argv, options* args)
{
    if (argc != REQ_ARGC)
    {
        return EXIT_FAILURE;
    }
    int opt,port,h=0,p=0,u=0,d=0;
    char* garbage = '\0';
    while ((opt = getopt(argc,argv,"h:p:u:d:")) != EOF)
    {
        switch(opt)
        {
            case 'h': 
            {
                h += 10; args->hostname = optarg ; break;
            }
            case 'u':
            { 
                u++; args->file_name = optarg; args->download=false; break;
            }
            case 'd': 
            {
                d++; args->file_name = optarg; args->download=true; break;
            }
            case 'p': 
            {
                port = strtoul(optarg,&garbage,0);
                if ((garbage[0]!='\0') || (port < 1024) || (port > 65535)) 
                {
                    return EXIT_FAILURE;
                }
                p += 9; args->port = port; 
                break;
            }
            case '?': 
            {
                return EXIT_FAILURE;
            }
        }
    }
    if ((h > 10) || (p > 9) || (u > 1) || (d > 1) || ((h+p+d+u) != REQ_COMB))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
/*
*   For connection handling
*/
bool connect_to(char* hostname, int port, bool download, char* file_name)
{   
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
   
    char resp_buffer[256];
    string req_msg = generate_message(download,file_name);
    cout << "Sending:\n" << req_msg;
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return EXIT_FAILURE;
    }
    /* Maybe use for */
    server = gethostbyname(hostname);
   
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        return EXIT_FAILURE;
    }
   
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR connecting");
        return EXIT_FAILURE;
    }

    /* Send message to the server */
    n = write(sockfd, req_msg.c_str(), req_msg.size());
   
    if (n < 0) 
    {
        perror("ERROR writing to socket");
        return EXIT_FAILURE;
    }
   
    /* Now read server response */
    bzero(resp_buffer,256);
    n = read(sockfd, resp_buffer, 255);
   
    if (n < 0) 
    {
        perror("ERROR reading from socket");
        return EXIT_FAILURE;
    }
    
    printf("%s\n",resp_buffer);

    return EXIT_SUCCESS;
}
/*
*   For writing into file
*/
string generate_message(bool download,char* file_name)
{
    string req_msg = "";
    if (download)
    {
        req_msg += "GET\r\n";
        req_msg += static_cast<string>(file_name)+"\r\n\r\n";
    }
    else
    {
        req_msg += "SAVE\r\n";
        req_msg += static_cast<string>(file_name) + "\r\n";
        struct stat filestatus;
        stat( file_name, &filestatus );
        req_msg += to_string(filestatus.st_size) + "\r\n\r\n";
    }
    return req_msg;
}
