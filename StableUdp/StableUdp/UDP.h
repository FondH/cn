#pragma once
#include<iostream>
#include <fstream>
#include <fstream>
#define BufferSize 20 * 1024 * 1024 
#define HeadSize sizeof(Header)
#define PayloadSize (16 * 1024)
#define PacketSize HeadSize+PayloadSize


#define ReciIp "127.0.0.1"
#define ReciPort 8999

using streamsize = long long;
using namespace std;
struct Header {
    uint16_t flag;
    // uint8_t code;
    uint16_t checksum;
    uint32_t seq;
    uint32_t ack;
    uint32_t data_size;
    // uint16_t headerLen;

    Header() : flag(0), checksum(0), seq(0), ack(0), data_size(0) {}
    void init() { this->flag = this->checksum = this->seq = this->ack = this->data_size = 0; }
    void set_bit(int f, bool value) {
        value ? this->flag |= (uint16_t)1 << f : flag &= ~((uint16_t)1 << f);
    }
    void set_St(bool va) { set_bit(5, va); }
    void set_End(bool va) { set_bit(4, va); }
    void set_Status(bool va) { set_bit(3, va); }
    void set_Syn(bool va) { set_bit(2, va); }
    void set_Fin(bool va) { set_bit(1, va); }
    void set_Ack(bool va) { set_bit(0, va); }

    void set_seq(int s) { seq = s; }
    void set_ack(int a) { ack = a; }
    void set_size(int s) { data_size = s; }
    void set_chekcum(int c) { checksum = c; }


    int get_st() { return (flag & (0x20)) > 0 ? 1 : 0; }
    int get_end() { return (flag & (0x10)) > 0 ? 1 : 0; }
    int get_status() { return (flag & (0x8)) > 0 ? 1 : 0; }
    int get_Syn() { return (flag & (0x4)) > 0 ? 1 : 0; }
    int get_Fin() { return (flag & (0x2)) > 0 ? 1 : 0; }
    int get_Ack() { return (flag & (0x1)) > 0 ? 1 : 0; }
};

class Udp {

public:
    Header  header;
    char payload[PayloadSize] = { '0' };
    Udp() {
        header = Header();

    }
    Udp(Header& h, char* c) {
        this->header = h;
        memcpy(payload, c, PayloadSize);
    }
    void set(Header& h, char* c) {
        this->header = h;
        memcpy(payload, c, PayloadSize);
    }
    void packet_data(const char* c, int size) {
        //size 数据段大小
        //header.set_Ack(1);
        this->set_cheksum();
        this->header.set_size(size);
        memset(this->payload, 0, PayloadSize);
        memcpy(this->payload, c, size);
        
    }
    void set_flag(bool SYN, bool Fin, bool ACK) {
        this->header.set_Syn(SYN);
        this->header.set_Fin(Fin);
        this->header.set_Ack(ACK);
    }
    void set_status(bool STATUS) {
        this->header.set_Status(STATUS);
        this->header.set_Ack(1);
    }
    void set_flag(bool SYN, bool Fin, bool STATUS, bool ACK, bool St, bool Ed) {
        set_flag(SYN, Fin, ACK);
        this->header.set_Status(STATUS);
        this->header.set_St(St);
        this->header.set_End(Ed);
    }
    void set_flag(bool St, bool Ed) {
        this->header.set_St(St);
        this->header.set_End(Ed);
    }
    void set_cheksum() {
        this->header.checksum = ~calc_cheksum();
    }

    uint16_t calc_cheksum() {
        /*ToDo*/
        return 0;
    }
    bool cmp_cheksum() {
        uint16_t ut = calc_cheksum();
        return (ut | header.checksum) == 0xffff;
    }
    ~Udp() {

    }
};

void print_udp(Udp& u) {
    Header header = u.header;
    cout << "Flag: " << hex << header.flag;
    cout << dec
        << ", Checksum: " << header.checksum
        << ", Sequence: " << header.seq
        << ", Acknowledgment: " << header.ack
        << ", Data Size: " << header.data_size
        << endl;
}

//输出到dst文件
void Out2file(const char* src, const streamsize& size, const string& dst_name) {
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
    float ksize = (float)size / PayloadSize * 4;
    cout << "File: " << name << " is Loading\n" << "Size: " << ksize << " KB" << endl;
    file.close();
    return size;
}