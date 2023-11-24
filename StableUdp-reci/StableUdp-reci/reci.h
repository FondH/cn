#pragma once
#include <winsock2.h>
#include<WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include<iomanip>
#include<stdlib.h>
#include<vector>
#include <windows.h>
#include<thread>
#include "UDP.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WindowLen 32

using namespace std;

const int MAX_DELAY_MS = 300; // 最大延迟时间（毫秒）
const double MAX_DELAY_RATE = 0.02;
const double DEFAULT_DELAY_Time= 50;


void SimulateDelay(bool IsDelay) {
    if (IsDelay)
        Sleep(MAX_DELAY_MS);
    else
        Sleep(DEFAULT_DELAY_Time);
}


class Reciver {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in* reci_addr = new sockaddr_in();
    sockaddr_in* send_addr = new sockaddr_in();

    Udp* package = new Udp();

    char* SendBuffer = new char[PacketSize];
    char* FileBuffer = new  char[BufferSize];
    string InforBuffer = "";
    streamsize fileSize = 0;

    HANDLE reci_runner_handle = NULL;
    volatile bool connected = false;
    volatile bool reci_runner_keep = true;

    int window = WindowLen;
    int bytes = 0; //sum_of sended bytes
public:
    Reciver();
    void _send(char* tb, bool IsDelay);
    void ThreadSend() {
        bool IsDelay = 0;
        double i = rand() / (double)RAND_MAX;
        if (i < MAX_DELAY_RATE)
            IsDelay = 1;
        //cout << "\n!! " << i <<" " << IsDelay << "!!\n";
        this->package->set_cheksum();
        char* threadBuffer = new char[HeadSize];
        memcpy(threadBuffer, this->package, HeadSize);

        thread send_thread(&Reciver::_send, this, threadBuffer, IsDelay);
        send_thread.detach();

    }
    int get_connection();
    void to_file();
    friend DWORD WINAPI ReciHandler(LPVOID param);
    friend DWORD WINAPI ConnectHandler(LPVOID param);
    friend DWORD WINAPI GBNReciHandler(LPVOID param);
    friend DWORD WINAPI SRReciHandler(LPVOID param);
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
    void init() {

        memset(this->FileBuffer, 0, BufferSize);
        InforBuffer = "";
        fileSize = 0;
        connected = false;
        reci_runner_keep = true;
        bytes = 0;
    }

};
Reciver::Reciver() {


    reci_addr->sin_family = AF_INET;
    reci_addr->sin_port = htons(8999);
    inet_pton(AF_INET, ReciIp, &(reci_addr->sin_addr));

    int iResult = bind(s, (struct sockaddr*)this->reci_addr, sizeof(*reci_addr));
    if (iResult != 0)
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
}
void Reciver::_send(char* tb, bool IsDelay) {

    //this->package.set_cheksum();
    //memcpy(tb, &package, HeadSize);
    //cout << "\n\n+--+--+- Reciver'Packetage +--+--+--+--+\n";
    print_udp(*this->package, 2);
    Udp* pp = (Udp*)tb;

    /*  cout << "|  SYN: " << pp->header.get_Syn()
          << " ACK " << pp->header.get_Ack()
          << "ST: " << pp->header.get_st()
          << " ED: " << pp->header.get_end()
          << " STATUS: " << pp->header.get_status() << "   |" << endl;
      cout << "+--+--+--+--+- SEQ: " << pp->header.seq << " +--+--+--+--+--+\n\n";*/

    this->bytes += HeadSize;


    SimulateDelay(IsDelay);

    int rst_byte = sendto(this->s, (const char*)tb, HeadSize, 0, (struct sockaddr*)this->send_addr, sizeof(*(this->send_addr)));
    char error_message[100];
    if (rst_byte == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        cerr << "sendto failed with error: " << error_code << endl;
        return;
    }

    return;
}
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
    tokens[0] = "(reci)" + tokens[0];
    Out2file(this->FileBuffer, stoi(tokens[1]), tokens[0]);

}
DWORD WINAPI ReciHandler(LPVOID param) {
    cout << "Start Acc the file\n";
    Reciver* reci = (Reciver*)param;
    srand((unsigned)time(NULL));


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
            cout << "\n+--------- Sender' Package ---------+\n";
            //print_udp(*package);
            cout << "|         ST: " << package->header.get_st() <<
                " ED: " << package->header.get_end() <<
                " STATUS: " << package->header.get_status() <<
                "     |" << endl;
            cout << "+------------- SEQ: " << package->header.seq << " -------------+\n";


            if (!package->cmp_cheksum() || !(package->header.get_status() == wait_ack))
            {
                cout << "---------- Time Out or CheckSum error ---------\n" << endl;
                reci->ThreadSend();
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
            reci->package->header.set_Ack(1);
            reci->package->header.set_St(st);
            reci->package->header.set_End(fin);
            reci->package->set_flag(wait_ack);
            reci->package->header.seq = seq++;
            reci->package->header.ack = package->header.seq + 1;

            reci->ThreadSend();
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
    memset(ReciBuffer, 0, PacketSize);

    Reciver* reci = (Reciver*)param;
    socklen_t send_addr_len = sizeof(*reci->send_addr);

    bool SYN = 0;
    bool succed = 0;


    while (!succed) {


        while (recvfrom(reci->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)reci->send_addr, &send_addr_len) <= 0) {

            continue;

        }

        Udp* dst_package = (Udp*)ReciBuffer;
        int dSYN = dst_package->header.get_Syn();
        int dACK = dst_package->header.get_Ack();
        if (dst_package->cmp_cheksum())
        {
            cout << "------------------- Dst Package ---------------- \nSYN: " << dst_package->header.get_Syn() << " ACK: " << dst_package->header.get_Ack() << endl;


            if (!SYN) {
                if (dSYN && !dACK)
                {
                    SYN = 1;
                    reci->package->set_flag(SYN, 0, 1);
                    reci->ThreadSend();
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
                    reci->package->set_flag(0, 0, 1);
                }

            }

        }

    }
    reci->connected = 1;
    return 1;
}

