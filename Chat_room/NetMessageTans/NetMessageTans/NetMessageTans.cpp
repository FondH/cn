#include<iostream>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") 
using namespace std;
int client_start() {

    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }




    while (true) {
        SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock == INVALID_SOCKET) {
            printf("INVALID scoket");
            return 0;
        }
        sockaddr_in sock_addr;
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_port = htons(30000);
     
     //   sock_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
        if (connect(client_sock, (sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
            printf("连接Server失败");
            closesocket(client_sock);
            return 0;
        }
        char* sendBuffer = new char[255];
        string tpstring = "Hi,FondQ";

        for (int i = 0; i < tpstring.length();i++) {
            sendBuffer[i] = tpstring[i];
        }

        sendBuffer[tpstring.length()] = 0x00;
        send(client_sock, sendBuffer, tpstring.length(), 0);

        char* reciBuffer = new char[255];
        int ret = recv(client_sock, reciBuffer, 255, 0);

        if (ret > 0) {
            reciBuffer[ret] = 0x00;
            printf("接受从Server: \n");
            printf(reciBuffer);
        }
        closesocket(client_sock);
    }

    WSACleanup();
    return 0;


}