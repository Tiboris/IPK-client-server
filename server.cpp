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

/*
* Used regular expressions
*/

/*
* Prototypes of functions
*/
void err_print(const char *msg);
/*
*   Main
*/
int main(int argc, char const *argv[])
{   
    if ( ! ( argc != 1 ) ) {
        err_print("Wrong parameters, usage: ./webclient url");
        return EXIT_FAILURE;
    }
    cout<<argv[0]<<endl;
    return EXIT_SUCCESS;
}
/*
*   For printing errors
*/
void err_print(const char *msg)
{
    cerr << msg << endl;    
}
