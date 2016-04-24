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
void start_upload(int socket, string target);
void start_download(int socket, string target, size_t size);
/*
*   Main
*/
int main(int argc, char const *argv[])
{   
    if (args_err(argc,argv)) 
    {
        cerr<<("Wrong parameters, usage: ./server -p [port_number]")<<endl;
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
    char buffer[RECV_BUFF];
    bzero(buffer,RECV_BUFF);
    // reading request into buffer 
    if (read(cli_socket,buffer,RECV_BUFF) < 0) 
    {
        perror("ERROR reading from socket");
        return;
    } 
    // parsing message from request
    string msg = static_cast<string>(buffer);
    size_t size = msg.find("\r\n");
    string act = msg.substr(0,size);
    msg.erase(0,size+2);
    size = msg.find("\r\n");
    string target = msg.substr(0,size);
    msg.erase(0,size+2);
    // if there is SAVE in message client wants to upload 
    if (act == "SAVE")
    {   // then server find size of file in message and begin download
        size = msg.find("\r\n");
        size = atoi(msg.substr(0,size).c_str());
        start_download(cli_socket,target,size);
    }
    else if (act == "SEND")
    {   // if there is SEND in message client wants to upload 
        start_upload(cli_socket,target);
    }    
    else // if bad message came
    {
        if ((write(cli_socket,"ERR Server does not understand request",38)) < 0)
        {
            close(cli_socket);
            return;
        }
    }
    close(cli_socket);
}

void start_upload(int socket, string target)
{
    int code;
    ifstream file;
    struct stat filestatus;
    file.open (target, ios::in );
    if (stat( target.c_str() , &filestatus ) != 0)
    {
        if ((write(socket,"ERR Server can not open file",29))< 0) 
        {
            perror("ERROR writing to socket");
            return;
        }
    }

    if (file.is_open())
    {
        string msg = "OK\r\n";
        stringstream ss;
        ss << filestatus.st_size;
        msg += ss.str() +"\r\n";
        code = write(socket,msg.c_str(),msg.size());
    }
    else
    {
        code = write(socket,"ERR Server can not open file",28);
        return;
    }
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        return;
    }

    char buffer[RECV_BUFF];
    bzero(buffer,RECV_BUFF);
    // reading request into buffer 
    if (read(socket,buffer,RECV_BUFF) < 0) 
    {
        perror("ERROR reading from socket");
        return;
    }
    
    string msg = static_cast<string>(buffer);
    if (msg != "READY\r\n")
    {
        cerr<<("ERROR do not understand")<<endl;
        return;
    }

    char data [MAX_BUFF_SIZE];
    bzero(data, MAX_BUFF_SIZE);
    while(true)
    {
        file.read(data,MAX_BUFF_SIZE);
        code = write(socket,data,file.gcount());
        bzero(data, MAX_BUFF_SIZE);
        if (code < 0) 
        {
            perror("ERROR writing to socket");
            return;
        }

        if (file.eof())
        {
            break;
        }
    }
}

void start_download(int socket, string target, size_t size)
{
    ofstream file;
    stringstream ss;
    ss << socket;
    string tail = "_("+ ss.str() +").temporary";
    string tmp = target+tail;
    file.open (tmp, ios::out | ios::binary );

    if (! file.is_open())
    {
        perror("ERR can not create file");
        return;
    }

    if ((write(socket,"READY\r\n",10)) < 0)
    {
        perror("ERROR writing to socket");
        return;
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
            return;
        }
        else 
        { 
            file.write(part,res);
            bzero(part, MAX_BUFF_SIZE); // buff erase
        }
    }

    if (total != size)
    {
        perror("ERROR size of temporary file do not match");
        return;
    }

    file.close();    
    if (rename( tmp.c_str() , target.c_str() ) != 0)
    {
        perror("ERROR while renamig temp file");
        return;
    }
}

int create_socket(int port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    // First call socket() function 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return -1;
    }
    // Initialize socket structure 
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    // Binding socket
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
    {
        perror("ERROR on binding");
        return -1;
    }
    // Returning binded socket
    return sockfd;
}

bool begin_listen(int socket, bool interrupt)
{
    struct sockaddr_in client_addr;
    // Start listening to clients max 10 9 in queue
    listen(socket,MAX_CLIENTS);
    socklen_t cli_len = sizeof(client_addr);
    while(true)
    {
        if (interrupt)
        {
            break;
        }
        // creating new socket for client
        int cli_socket = accept(socket, (struct sockaddr *) &client_addr, &cli_len);
        // checking if error occured
        if (cli_socket < 0) 
        {
            perror("ERROR on accept");
            continue;
        }
        // let thread handle client
        thread t (handle_request, cli_socket);
        t.detach();
    }
    // when keyboard interrupt close socket and exit
    close(socket);
    return EXIT_SUCCESS;
}
