#include <string>
#include <time.h>

#include "Sender.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define GBN 2
#define SR 3
using namespace std;




int main(){

    //string filename = "helloworld.txt";
    string filename = "2.jpg";
    string cin_buffer;

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
        cout << "WSAStartup failed with error: " << iResult << endl;

    Sender sender = Sender();
    sender.GetFile(filename);
    sender.get_connection(3);


    while (true) {
       /* cout << "input filename\n";
        cin >> filename;
        sender.GetFile(filename);
        sender.init();*/
       // sender.get_connection();
        
        cout << "q to exit\n";
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;
    }
    
}