DWORD WINAPI GBNReciHandler(LPVOID param) {
    cout << "GBN Start to Acc the file\n";
    Reciver* reci = (Reciver*)param;
    srand((unsigned)time(NULL));

    char* buffer = new char[PacketSize];


    bool fin = 0;
    bool st = 1;
    //bool RE = 0;
    int expect_ack = 1;
    int seq = 0;

    int rst_byte = -1;
    char* iter = reci->FileBuffer;

    int send_addr_size = sizeof(*reci->send_addr);
    clock_t send_st = clock();
    while (reci->reci_runner_keep && !fin) {


        if (fin || recvfrom(reci->s, buffer, PacketSize, 0, (sockaddr*)reci->send_addr, &send_addr_size) <= 0)
        {

            continue;
        }
        Udp* dst_package = (Udp*)buffer;
        print_udp(*dst_package, 1);
        //cout << "\n+--------- Sender' Package ---------+\n";

        if (!dst_package->cmp_cheksum() || !(dst_package->header.seq + 1 == expect_ack))
        {
            //cout << "---------- Lost or CheckSum error ---------\n" << endl;
            reci->package->header.set_r(1);
            reci->ThreadSend();
            reci->package->header.set_r(0);

            continue;

        }
        // 通过校验和 

        if (dst_package->header.get_st())
            reci->InforBuffer += dst_package->payload;

        else {
            st = 0;
            memcpy(iter, dst_package->payload, dst_package->header.data_size);
            iter += dst_package->header.data_size;
        }

        if (dst_package->header.get_Fin())
            //fin报文收到后，则完全收到了，回复一个fin报文，让sender结束。即挥手第二次
            fin = 1;

        reci->package->header.set_Ack(1);
        reci->package->header.set_St(st);
        reci->package->header.set_Fin(fin);
        reci->package->header.seq = seq++;
        reci->package->header.ack = expect_ack++;

        reci->ThreadSend();

    }

    Sleep(100);
    cout << "Thread exit... \nTotal Length: " << reci->bytes << " bytes\n" << "Duration: " << (double)((clock() - send_st) / CLOCKS_PER_SEC) << " secs" << endl;
    cout << "Speed Rate: " << (double)reci->bytes / ((clock() - send_st) / CLOCKS_PER_SEC) << " Bps" << endl;
    reci->to_file();
    return 1;

}

