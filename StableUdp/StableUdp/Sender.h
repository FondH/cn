#pragma once
#include <winsock2.h>
#include<WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <string>
#include <time.h>
#include<iomanip>
#include "UDP.h"
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

#define MAX_TIME 10 * CLOCKS_PER_SEC 

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

public:
    Sender();

    int _send(int size) {

        memcpy(this->SendBuffer, &this->package, size + HeadSize);
        int rst_byte  = sendto(this->s, this->SendBuffer, size + HeadSize, 0, (sockaddr*)this->dst_addr, sizeof(*this->dst_addr));
        char error_message[100];
        if (rst_byte == SOCKET_ERROR) {
            int error_code = WSAGetLastError();
            cerr << "sendto failed with error: " << error_code << endl;
            return rst_byte;
        }
       
        cout << "------------------ SEQ: "<< this->package.header.seq<<" -------------------\n";
        print_udp(this->package);
       
        return rst_byte;
    }
    int get_connection();
    int set_flag(bool seq, bool ack, bool syn);
    int set_seq_ack(int seq, int ack) {
        this->package.header.set_seq(seq);
        this->package.header.set_ack(ack);
    };
    void print_info() { print_udp(package); }
    int set_size(int s) { package.header.set_size(s); }
    void GetFile(string name) { 
        fileSize = ReadFile(name, FileBuffer);
    };

    friend DWORD WINAPI SendHandler(LPVOID param);
    ~Sender();
};
Sender::Sender() {

   
    FileBuffer = new char[BufferSize];
    send_runner_handle = NULL;

}
Sender::~Sender() {
    closesocket(s);
    send_runner_keep = false;
}


DWORD WINAPI SendHandler(LPVOID param) {
 
    char* ReciBuffer = new char[PacketSize];
    Sender *sender = (Sender*)param;
    

    string name_size = sender->fileName + ":" + to_string(sender->fileSize);

    /* �����ļ� ���ڹ̶�patloadSize*/
    char* iter = sender->FileBuffer;
    streamsize indx = 0;


    streamsize fileSize = sender->fileSize;
    int payload_size = sizeof(name_size);


    int seq = 0;
    bool status = 0;
    bool st = 1;
    bool fin = 0;
    while (sender->send_runner_keep && !fin) {
        //����seq״̬0/1�İ� 
        if (st)
        {
         
            sender->package.set_status(status);
            sender->package.set_flag(st, fin);
            sender->package.packet_data(name_size.c_str(), sizeof(name_size));

            sender->_send(payload_size);
            //cout.write(sender->package.payload, sizeof(name_size));
            //cout << endl;
            st = false;
        }
       

        /*����ack status!=  ��ʱ�ش� */
        clock_t sec_st = clock();
        socklen_t dst_addr_len = sizeof(*sender->dst_addr);

        while (true) {
            while (recvfrom(sender->s, ReciBuffer, PacketSize, 0, (struct sockaddr*)sender->dst_addr, &dst_addr_len) <= 0)
            {
                if (clock() - sec_st > MAX_TIME) {
                    cout << "----- Time out -----\n" << "Send again: send_status: " << status << " send_seq: " << seq << endl;
                    sender->_send(payload_size);
                    sec_st = clock();
   
                }
            }
            
            Udp* dst_package = (Udp*)ReciBuffer;
            if (
                (sender->package.cmp_cheksum() )
                && (sender->package.header.get_status() == status)
                && (dst_package->header.get_Ack())
                )
            {

                
                //ͨ��У��͡���Ӧ״̬
                cout << "----- Check correct -----\n";
                if (dst_package->header.get_end()) {
                    fin = 1;
                    break;
                }

                if (!dst_package->header.get_st())
                    indx += PayloadSize;

                //״̬ת��
                status = !status;
                sender->package.header.set_Status(status);
                
            
                //����´η���
                sender->package.header.seq = ++seq;
                sender->package.header.ack = dst_package->header.seq + 1;
                
                if (indx + PayloadSize > fileSize) {
                   
                    payload_size = fileSize - indx;
                    sender->package.set_flag(0, 0, status, 1, 0, 1);
                    
                    //sender->_send(fileSize - indx);

                }

                else {     
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

    /*���λ���*/
    cout << "�߳��˳�\n";
    return 0;
}
int Sender::get_connection() {

    this->dst_addr->sin_family = AF_INET;
    this->dst_addr->sin_port = htons(8999);
    inet_pton(AF_INET, ReciIp, &(this->dst_addr->sin_addr));
   
    //cout << "Dst addr " << inet_ntoa(dst_addr.sin_addr) << " : " << ntohs(dst_addr.sin_port)<<endl;

    /*��������*/
    

    cout << "Sender is Ready: " << endl;

    Sender* tp = (Sender*)this;
    send_runner_handle = CreateThread(NULL, 0, SendHandler, (LPVOID)this, 0, NULL);


    return 1;
}