#pragma once
#include <winsock2.h>
#include<WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include<iomanip>
#include <ctime>
#include<stdlib.h>
#include <windows.h>
#include "UDP.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

#define MAX_TIME  0.2*CLOCKS_PER_SEC 


const double LOSS_RATE = 0.01; // 丢包率（0.1 表示10%的丢包率）


void SimulateDelay() {
    double i = rand() / RAND_MAX;
    if(i < MAX_DELAY_RATE)
        Sleep(MAX_DELAY_MS);
 
}

bool SimulateDrop() {
    
    double i = rand()/double(RAND_MAX);
    return i < LOSS_RATE;
}



class Sender {
private:

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in* dst_addr= new sockaddr_in();
    sockaddr_in* src_addr = new sockaddr_in();
 

    Udp package = Udp();
    char* SendBuffer = new char[PacketSize];
    char* FileBuffer;

    string fileName = "Fond.txt";
    streamsize fileSize = 0;
    HANDLE send_runner_handle = NULL;


    volatile bool send_runner_keep = true;
    volatile bool connected = false;

    int bytes;
public:
    Sender();

    int _send(int size);  
    int get_connection();
    int set_flag(bool seq, bool ack, bool syn);
    void print_info() { print_udp(package); }
    int set_size(int s) { package.header.set_size(s); }
    void GetFile(string name) { 
        this->fileName = name;
        this->fileSize = ReadFile(name, FileBuffer);
    };
    void init() {
        memset(this->SendBuffer, 0, PacketSize);
        send_runner_keep = true;
        connected = false;
        
        bytes=0;
    }
    friend DWORD WINAPI SConnectHandler(LPVOID param);
    friend DWORD WINAPI SendHandler(LPVOID param);
    ~Sender();
};
Sender::Sender() {
    //srand(static_cast<unsigned int>(time(0)));
    //Bind 8998 
    this->src_addr->sin_family = AF_INET;
    this->src_addr->sin_port = htons(8998);
    inet_pton(AF_INET, ReciIp, &(src_addr->sin_addr));
    int iResult = bind(s, (struct sockaddr*)this->src_addr, sizeof(*src_addr));

    if (iResult != 0)
        cout << "Bind failed with error: " << WSAGetLastError() << endl;

    else
        cout << "Sender set up\n" << "Port: " << ntohs(this->src_addr->sin_port)<<endl;
   
    //非阻塞态
    unsigned long mode = 1;
    ioctlsocket(this->s, FIONBIO, &mode);
    FileBuffer = new char[BufferSize];
    send_runner_handle = NULL;
    bytes = 0;
}
Sender::~Sender() {
    closesocket(s);
    send_runner_keep = false;
}

int Sender::_send(int size) {

    memcpy(this->SendBuffer, &this->package, size + HeadSize);

    int rst_byte = size + HeadSize;
    if (!SimulateDrop()) {
        SimulateDelay();
        rst_byte = sendto(this->s, this->SendBuffer, size + HeadSize, 0, (sockaddr*)this->dst_addr, sizeof(*this->dst_addr));

        char error_message[100];
        if (rst_byte == SOCKET_ERROR) {
            int error_code = WSAGetLastError();
            cerr << "sendto failed with error: " << error_code << endl;
            return rst_byte;
        }

    }
    else
        cout << "\n---------------------------- DROP ---------------------------- " << endl;


    cout << "\n------------------------- SEQ: " << this->package.header.seq << " --------------------------\n";
    print_udp(this->package);
    this->bytes += size + HeadSize;
    return rst_byte;
}

