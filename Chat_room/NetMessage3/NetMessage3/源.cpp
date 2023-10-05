#include<iostream>
#include<winsock2.h>
#include <WS2tcpip.h> 
#include<vector>
#include<ctime>
#include<string>
#include "mess.h"
#pragma comment(lib,"ws2_32.lib") 
using namespace std;


volatile bool KeepThread = false;
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
    //��string ����;:, ����
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
        cout << tokens[1] << endl;
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
        NULL,                     // Ĭ�ϰ�ȫ����
        0,                        // Ĭ���̶߳�ջ��С
        GetMessFromClient,         // �̺߳���
        (LPVOID)id,                     // ���ݸ��̺߳����Ĳ���
        0,                        // ������־��0��ʾĬ��
        &dwThreadId               // ���ڴ洢�߳�ID�ı���
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
    messBuffer.push_back(tp);
    messBuffer.push_back(tp2);
    //display();
    string username;
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

    while (true) {
        // ����
        SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock == INVALID_SOCKET) {

            printf("INVALID scoket");
            break;
        }
        sockaddr_in sock_addr;
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_port = htons(8999);
        inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr);
        if (connect(client_sock, (sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
            printf("����Serverʧ��");

            closesocket(client_sock);
            break;
        }

        CreateThread(client_sock);

        // ѡ�񷿼����
        string tpstring = "choice:1:xuyihang";
        myusername = "xuyihang";
        roomNum = 1;


        send(client_sock, tpstring.c_str(), 255, 0);

        //������Ϣ ˢ�½���



        // ������Ϣ
        while (true) {

            cin >> sendBuffer;
            sendMessProc();
            send(client_sock, sendString.c_str(), 255, 0);
            //refrash();
            if (!strcmp(sendBuffer, "exit"))
                break;
            
        }
        closesocket(client_sock);
    }

    WSACleanup();
    return 0;
}