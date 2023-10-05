#include<iostream>
#include<winsock2.h>
#include <windows.h>
#include<string>
#pragma comment(lib,"ws2_32.lib") 
#include "member.h"
#include "Server.h"
using namespace std;
volatile bool KeepThread = false;
DWORD WINAPI GetMessFromClient(LPVOID lpParam) {

    char* recvBuffer = new char[255];
    SOCKET ClientSocket = (SOCKET)lpParam;
    while (KeepThread) {

        int ret = recv(ClientSocket, recvBuffer, 255, 0);
        //recvBuffer[9] = '\0';
        cout << recvBuffer<<endl;
}
  
    return 0;
}
int CreateThread(SOCKET id) {

    HANDLE hThread;
    DWORD dwThreadId;
    KeepThread = true;
    hThread = CreateThread(
        NULL,                     // 默认安全属性
        0,                        // 默认线程堆栈大小
        GetMessFromClient,         // 线程函数
        (LPVOID)id,                     // 传递给线程函数的参数
        0,                        // 创建标志，0表示默认
        &dwThreadId               // 用于存储线程ID的变量
    );

    if (hThread == NULL) {
        printf("Failed to create thread  \n");
        return 0;
    }

    printf("Thread is done of  ! \n");
    return 1;
}
void CloseThread(int Thread_id) {
    KeepThread = false;
    printf("Thread %n has closed", &Thread_id);
}


int start() {

    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(30000);
    sock_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    if(bind(server_sock,(LPSOCKADDR)&sock_addr,sizeof(sock_addr)) == SOCKET_ERROR){
        cout << "BIND Server Error !" << endl;
        
    }

    if (listen(server_sock, 5) == SOCKET_ERROR)
    {
        printf("listen Server Error !");
        return 0;
    }


    SOCKET sClient;
    sockaddr_in remote_addr;
    int lenOfaddr= sizeof(remote_addr);
    char* reciBuffer = new char[255];

        printf("等待连接!");
        sClient = accept(server_sock, (SOCKADDR*)&remote_addr, &lenOfaddr);  
        
        if (sClient == INVALID_SOCKET) {
            printf("accept error");
        }
        else {
            CreateThread(sClient);

        }
        printf("接受一个连接： \n");

        /*
        int ret = recv(sClient, reciBuffer, 255, 0);

        if (ret > 0)
        {
            reciBuffer[ret] = 0x00;
            printf("Server已接收：\n");
            printf(reciBuffer);
        }*/
        
        char* sendBuffer = new char[255];
        //send(sClient, sendBuffer, strlen(sendBuffer), 0);

        while (true) {
            cin >> sendBuffer;
            sendBuffer[strlen(sendBuffer)] = '`';
            if(!strcmp(sendBuffer, "exit")) {
                CloseThread(0);
                printf("结束通话!");
                return 0;
            }
            send(sClient, sendBuffer,255, 0);
        }
        closesocket(sClient);
        WSACleanup();
        printf("Server Closed");
        return 0;
    

}



int main() {
  /*
    start();
    char* sendBuffer = new char[25];

    string tpstring = "Hi,FondQ";
    for (int i = 0; i < tpstring.length();i++) {
        sendBuffer[i] = tpstring[i];
    }
    sendBuffer[tpstring.length()] = '\0';
    cout << strlen(sendBuffer);
    printf("\n%s", sendBuffer);
    
    
    string mess = "";
    mess.append("status");
    for (int i = 0;i < 10;i++) {
        mess += to_string(i);
    }
    cout <<mess.c_str();
    */
    Server server(8999);
        server.start_listen();

    
}