DWORD WINAPI SRReciHandler(LPVOID param) {
    cout << "SR Start to Acc the file\n";
    Reciver* reci = (Reciver*)param;
    srand((unsigned)time(NULL));

    char* buffer = new char[PacketSize];


    bool fin = 0;
    bool st = 1;
    //int expect_ack = 1;
    int seq = 0;
    int base = 0;
    int N = reci->window;

    rQueue qbuffer = rQueue(N);
    int rst_byte = -1;
    char* iter = reci->FileBuffer;
    int shift = -1;
    int send_addr_size = sizeof(*reci->send_addr);
    clock_t send_st = clock();
    while (reci->reci_runner_keep && !fin) {


        if (fin || recvfrom(reci->s, buffer, PacketSize, 0, (sockaddr*)reci->send_addr, &send_addr_size) <= 0)
        {

            continue;
        }
        Udp* dst_package = (Udp*)buffer;
        print_udp(*dst_package, 1);
        int dst_seq = dst_package->header.seq;
        if (!dst_package->cmp_cheksum() || (dst_seq >= base + N))
        {

            continue;
        }
        // 通过校验和 在窗口内

        if (dst_seq < base)
        {
            reci->package->header.set_r(1);
            reci->ThreadSend();
            reci->package->header.set_r(0);
            continue;
        }

        if (dst_package->header.get_st()) {
            reci->InforBuffer += dst_package->payload;
            base++;
        }
        else {
            qbuffer.push(dst_package);
            cout << dst_package->header.seq << " buffer_size: " << qbuffer.CurrNum;
            st = 0;
            if (dst_seq == base) {
                shift = qbuffer.rpop(&iter);
                 base += shift;
                 cout << " window --> " << shift << " base " << base << endl;

            }
                
            
        }


        if (dst_package->header.get_Fin())
            //fin报文收到后，则完全收到了，回复一个fin报文，让sender结束。即挥手第二次
            fin = 1;
        reci->package->header.set_Ack(1);
        reci->package->header.set_St(st);
        reci->package->header.set_Fin(fin);
        reci->package->header.seq = seq++;
        if (reci->package->header.ack == 2)
            cout << 2;
        reci->package->header.ack = dst_package->header.seq + 1;

        reci->ThreadSend();

    }

    Sleep(100);
    cout << "Thread exit... \nTotal Length: " << reci->bytes << " bytes\n" << "Duration: " << (double)((clock() - send_st) / CLOCKS_PER_SEC) << " secs" << endl;
    cout << "Speed Rate: " << (double)reci->bytes / ((clock() - send_st) / CLOCKS_PER_SEC) << " Bps" << endl;
    reci->to_file();
    return 1;

}
int Reciver::get_connection() {
    /*等待连接 reci*/

    cout << "wait..." << endl;

    /*三次握手  回复第二次*/

    CreateThread(NULL, 0, ConnectHandler, (LPVOID)this, 0, NULL);


    while (!this->connected)
        Sleep(100);

    //非阻塞态
    unsigned long mode = 1;
    ioctlsocket(this->s, FIONBIO, &mode);

    cout << "\n\nstart to reci !" << endl;
    reci_runner_handle = CreateThread(NULL, 0, GBNReciHandler, (LPVOID)this, 0, NULL);

    return 1;
}


