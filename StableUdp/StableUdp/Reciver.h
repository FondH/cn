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

    volatile bool connected = true;
    volatile bool reci_runner_keep = true;

    int bytes = 0;
public:
    Reciver() {

        reci_addr->sin_family = AF_INET;
        reci_addr->sin_port = htons(8999);
        inet_pton(AF_INET, ReciIp, &(reci_addr->sin_addr));

        int iResult = bind(s, (struct sockaddr*)this->reci_addr, sizeof(*reci_addr));
        if (iResult != 0) 
            cout << "Bind failed with error: " << WSAGetLastError() << endl;     
    }
    int _send() {
        this->package.set_cheksum();
        memcpy(SendBuffer, &package, HeadSize);
        int rst_byte = sendto(this->s, (const char*)SendBuffer, HeadSize, 0, (struct sockaddr*)send_addr, sizeof(*send_addr));
        char error_message[100];
        if (rst_byte == SOCKET_ERROR) {
            int error_code = WSAGetLastError();
            cerr << "sendto failed with error: " << error_code << endl;
            return rst_byte;
        }

        cout << "\n-+-+-+-+-+-+-+-+-+- Reciver' Packetage -+-+-+-+-+-+-+-+-+-\n";
        //print_udp(this->package);

        cout <<"SYN: "<< this->package.header.get_Syn()
            <<" ACK "<< this->package.header.get_Ack()
            << "ST: " << this->package.header.get_st()
            << " ED: " << this->package.header.get_end() 
            <<" STATUS: " <<this->package.header.get_status()<< endl;
        cout << "-+-+-+-+-+-+-+-+-+-SEQ: " << this->package.header.seq << " -+-+-+-+-+-+-+-+-+-\n";
        
        this->bytes += HeadSize;
        return rst_byte;
    }
    int get_connection();
    void to_file();
    friend DWORD WINAPI ReciHandler(LPVOID param);
    friend DWORD WINAPI ConnectHandler(LPVOID param);
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

    clock_t send_st = clock();
    while (reci->reci_runner_keep && !fin) {


        if (recvfrom(reci->s, buffer, PacketSize, 0, (sockaddr*)reci->send_addr, &send_addr_size) > 0) {
            /*
            拆包   get_status() 返回0/1
            */

            Udp* package = (Udp*)buffer;
            cout << " ------------- Sender: ------------\n";
            //print_udp(*package);
            cout << "ST: " << package->header.get_st() <<
                " ED: " << package->header.get_end()<<
                " STATUS: " <<package->header.get_status()<< endl;

   
            if (!package->cmp_cheksum() || !(package->header.get_status() == wait_ack))
            {
                cout << "------- Time Out or CheckSum error -------" << endl;
                rst_byte = reci->_send();
                continue;

            }

            

            // 通过校验和 发送状态对应

            if (package->header.get_st()) 
                reci->InforBuffer += package->payload;
            
            else
            {


                st = 0;
                memcpy(iter, package->payload, package->header.data_size);
                iter += package->header.data_size;
            }
            
            if (package->header.get_end())
                //fin报文收到后，则完全收到了，回复一个fin报文，让sender结束。即挥手第二次
                fin = 1;


            //打包
            reci->package.header.set_Ack(1);
            reci->package.header.set_St(st);
            reci->package.header.set_End(fin);
            reci->package.set_flag(wait_ack);
            reci->package.header.seq = seq++;
            reci->package.header.ack = package->header.seq + 1;

            rst_byte = reci->_send();    
            wait_ack = !wait_ack;

        }


    }
    
    cout << "Thread exit... \nTotal Length: " << reci->bytes << " bytes\n" << "Duration: " << (double)((clock() - send_st) / CLOCKS_PER_SEC) << " secs" << endl;
    cout << "Speed Rate: " << (double)reci->bytes / ((clock() - send_st) / CLOCKS_PER_SEC) << " Bps" << endl;
    reci->to_file();
    return 1;
}


DWORD WINAPI ConnectHandler(LPVOID param) {
    srand((unsigned)time(NULL));
    char* ReciBuffer = new char[PacketSize];


    Reciver* reci = (Reciver*)param;
    socklen_t send_addr_len = sizeof(*reci->send_addr);

    bool SYN = 0;
    
    bool succed = 0;



    clock_t sec_st = clock();

    while (!succed) {


        while (recvfrom(reci->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)reci->send_addr, &send_addr_len) <= 0) {

          /* if (clock() - sec_st > MAX_TIME) {
                cout << "-------- Time out --------\n";
                sender->_send(0);
                sec_st = clock();
            }*/

        }

        Udp* dst_package = (Udp*)ReciBuffer;
        int dSYN = dst_package->header.get_Syn();
        int dACK = dst_package->header.get_Ack();
        if ( dst_package->cmp_cheksum() )
        {
            cout << "------------------- Dst Package ---------------- \nSYN: " << dst_package->header.get_Syn() << " ACK: " << dst_package->header.get_Ack() << endl;


            if (!SYN) {
                if (dSYN && !dACK)
                {
                    SYN = 1;
                    reci->package.set_flag(SYN, 0, 1);
                    reci->_send();
                    cout << "第一次挥手成功" << endl;
                }
                else {
                    cout << "第一次挥手失败" << endl;
                    continue;
                }
            }
            else {
                if (!dSYN && dACK) {
                    succed = true;
                    cout << "第三次握手成功" << endl;
                    cout << "Recive Thread ready" << endl;
                    reci->package.set_flag(0,0,1);
                 }

            }

        }

    }
    reci->connected = 1;
    return 1;
}




int Reciver::get_connection() {
    /*等待连接 reci*/
    cout << "wait..." << endl;
    
    /*三次握手  回复第二次*/

    CreateThread(NULL, 0, ConnectHandler, (LPVOID)this, 0, NULL);


    while (!this->connected)
        Sleep(100);
    cout << "\n\nstart to reci !" << endl;
    reci_runner_handle = CreateThread(NULL, 0, ReciHandler, (LPVOID)this, 0, NULL);
 
     return 1;
}





int main(){
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
        cout << "WSAStartup failed with error: " << iResult << endl;
        
    
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