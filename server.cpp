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
void start_upload(int socket, const char* target);
void start_download(int socket, const char* target, size_t size);
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
    char buffer[RECV_BUFF];
    bzero(buffer,RECV_BUFF);
    // reading request into buffer 
    if (read(cli_socket,buffer,RECV_BUFF) < 0) 
    {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    } 
    // parsing message from request
    string msg = static_cast<string>(buffer);
    size_t size = msg.find("\r\n");
    string act = msg.substr(0,size);
    msg.erase(0,size+2);
    size = msg.find("\r\n");
    const char* target = (msg.substr(0,size)).c_str();
    msg.erase(0,size+2);
    // if there is SAVE in message client wants to upload 
    if (act == "SAVE")
    {   // then server find size of file in message and begin download
        size = msg.find("\r\n");
        size = stoi(msg.substr(0,size), &size);
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
            exit(EXIT_FAILURE);
        }
    }
    close(cli_socket);
}

void start_upload(int socket, const char* target)
{
    int code;
    printf("U want me to SEND u target: %s\n",target);

    ifstream file;

    struct stat filestatus;
    if (stat( target, &filestatus ) != 0)
    {
        code = write(socket,"ERR Server can not open file",28);
    }

    file.open (target, ios::in );

    if (file.is_open())
    {
        string msg = "OK\r\n";
        msg += to_string(filestatus.st_size) +"\r\n";
        code = write(socket,msg.c_str(),msg.size());
    }
    else
    {
        code = write(socket,"ERR Server can not open file",28);
    }
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
    printf("Send and exit\n");

}

void start_download(int socket, const char* target, size_t size)
{
    int code;
    printf("U want me to SAVE your target: %s size ",target);
    cout<<size<<endl;
    ofstream file;

    file.open (target, ios::binary );

    if (file.is_open())
    {
        code = write(socket,"READY\r\n",256);
    }
    else
    {
        code = write(socket,"ERR Server can not create file",31);
    }   
    if (code < 0) 
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
    printf("Save and exit\n");
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
            return EXIT_FAILURE;
        }
        // let thread handle client
        handle_request(cli_socket);
        //thread doer (handle_request, cli_socket); //spawn new thread that calls handle_request(cli_socket)
    }
    // when keyboard interrupt close socket and exit
    close(socket);
    return EXIT_SUCCESS;
}
