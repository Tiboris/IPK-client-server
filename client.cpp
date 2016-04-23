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
const size_t RESP_BUFF = 40;
const size_t MAX_BUFF_SIZE = 1024;
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

bool start_upload(int socket, const char* target);

bool start_download(int socket, const char* target, size_t size);

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
    int sockfd, code;
    struct sockaddr_in serv_addr;
    struct hostent *server;
   
    char resp_buffer[RESP_BUFF];
    string req_msg = generate_message(download,file_name);
    if (req_msg == "ERR")
    {
        return EXIT_FAILURE;
    }
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
    code = write(sockfd, req_msg.c_str(), req_msg.size());
   
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        return EXIT_FAILURE;
    }
    /*
        TODO DOWN / UPLO
    */
    /* Now read server response */
    bzero(resp_buffer,RESP_BUFF);
    code = read(sockfd, resp_buffer, RESP_BUFF-1);
   
    if (code < 0) 
    {
        perror("ERROR reading from socket");
        return EXIT_FAILURE;
    }
    string resp = static_cast<string>(resp_buffer);
    size_t size = resp.find("\r\n");
    string act = resp.substr(0,size);
    resp.erase(0,size+2);
    if (act == "OK")
    {
        size = resp.find("\r\n");
        size = stoi(resp.substr(0,size), &size);
    }

    if (download && (act == "OK"))
    {
        return start_download(sockfd, file_name, size);
    }
    else if (!download && (act == "READY"))
    {
        return start_upload(sockfd, file_name);        
    }
    else
    {
        cerr << (resp_buffer) << endl;
        return EXIT_FAILURE;
    }
}
/*
*   For writing into file
*/
string generate_message(bool download,char* file_name)
{
    string req_msg = "";
    if (download)
    {
        req_msg += "SEND\r\n";
        req_msg += static_cast<string>(file_name)+"\r\n";
    }
    else
    {
        req_msg += "SAVE\r\n";
        req_msg += static_cast<string>(file_name) + "\r\n";
        struct stat filestatus;
        if (stat( file_name, &filestatus ) != 0)
        {
            perror("ERROR opening local file to upload");
            return "ERR";
        }
        req_msg += to_string(filestatus.st_size) + "\r\n";
    }
    return req_msg;
}

bool start_upload(int socket, const char* target)
{
    ifstream file;
    file.open (target, ios::in );  
    if (! file.is_open())
    {
        perror("ERROR can not open file");
        return EXIT_FAILURE;
    }

    char data [MAX_BUFF_SIZE];
    bzero(data, MAX_BUFF_SIZE);
    while(true)
    {
        file.read(data,MAX_BUFF_SIZE);
        if ((write(socket,data,file.gcount())) < 0) 
        {
            perror("ERROR writing to socket");
            return EXIT_FAILURE;
        }

        bzero(data, MAX_BUFF_SIZE);
        if (file.eof())
        {
            break;
        }
    }
    return EXIT_SUCCESS;
}

bool start_download(int socket, const char* target, size_t size)
{

    ofstream file;
    string tail = "_("+to_string(socket)+").temporary";
    string tmp = target+tail;
    file.open (tmp, ios::out | ios::binary );

    if (! file.is_open())
    {
        perror("ERR can not create file");
        return EXIT_FAILURE;
    }

    if ((write(socket,"READY\r\n",10)) < 0)
    {
        perror("ERROR writing to socket");
        return EXIT_FAILURE;
    }

    int res;
    size_t total=0;
    char part[MAX_BUFF_SIZE];
    while ((res = read(socket, part, MAX_BUFF_SIZE)))
    {
        total += res;
        if (res < 1) 
        {
            perror("ERROR while getting response");
            return EXIT_FAILURE;
        }
        else 
        { 
            file.write(part,res);
            bzero(part, MAX_BUFF_SIZE); // buff erase
        }
    }
    file.close(); 

    if ( total != size)
    {
        perror("ERROR size of temporary file do not match");
        return EXIT_FAILURE;
    }
       
    if (rename( tmp.c_str() , target ) != 0)
    {
        perror("ERROR while renamig temp file");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
