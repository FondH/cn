#include<iostream>
#include<winsock2.h>
#include <windows.h>
#include<string>
#pragma comment(lib,"ws2_32.lib") 
#include "member.h"
#include "Server.h"
using namespace std;

int main() {
    Server server(8999);
    server.start_listen();

}