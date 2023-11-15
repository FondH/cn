#include <string>
#include <time.h>

#include "Reciver.h"
#include "Sender.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

using namespace std;




int main(){

    string filename = "test.jpg";
    string outFile = "out.txt";
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << endl;

    }
    Sender sender = Sender();
    sender.GetFile(filename);
    sender.get_connection();
    string cin_buffer;
    while (true) {
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;

    }
}