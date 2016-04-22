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

string known_urls = "";
int attempts = 0; 
/*
* Used regular expressions
*/

/*
* Prototypes of functions
*/
void err_print(const char *msg);

bool args_err(int argc, char** argv, options* args);

string get_cont();

bool connect_to(char* hostname, int port, bool download, char* file_name);

bool save_response();
/*
*   Main
*/
int main(int argc, char **argv)
{   
    options args;
    if ( args_err(argc,argv,&args) ) 
    {
        err_print("Wrong parameters, usage: ./client -h hostname -p port [-­d|u] file_name");
        return EXIT_FAILURE;
    }

    if ( connect_to(args.hostname, args.port, args.download, args.file_name) ) 
    {
        return EXIT_FAILURE;
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
*   For checking arguments
*/
bool args_err(int argc, char** argv, options* args)
{
    if (argc != REQ_ARGC)
    {
        return EXIT_FAILURE;
    }
    int opt,h=0,p=0,u=0,d=0;
    while ((opt = getopt(argc,argv,"h:p:u:d:")) != EOF)
    {
        switch(opt)
        {
            case 'h': 
            {
                h += 10; 
                args->hostname = optarg ; 
                break;
            }
            case 'u':
            { 
                u++; 
                args->file_name = optarg; 
                break;
            }
            case 'd': 
            {
                d++; 
                args->file_name = optarg; ; 
                break;
            }
            case 'p': 
            {
                p += 9; 
                char* garbage = '\0';
                int port = strtoul(optarg,&garbage,0);
                if ((garbage[0]!='\0') || (port < 1024) || (port > 65535)) 
                {
                    return EXIT_FAILURE;
                }
                args->port = port; 
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
   
    char buffer[256];
    
    if (download)
    {
        cout<<"down "<< file_name <<endl;
    }
    else
    {
        cout<<"up "<< file_name <<endl;
    }
   
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return EXIT_FAILURE;
    }
    
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
   
    /* Now ask for a message from the user, this message
      * will be read by server
    */
    
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin); 
   
    /* Send message to the server */
    n = write(sockfd, buffer, strlen(buffer));
   
    if (n < 0) 
    {
        perror("ERROR writing to socket");
        return EXIT_FAILURE;
    }
   
    /* Now read server response */
    bzero(buffer,256);
    n = read(sockfd, buffer, 255);
   
    if (n < 0) 
    {
        perror("ERROR reading from socket");
        return EXIT_FAILURE;
    }
    
    printf("%s\n",buffer);

    return EXIT_SUCCESS;
}
/*
*   For sending head request and returning response
*/
string get_cont()
{
    return EXIT_SUCCESS;
}
/*
*   For writing into file
*/
bool save_response(string response, bool chunked)
{
    size_t pos = response.find("\r\n\r\n");
    response.erase(0, pos+4);
    if (chunked) 
    {
        string chunked_resp = response;
        response = "";
        unsigned int block_size;
        while ((pos = chunked_resp.find("\r\n")) != string::npos )
        {   // real cutting of string magic
            block_size = strtoul(chunked_resp.substr(0,pos).c_str(), NULL, 16);
            chunked_resp.erase(0, pos+2);
            response += chunked_resp.substr(0,block_size);
            chunked_resp.erase(0,block_size);
        }
    }
    ofstream myfile;
    myfile.open(file_name);
    myfile << response;
    myfile.close();
    return EXIT_SUCCESS;
}
