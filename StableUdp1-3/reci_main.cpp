#pragma once
#include <winsock2.h>
#include<WS2tcpip.h>
#include <windows.h>

#include "UDP.h"
#include "reci.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define GBN 2
#define SR 3
#define Default 1

using namespace std;

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
        cout << "WSAStartup failed with error: " << iResult << endl;


    Reciver reciver = Reciver();
    //reciver.get_connection();
   // reciver.send_rto();


    string cin_buffer;
    int i = 1;
    while (true) {
        cout << "No: " << i++ << endl;
        reciver.init();
        reciver.get_connection(3);
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;
    }
}