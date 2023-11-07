#include <winsock2.h>
#include<WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <string>
#include <time.h>
#include<iomanip>
#pragma comment(lib, "Ws2_32.lib")

#define BufferSize 20 * 1024 * 1024 
#define HeadSize sizeof(Header)
#define PayloadSize (4 * 1024)
#define PacketSize HeadSize+PayloadSize

#define MAX_TIME 0.2 * CLOCKS_PER_SEC 
#define Ip "127.0.0.1"
#define ReciPort 8999
#define testFile "test.jpg"
#define outputFile "out.jpg"

#define End 0x10

using namespace std;
using streamsize = long long;

//输出到dst文件
void Out2file(char* src, streamsize& size, string& dst_name) {
    ofstream outFile(dst_name, ifstream::binary);
    // cout << src;
    outFile.write(src, size);
    if (!outFile.good())
        cerr << "Write Defeted" << endl;
    outFile.close();

}
//读取至buffer
streamsize ReadFile(string name, char* buffer) {

    ifstream file(name, ifstream::binary);
    if (!file) {
        cerr << "Error:" << name << " Open Defeted ! " << endl;
        return 0;
    }
    file.seekg(0, ifstream::end);
    streamsize size = file.tellg();
    file.seekg(0);

    if (!file.read(buffer, size)) {
        cerr << "Error:" << name << " Read Defeted !" << endl;
        return 0;
    }
    float ksize = (float)size / PayloadSize * 4 ;
    cout << "File: " << name << " is Loading\n" << "Size: " << ksize << " KB" << endl;
    file.close();
    return size;
}


struct Header {
    uint16_t flag;
  // uint8_t code;
    uint16_t checksum;
    uint32_t seq;
    uint32_t ack;
    uint32_t data_size;
   // uint16_t headerLen;

    Header() : flag(0), checksum(0), seq(0), ack(0), data_size(0) {}
    void set_bit(int f, bool value) {
        value ? flag |= (uint16_t)1 << f : flag &= ~((uint16_t)1 << f);
    }
    void set_end(bool va) { set_bit(4, va);}
    void set_status(bool va) { set_bit(3, va); }
    void set_Syn(bool va) { set_bit(2, va); }
    void set_Fin(bool va) { set_bit(1, va); }
    void set_Ack(bool va) { set_bit(0, va); }

    void set_seq(int s) { seq = s; }
    void set_ack(int a) { ack = a; }
    void set_size(int s) { data_size = s; }
    void set_chekcum(int c) { checksum = c; }

    int get_end() { return flag & (0x10); }
    int get_status() { return flag & (0x8); }
    int get_Syn() { return flag & (0x4); }
    int get_Fin() { return flag & (0x2); }
    int get_Ack() { return flag & (0x1); }
};

class Udp {

public:
    Header  header;
    char* payload;
    Udp(){
        header = Header();
        payload = new char[PayloadSize];
    }
    Udp(Header & h, char* c) {
        header = h;
        payload = new char[PayloadSize];
        memcpy(payload, c, PayloadSize);
    }
    void set(Header& h, char* c) {
        header = h;
        payload = new char[PayloadSize];
        memcpy(payload, c, PayloadSize);
    }
    void packet_data(char* c, int size){
        memcpy(payload, c, size);
    }
    
    ~Udp() {
        delete payload;
    }
};
void print_udp(Udp & u) {
    Header header = u.header;
    cout << "Flag: " << hex <<header.flag
        << ", Checksum: " << header.checksum
        << ", Sequence: " << header.seq
        << ", Acknowledgment: " << header.ack
        << ", Data Size: " << header.data_size
        << endl;
}

class Sender {
private:
    WSADATA wsaData;
    SOCKET s;
    sockaddr_in dst_addr;
    char* SendBuffer = new char[PacketSize];
    Udp package = Udp();
    
    char* FileBuffer;
    streamsize fileSize=0;

    HANDLE send_runner_handle;
    volatile bool send_runner_keep = true;



public:
    Sender();

    int _send() { return sendto(s, SendBuffer, PacketSize, 0,(struct sockaddr*)&dst_addr, sizeof(dst_addr)); }
    int get_connection();
    int set_flag(bool seq, bool ack, bool syn);
    int set_seq_ack(int seq, int ack) {
        package.header.set_seq(seq);
        package.header.set_ack(ack);
    };
    
    void print_info() { print_udp(package); }
    int set_size(int s) { package.header.set_size(s); }
    void GetFile(string name) { fileSize = ReadFile(name,FileBuffer); };
    
    friend DWORD WINAPI SendHandler(LPVOID param);
    ~Sender();
};
Sender::Sender() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        cerr << "Ws2_32 Initial Error" << endl;
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(ReciPort);
    inet_pton(AF_INET, Ip, &dst_addr.sin_addr);
    FileBuffer = new char[BufferSize];
    
  
}
Sender::~Sender() {
    closesocket(s);
    WSACleanup();
}


DWORD WINAPI SendHandler(LPVOID param) {
    cout << "Start Send" << endl;
    char* ReciBuffer = new char[PacketSize];
    Sender sender = *(Sender*)param;

    char* iter = sender.FileBuffer;
    streamsize indx = 0;
    streamsize fileSize = sender.fileSize;
    int package_size = PayloadSize;

    
    int seq = 0;
    bool send_seq = 0;
    //bool wait_ack = 0;
    bool fin = 0;
    while (sender.send_runner_keep || fin) {
        //发送seq状态0/1的包 header
        sender.package.header.set_status(send_seq);
        
        if (indx+PayloadSize > fileSize) {
            package_size = fileSize - indx;
            fin = true;
            sender.package.header.set_end(1);
        }

        sender.package.packet_data(iter + indx, package_size);
        int rst_byte = sender._send();
        cout << "No." << seq << ": \nindex: " << indx << "  fileSize: " << package_size << "  send_seq: " << send_seq << endl;
        cout << "The Udp Header Mess: \n";
        sender.print_info();


        clock_t sec_st = clock();
        /*接受ack status!=  超时重传 */
        socklen_t dst_addr_len = sizeof(sender.dst_addr);
        while (true) {
            while (recvfrom(sender.s, ReciBuffer, PacketSize, 0, (struct sockaddr*)&sender.dst_addr, &dst_addr_len) <= 0)
            {
                if (clock() - sec_st > MAX_TIME) {
                    cout << "Time out" << endl;
                    sender._send();

                }
            }
            Udp* dst_package = (Udp*)ReciBuffer;
            if (
                (sender.package.header.checksum | dst_package->header.checksum) == 0xffff 
                && (sender.package.header.get_status()==send_seq )
                && (dst_package->header.get_Ack())
                ) 
            {
                print_udp(*dst_package);
                break;
            }

        }

        seq++;
        indx += PayloadSize;
        send_seq = !send_seq;
    }
    return 0;
}
int Sender::get_connection() {

    /*三次握手*/
    cout << "Sender is Ready: " << endl;


    send_runner_handle = CreateThread(NULL, 0, SendHandler, (LPVOID)this, 0, NULL);


    //cout << "Sender has left! " << endl;
    return 1;
}

int main(){
    string filename = "test.txt";
    string outFile = "out.txt";
    //char* buffer = new char[BufferSize];
    Sender sender = Sender();
    sender.GetFile(filename);
    sender.get_connection();


    //Out2file(buffer, filesize, outFile);
    
}