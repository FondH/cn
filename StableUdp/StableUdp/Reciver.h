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
using namespace std;


const int MAX_DELAY_MS = 100; // ����ӳ�ʱ�䣨���룩
const double MAX_DELAY_RATE = 0.1;


void SimulateDelay(bool IsDelay) {
    if (IsDelay)
        Sleep(MAX_DELAY_MS);

}


class Reciver {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in* reci_addr = new sockaddr_in();
    sockaddr_in* send_addr = new sockaddr_in();

    Udp package = Udp();

    char* SendBuffer = new char[PacketSize];
    char* FileBuffer = new  char[BufferSize];
    string InforBuffer = "";
    streamsize fileSize = 0;

    HANDLE reci_runner_handle = NULL;
    volatile bool connected = false;
    volatile bool reci_runner_keep = true;

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
        this->package.set_cheksum();
        char* threadBuffer = new char[PacketSize];
        memcpy(threadBuffer, &this->package, HeadSize);

        thread send_thread(&Reciver::_send, this, threadBuffer, IsDelay);
        send_thread.detach();
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
    cout << "\n\n+--+--+- Reciver'Packetage +--+--+--+--+\n";
    //print_udp(this->package);

    cout << "|  SYN: " << this->package.header.get_Syn()
        << " ACK " << this->package.header.get_Ack()
        << "ST: " << this->package.header.get_st()
        << " ED: " << this->package.header.get_end()
        << " STATUS: " << this->package.header.get_status() << "   |" << endl;
    cout << "+--+--+--+--+- SEQ: " << this->package.header.seq << " +--+--+--+--+--+\n\n";

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
            ���   get_status() ����0/1
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
                //fin�����յ�������ȫ�յ��ˣ��ظ�һ��fin���ģ���sender�����������ֵڶ���
                fin = 1;


            //���
            reci->package.header.set_Ack(1);
            reci->package.header.set_St(st);
            reci->package.header.set_End(fin);
            reci->package.set_flag(wait_ack);
            reci->package.header.seq = seq++;
            reci->package.header.ack = package->header.seq + 1;

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



    clock_t sec_st = clock();

    while (!succed) {


        while (recvfrom(reci->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)reci->send_addr, &send_addr_len) <= 0) {

            /* if (clock() - sec_st > MAX_TIME) {
                  cout << "-------- Time out --------\n";
                  sender->_send(0);
                  sec_st = clock();
              }*/
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
                    reci->package.set_flag(SYN, 0, 1);
                    reci->ThreadSend();
                    cout << "��һ�λ��ֳɹ�" << endl;
                }
                else {
                    cout << "��һ�λ���ʧ��" << endl;
                    continue;
                }
            }
            else {
                if (!dSYN && dACK) {
                    succed = true;
                    cout << "���������ֳɹ�" << endl;
                    cout << "Recive Thread ready" << endl;
                    reci->package.set_flag(0, 0, 1);
                }

            }

        }

    }
    reci->connected = 1;
    return 1;
}


int Reciver::get_connection() {
    /*�ȴ����� reci*/

    cout << "wait..." << endl;

    /*��������  �ظ��ڶ���*/

    CreateThread(NULL, 0, ConnectHandler, (LPVOID)this, 0, NULL);


    while (!this->connected)
        Sleep(100);
    cout << "\n\nstart to reci !" << endl;
    reci_runner_handle = CreateThread(NULL, 0, ReciHandler, (LPVOID)this, 0, NULL);

    return 1;
}


int Udp_main() {
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
        reciver.get_connection();
        cin >> cin_buffer;
        if (cin_buffer == "q")
            break;
    }
    return 0;
}