DWORD WINAPI SendHandler(LPVOID param) {
    srand((unsigned)time(NULL));
    char* ReciBuffer = new char[PacketSize];
    Sender *sender = (Sender*)param;
    

    string name_size = sender->fileName + ":" + to_string(sender->fileSize);

    /* 遍历文件 窗口固定payloadSize*/
    char* iter = sender->FileBuffer;
    streamsize indx = 0;


    streamsize fileSize = sender->fileSize;
    int payload_size = sizeof(name_size);
    

    int seq = 0;
    bool status = 0;
    bool st = 1;
    bool fin = 0;

    clock_t send_st = clock();
    while (sender->send_runner_keep && !fin) {
        //发送seq状态0/1的包 
        if (st)
        {
         
            sender->package.set_status(status);
            sender->package.set_flag(st, fin);
            sender->package.packet_data(name_size.c_str(), payload_size);

            sender->_send(payload_size);
            //cout.write(sender->package.payload, sizeof(name_size));
            //cout << endl;
            st = false;
        }
       

        /*接受ack status!=  超时重传 */
        clock_t sec_st = clock();
        socklen_t dst_addr_len = sizeof(*sender->dst_addr);

        while (true) {
            while (recvfrom(sender->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)sender->dst_addr, &dst_addr_len) <= 0)
            {
                if (clock() - sec_st > MAX_TIME) {
                    cout << "\n-------------------- Time out  -----------------\n" << "Send again: send_status: " << status << " send_seq: " << seq << endl;
                    sender->_send(payload_size);
                    sec_st = clock();
   
                }
            }
            
            Udp* dst_package = (Udp*)ReciBuffer;
            if (
                (dst_package->cmp_cheksum() )
                && ( dst_package->header.get_status() == status)
                && (dst_package->header.get_Ack())
                )
            {

                
                //通过校验和、对应状态
                cout << "----- Check correct -----\n";
                if (dst_package->header.get_end()) {
                    fin = 1;
                    break;
                }

                if (!dst_package->header.get_st())
                    indx += PayloadSize;

                //状态转移
                status = !status;
                sender->package.header.set_Status(status);
                
            
                //打包下次发包
                sender->package.header.seq = ++seq;
                sender->package.header.ack = dst_package->header.seq + 1;
                
                if (indx + PayloadSize > fileSize) {
                   
                    payload_size = fileSize - indx;
                    sender->package.set_flag(0, 0, status, 1, 0, 1);
                    
                    //sender->_send(fileSize - indx);

                }

                else {     
                    payload_size = PayloadSize;
                    sender->package.set_flag(0, 0, status, 1, 0, 0);
                }

                sender->package.header.data_size = payload_size;
                sender->package.packet_data(iter + indx, payload_size);
                sender->_send(payload_size);

                break;
            }

            else {
                cout << "----- check error -----\n ";
                sender->_send(payload_size);
                cout << "Send again: send_status: " << status << "seq: " << seq << endl;
  
            }

            
        }

    }

    cout << "Sending Over\n";
    cout << "Total Length: " << sender->bytes << " bytes\n"<<"Duration: " << double((clock() - send_st)/ CLOCKS_PER_SEC)<<" secs"<<endl;
    
    if((clock() - send_st) / CLOCKS_PER_SEC)
        cout << "Speed Rate: " << double( sender->bytes / ((clock() - send_st) / CLOCKS_PER_SEC)) << " Bps"<<endl;
    return 0;
}

DWORD WINAPI SConnectHandler(LPVOID param) {
    srand((unsigned)time(NULL));
    char* ReciBuffer = new char[PacketSize];
    

    Sender* sender = (Sender*)param;
    socklen_t dst_addr_len = sizeof(*sender->dst_addr);
    
    bool SYN = 1;
    bool ACK = 0;

    bool succed = 0;
    


    clock_t sec_st = clock();

    while (!succed) {

        sender->package.set_flag(SYN, 0, ACK);
        sender->package.set_cheksum();
        sender->_send(0);

        while (recvfrom(sender->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)sender->dst_addr, &dst_addr_len) <= 0){
            
            if(clock() - sec_st > MAX_TIME) {
                cout << "\n-------------------- Time out  -----------------\n";
                sender->_send(0);
                sec_st = clock();
            }
            
        }

        Udp* dst_package = (Udp*)ReciBuffer;
        int dSYN = dst_package->header.get_Syn();
        int dACK = dst_package->header.get_Ack();
        if (
            (dst_package->cmp_cheksum())
            && (dst_package->header.get_Ack())
            )//校验和、ACK
        {
            cout << "\n------------------- Dst Package ---------------- \nSYN: " << dst_package->header.get_Syn() << " ACK: " << dst_package->header.get_Ack()<<endl;

           
            if (SYN && !ACK) {
                if (dSYN && dACK)
                {
                    SYN = 0;ACK = 1;
                    cout << "第二次挥手成功"<<endl;

                    succed = true;
                    cout << "Send Thread Ready" << endl;
                }
                else {
                    cout << "第二次挥手失败" << endl;
                    continue;
                }
                    
                   
            }
            else 
                {   //重新回1状态
                    SYN =1;ACK = 0;
                    cout << "重新挥手" << endl;
                }
                
            }     
    }
    sender->package.set_flag(0, 0, ACK);
    sender->package.set_cheksum();
    sender->_send(0);
    sender->connected = 1;
    return 1;
}

int Sender::get_connection() {

    this->dst_addr->sin_family = AF_INET;
    this->dst_addr->sin_port = htons(8999);
    inet_pton(AF_INET, ReciIp, &(this->dst_addr->sin_addr));
    cout << "Dst Prot: " << ntohs(this->dst_addr->sin_port)<<endl;
    //cout << "Dst addr " << inet_ntoa(dst_addr.sin_addr) << " : " << ntohs(dst_addr.sin_port)<<endl;

    /*三次握手*/
    
    cout << "Try to connect\n\n" << endl;
    CreateThread(NULL, 0,ConnectHandler, (LPVOID)this, 0, NULL);





    while (!this->connected) 
        Sleep(100);
    
    cout << "\n\nstart to send: " << endl;
    
    send_runner_handle = CreateThread(NULL, 0, SendHandler, (LPVOID)this, 0, NULL);


    return 1;
}