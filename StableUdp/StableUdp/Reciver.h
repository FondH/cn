#pragma once
#include <winsock2.h>
#include<WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <string>
#include <time.h>
#include<iomanip>
#include<vector>
#include "UDP.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;


class Reciver {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in* reci_addr= new sockaddr_in();
    sockaddr_in* send_addr= new sockaddr_in();
    
    Udp package = Udp();

    char* SendBuffer = new char[PacketSize];

    
    char* FileBuffer = new  char[BufferSize];
    string InforBuffer = "";
    streamsize fileSize = 0;
    HANDLE reci_runner_handle = NULL;
    volatile bool reci_runner_keep = true;

public:
    Reciver() {

        reci_addr->sin_family = AF_INET;
        reci_addr->sin_port = htons(8999);
        inet_pton(AF_INET, ReciIp, &(reci_addr->sin_addr));

        int iResult = bind(s, (struct sockaddr*)this->reci_addr, sizeof(*reci_addr));
        
        if (iResult != 0) {
            cout << "Bind failed with error: " << WSAGetLastError() << endl;
        }
    }
    int _send() {

        memcpy(SendBuffer, &package, HeadSize);
        int rst_byte = sendto(this->s, (const char*)SendBuffer, HeadSize, 0, (struct sockaddr*)send_addr, sizeof(*send_addr));
        char error_message[100];
        if (rst_byte == SOCKET_ERROR) {
            int error_code = WSAGetLastError();
            cerr << "sendto failed with error: " << error_code << endl;
            return rst_byte;
        }

        cout << "------------------SEQ: "<< this->package.header.seq<<" -------------------\n";
        //print_udp(this->package);
        cout << "ST: " << this->package.header.get_st() << " ED: " << this->package.header.get_end() << endl;

        return rst_byte;
    }
    int get_connection();
    void to_file();
    friend DWORD WINAPI ReciHandler(LPVOID param);

    void send_rto() {
        sockaddr_in* dd = new sockaddr_in();
        dd->sin_family = AF_INET;
        dd->sin_port = htons(9001);
        inet_pton(AF_INET, ReciIp, &(dd->sin_addr));
        string se = "123";
        int rsb = sendto(this->s, se.c_str(), 4, 0, (sockaddr*)dd, sizeof(*dd));
        if (rsb == SOCKET_ERROR)
        {
            int error_code = WSAGetLastError();
            cerr << error_code;
        }
    }
};
void Reciver::to_file() {
    vector<string> tokens;
    string token;
    
    for (int i = 0; i < this->InforBuffer.length(); i++) {
        if (this->InforBuffer[i] != ':') {
            token += this->InforBuffer[i];
        }
        else {
            tokens.push_back(token);
            token.clear();
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    Out2file(this->FileBuffer, stoi(tokens[1]), "temp.jpg");

}
DWORD WINAPI ReciHandler(LPVOID param) {
    cout << "Start Acc the file\n";
    Reciver* reci = (Reciver*)param;
    char* buffer = new char[PacketSize];
    bool wait_ack = 0;
    bool fin = 0;
    bool st = 1;
    int seq = 100;
    int rst_byte = -1;
    char* iter = reci->FileBuffer;
    int send_addr_size = sizeof(*reci->send_addr);
    while (reci->reci_runner_keep && !fin) {


        if (recvfrom(reci->s, buffer, PacketSize, 0, (sockaddr*)reci->send_addr, &send_addr_size) > 0) {
            /*
            ���   get_status() ����0/1
            */

            Udp* package = (Udp*)buffer;
            cout << " ------------- Sender: -----------\n";
            //print_udp(*package);
            cout << "ST: " << package->header.get_st() << " ED: " << package->header.get_end()<<endl;

   
            if (!package->cmp_cheksum() || !(package->header.get_status() == wait_ack))
            {
                cout << "----- Time Out or CheckSum error -----" << endl;
                rst_byte = reci->_send();
                continue;

            }
            // ͨ��У��� ����״̬��Ӧ

            if (package->header.get_st()) 
                reci->InforBuffer += package->payload;
            
            else
            {
                st = 0;
                memcpy(iter, package->payload, package->header.data_size);
                iter += package->header.data_size;
            }
            
            if (package->header.get_end())
                fin = 1;


            //���
            reci->package.header.set_St(st);
            reci->package.header.set_End(fin);
            reci->package.set_flag(wait_ack);
            reci->package.header.seq = seq++;
            reci->package.header.ack = package->header.seq + 1;

            rst_byte = reci->_send();    
            wait_ack = !wait_ack;

        }


    }
    reci->to_file();
    return 1;
}

int Reciver::get_connection() {
    /*�ȴ����� reci*/
    cout << "wait..." << endl;
    char* ReciBuffer = new char[PacketSize];
      /*��������  �ظ��ڶ���*/
    cout << "Reciver is ready !" << endl;
    reci_runner_handle = CreateThread(NULL, 0, ReciHandler, (LPVOID)this, 0, NULL);
   // to_file();
    return 1;
}
int main(){
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << endl;
        
    }

    Reciver reciver = Reciver();
    reciver.get_connection();
   // reciver.send_rto();
    string cin_buffer;
    while (true) {
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;

    }
}