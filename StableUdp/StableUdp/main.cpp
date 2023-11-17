#include <string>
#include <time.h>

#include "Reciver.h"
#include "Sender.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

using namespace std;




int main(){

    string filename = "helloworld.txt";

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
        cout << "WSAStartup failed with error: " << iResult << endl;

    
    Sender sender = Sender();
    string cin_buffer;
    //sender.GetFile(filename);
   // sender.get_connection();
   

    string a;
    while (true) {
        cout << "input filename\n";
        cin >> filename;
        sender.GetFile(filename);
        sender.init();
        sender.get_connection();

        cout << "q to exit\n";
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;
    }


}