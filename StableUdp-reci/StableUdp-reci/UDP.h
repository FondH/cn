#pragma once
#include<iostream>
#include <fstream>
#include <vector>
#include<unordered_map>
#include <memory>
#include <algorithm>
#define ReciIp "127.0.0.1"
#define ReciPort 8999

#define BufferSize 20 * 1024 * 1024 
#define HeadSize sizeof(Header)
#define PayloadSize (16 * 1024)
#define PacketSize HeadSize+PayloadSize
using streamsize = long long;
using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
struct Header {
    uint16_t flag;
    // uint8_t code;
    uint16_t checksum;
    uint32_t seq;
    uint32_t ack;
    uint32_t data_size;
    // uint16_t headerLen;

    Header() : flag(0), checksum(0), seq(0), ack(0), data_size(0) {}
    Header(bool ST, bool ACK, bool Fin) {
        this->init();
        this->set_St(ST);
        this->set_Ack(ACK);
        this->set_Fin(Fin);

    }
    void init() { this->flag = this->checksum = this->seq = this->ack = this->data_size = 0; }
    void set_bit(int f, bool value) {
        value ? this->flag |= (uint16_t)1 << f : flag &= ~((uint16_t)1 << f);
    }
    void set_r(bool va) { set_bit(6, va); }
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


    int get_r() { return (flag & (0x40)) > 0 ? 1 : 0; }
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
    Udp(bool ST, bool ACK, bool Fin, int seq, int ack) {

        this->header = Header(ST, ACK, Fin);
        this->header.set_seq(seq);
        this->header.set_ack(ack);
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
    void set_flag(bool STATUS) {
        this->header.set_Status(STATUS);
        this->header.set_Ack(1);
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
        int a = calc_cheksum();
        this->header.checksum = ~calc_cheksum();
    }

    uint16_t calc_cheksum() {
        /*ToDo*/
        int count = (PacketSize) / 2;
        uint16_t* buf = (uint16_t*)this;
        int res = 0;
        while (count--) {
            res += *buf++;
            if (res & 0x10000) {
                res &= 0xffff;
                res += 1;
            }
        }
        return (res & 0xffff);

    }
    bool cmp_cheksum() {
        uint16_t ut = calc_cheksum();
        return (ut | header.checksum);
    }
    ~Udp() {

    }
};

void print_udp(Udp& u, int type) {
    Header header = u.header;
    int t;
    if (type == 1) {
        t = BACKGROUND_BLUE;
        SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE);
        cout << "Send: ";
    }
    else if (type == 2) {
        t = BACKGROUND_BLUE;
        SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN);
        cout << "Reci: ";
    }
    else if (type == 3) {
        t = BACKGROUND_INTENSITY;
        SetConsoleTextAttribute(hConsole, BACKGROUND_INTENSITY);
        cout << "Invalid: ";
    }

    string out = "[";


    if (header.get_Syn()) {
        out += "SYN";
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    }


    else if (header.get_st()) {
        out += "ST";
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    }
    else if (header.get_end()) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        out += "END";
    }
    else if (header.get_Fin()) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        out += "FIN";
    }
    else {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        out += "STREAM";
    }

    if (header.get_r()) {
        out += " Re";
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
    }

    if (header.get_Ack())
        out += " ACK";

    out += "]";

    cout << out;
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << " Flag: " << hex << header.flag;
    cout << dec
        << ", Checksum: " << header.checksum
        << ", Sequence: " << header.seq
        << ", Acknowledgment: " << header.ack
        << ", Data Size: " << header.data_size
        << endl;
}


void print_udp(Udp& u) {
    print_udp(u, 0);
}



class pQueue {
private:

    int CurrNum;
    int MaxNum;
    friend class Sender;

public:
    vector<bool> waitAck;
    vector<Udp*> data;
    pQueue(int MaxNum) : MaxNum(MaxNum), CurrNum(0) { }
    bool push(Udp* value) {

        if (CurrNum >= MaxNum)
            return 0;
        data.push_back(value);
        waitAck.push_back(0);
        CurrNum++;
        return 1;

    }

    void pop() {
        if (!data.empty()) {
            data.erase(data.begin());
            waitAck.erase(waitAck.begin());
            this->CurrNum--;
        }
    }


    //return  -1 ACK小于窗口内最小值 >1 ACK大于循环第一个元素pop
    int GBNpop(const Udp& value) {
        int va = value.header.ack;
        if (this->empty() || va < (this->front()->header.seq + 1))
            return -1;
        //注: va大于front， 表示回复的ack丢失或者延迟，但是接受方已经收到，可以累计确认
        else {
            int rst = va - this->front()->header.seq;
            int n = rst;
            while (n--)
                this->pop();
            return rst;
        }

    }


    // return n 窗口移动n位  0 ack确认，窗口未移动  -1未Ack成功
    int SRpop(const Udp& value) {
        int indx = 0;
        while (indx < MaxNum) {
            if (value.header.ack == data[indx]->header.seq + 1)
                break;
            indx++;
        }
        if (indx == MaxNum)
            return -1;

        this->waitAck[indx] = 1;
        // return indx;
        if (!indx) {
            while (indx < MaxNum && this->waitAck[indx]) {
                this->pop();
                indx++;
            }
            return indx;
        }

        return 0;
    }

    //队头
    Udp* front() {
        if (this->empty())
            return nullptr;
        return this->data.front();
    }
    Udp* end() {
        if (this->empty())
            return nullptr;
        return this->data.back();
    }
    bool empty() {
        return this->data.empty();
    }

};
bool compareUdp(Udp* a, Udp* b) {
    return a->header.seq < b->header.seq;
}
class rQueue {
public:
    int CurrNum;
    int MaxNum;
    vector<Udp*> data;
    rQueue(int MaxNum) : MaxNum(MaxNum), CurrNum(0) { }
    //保证从小到大  return 0： 成功；  -1： 重复
    int push(Udp* u) {
        Udp* value = new Udp();
        value->header = u->header;
        memcpy(value->payload, u->payload, PayloadSize);

        int nSeq = value->header.seq;
        if (CurrNum >= MaxNum)
            return 1;

        auto it = lower_bound(data.begin(), data.end(), value,
            [](const Udp* a, const Udp* b) {
            return a->header.seq < b->header.seq;
        });

        if (it != data.end() && (*it)->header.seq == nSeq) {
            return -1; 
        }

        data.insert(it, value); // 在找到的位置插入新元素
        CurrNum++;
        return 0;
    
    }
    int rpop(char** iter) {
        int indx = 0;
        while (indx< data.size()) {

            

            //连续的区间写入
            memcpy(*iter, data[indx]->payload, data[indx]->header.data_size);
            *iter += data[indx]->header.data_size;
            
            if (data.size() == 1)
                break;
            // 检查下一个元素是否存在且序列连续
            if (indx + 1 < data.size() && data[indx]->header.seq + 1 != data[indx + 1]->header.seq) {
                break;
            }
            if (indx + 1 == data.size())
                break;
            indx++;
        }
        for(int i=0;i<=indx;i++)
            this->pop();

        return indx+1;

    }
    void pop() {
        if (!data.empty()) {
            data.erase(data.begin());
            this->CurrNum--;
            
        }
    }


};

//输出到dst文件
void Out2file(const char* src, const streamsize& size, const string& dst_name) {
    ofstream outFile(dst_name, ifstream::binary);
    // cout << src;
    outFile.write(src, size);
    if (!outFile.good())
        cerr << "Write Defeted" << endl;
    outFile.close();
    cout << "Write Fin" << endl;
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

