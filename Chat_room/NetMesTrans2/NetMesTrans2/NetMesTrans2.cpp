﻿#include<iostream>
#include<winsock2.h>
#include <WS2tcpip.h> 
#include<vector>
#include<ctime>
#include<string>
#include "mess.h"
#pragma comment(lib,"ws2_32.lib") 
using namespace std;


volatile bool KeepThread = false;
volatile bool Isready = 0;
string myusername = "default";
int roomNum = 0;
int currentNum = 1;
string sendString = "";
char* recvBuffer = new char[255];
char* sendBuffer = new char[255];
vector<mess> messBuffer;



const int chatWidth = 60;
const int mess_display = 15;



vector<string> split(string& s, char delimiter) {
    //将string 按照;:, 划分
    vector<string> tokens;
    string token;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] != delimiter) {
            token += s[i];
        }
        else {
            tokens.push_back(token);
            token.clear();
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

void printTime(time_t rawtime) {

    tm ptminfo;
    localtime_s(&ptminfo, &rawtime);
    printf("current: %02d-%02d-%02d %02d:%02d:%02d",
        ptminfo.tm_year + 1900, ptminfo.tm_mon + 1, ptminfo.tm_mday,
        ptminfo.tm_hour, ptminfo.tm_min, ptminfo.tm_sec);
}
void printCurrentTime() {
    time_t rawtime;
    time(&rawtime);
    printTime(rawtime);

}
time_t getCurrentTime() {

    time_t rawtime;
    time(&rawtime);

    return rawtime;
}
void CloseThread(int Thread_id) {
    KeepThread = false;
    printf("Thread %n has closed", &Thread_id);
}


void recvMessProc() {
    /*
    chat:username:mess:time
    exit:username:time
    enter:username:time
    */

    string stringBuffer = string(recvBuffer);
    vector<string> tokens = split(stringBuffer, ':');
    if (tokens[0] == "chat") {
        mess tm = { tokens[0],tokens[1],tokens[2],tokens[3] };
        messBuffer.push_back(tm);

    }
    else if (tokens[0] == "status") {
        mess tm = { "ready","default",tokens[1],"0" };

        messBuffer.push_back(tm);
        Isready = 0;

        //cout << tokens[1] << endl;

    }
    else if (tokens[0] == "exit") {
        mess tm = { "sys",tokens[1],"exit!",tokens[2] };
        messBuffer.push_back(tm);
        currentNum = stoi(tokens[3]);
        if (tm.username == myusername)
            CloseThread(0);

    }
    else if (tokens[0] == "choice") {

    }
    else if (tokens[0] == "enter") {
        // cout << roomNum << myusername;
        mess tm = { "sys",tokens[2],"enter!",tokens[3] };
        currentNum = stoi(tokens[4]);
        messBuffer.push_back(tm);
    }
}

void sendMessProc() {
    string stringBuffer = "";

    if (!strcmp(sendBuffer, "exit")) {
        stringBuffer += "exit:";
        stringBuffer += myusername;
    }
    else {
        stringBuffer += "chat:" + myusername + ":";
        for (int i = 0;i < strlen(sendBuffer);i++)
            stringBuffer += (sendBuffer[i]);
        stringBuffer += ":" + to_string(int(getCurrentTime()));

    }
    sendString = stringBuffer;
}
void display() {
    if (!Isready) {
        vector<string> status = split(messBuffer[0].message, ',');
        cout << "The Server's Room status:(Current num / Max num)" << endl;
        int i = 0;
        for (string sta : status) 

            cout << "Room " << i++ << ": " << sta << endl;
            
        return;
    }

    cout << "+----------------------------------------------------------------+\n";
    cout << "| Room " << roomNum << " | People: " << currentNum << " | " << "Current Time:";
    printCurrentTime();
    cout << " |\n";
    cout << "+----------------------------------------------------------------+\n";
    for (mess& m : messBuffer) {
        if (m.type == "sys") {
            cout << "           " << m.username << "  " << m.message << "...." << endl;
        }

        else {
            if (m.username == myusername)
            {
                int lenPadding = chatWidth - m.message.size();
                string padding = string(lenPadding, ' ');
                cout << padding << m.message << " :" << myusername << endl;
            }
            else
                cout << m.username << ": " << m.message << endl;
        }
    }
    cout << "+----------------------------------------------------------------+\n";
    cout << "Please enter ... \n";
}

void refrash() {
    system("cls");
    display();

}


DWORD WINAPI GetMessFromClient(LPVOID lpParam) {

    SOCKET ClientSocket = (SOCKET)lpParam;
    while (KeepThread) {
        int recvbytes = recv(ClientSocket, recvBuffer, 255, 0);
        //cout << recvBuffer;
        if (recvbytes > 1) {
            recvMessProc();
            refrash();
        }
        //printf("Client: %s \n", recvBuffer);

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



int main() {
    mess tp = { "chat","12","HHH","12" };
    mess tp2 = { "chat" ,"default","aL","12" };
    // messBuffer.push_back(tp);
    // messBuffer.push_back(tp2);
     //display();
    string username;
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

    while (true) {
        // 连接
        SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock == INVALID_SOCKET) {

            printf("INVALID scoket");
            break;
        }
        sockaddr_in sock_addr;
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_port = htons(8999);
        inet_pton(AF_INET, "192.168.137.1", &sock_addr.sin_addr);
        if (connect(client_sock, (sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
            printf("连接Server失败");
            closesocket(client_sock);
            break;
        }

        CreateThread(client_sock);
        

       

       // 选择房间进入
        
       string tpstring = "";
       while (!Isready) {
            Sleep(2000);
            cout << "\nPlease input roomNum:";
           // cin >> roomNum;
            cout << "\nPlease input username: ";
           // cin >> myusername;
            
            myusername = "xuyihang";
            roomNum = 1;

            Isready = 1;
            tpstring += "choice:" + to_string(roomNum) + ":" + myusername;
            send(client_sock, tpstring.c_str(), 255, 0);
            break;
        }
        


        // 发送消息
        while (true) {

            cin >> sendBuffer;
            sendMessProc();
            send(client_sock, sendString.c_str(), 255, 0);
            //refrash();
            if (!strcmp(sendBuffer, "exit"))
                break;

        }
        closesocket(client_sock);
        break;
    }

    WSACleanup();
    return 0;
}