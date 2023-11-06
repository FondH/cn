#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <string>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define BufferSize 20 * 1024 * 1024 
#define PayloadSize 4 * 1024
#define port 8999
#define testFile test.jpg
#define outputFile out.jpg
using namespace std;
using streamsize = long long;


struct Header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t seq;
    uint32_t ack;
    uint16_t dataLen;
    uint16_t headerLen;

    Header(): type(0), code(0), checksum(0), seq(0),ack(0), dataLen(0), headerLen(sizeof(Header)) {}

};
class Udp {
    Header *  header;
    char* payload;

    Udp(){
        header = nullptr;
        payload = new char[PayloadSize];
    }
    Udp(Header* h, char* c) {
        header = h;
        memcpy(payload, c, PayloadSize);
    }
};
void Out2file(char * src, streamsize& size ,string& dst_name) {
    ofstream outFile(dst_name, ifstream::binary);
   // cout << src;
    outFile.write(src, size);
    if (!outFile.good()) 
        cerr << "Write Defeted" << endl;
    outFile.close();

}
streamsize ReadFile(string name, char * buffer){

    ifstream file(name, ifstream::binary);
    if (!file) {
        cerr <<"Error:" << name << " Open Defeted ! " << endl;
        return 0;
    }
    file.seekg(0, ifstream::end);
    streamsize size = file.tellg();
    file.seekg(0);
       
    if (!file.read(buffer, size)) {
        cerr << "Error:" << name << " Read Defeted !" << endl;
        return 0;
    }
    cout << "File: " << name << " is Loading\n"<<"Size: " << size <<" Bytes" << endl;
    file.close();
    return size;
}
int main(){
    string filename = "test.jpg";
    string outFile = "out.txt";
    char* buffer = new char[BufferSize];
    
    streamsize filesize = ReadFile(filename, buffer);
    //Out2file(buffer, filesize, outFile);
}