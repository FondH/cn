#pragma once
#include<iostream>
#include<winsock2.h>
#include<string.h>
#pragma comment(lib,"ws2_32.lib") 
using namespace std;

struct mess {
	string username = "default";
	string message = "";
	string time = "";

};

class member {
private:
	SOCKET socket;
	string name;
public:

	member() { name =""; }
	member(SOCKET s, string n) { socket = s;name = n; }
	void set_name(string n) { name = n; }
	void set_socket(SOCKET s) { socket = s; }

	SOCKET get_socket() { return socket; }
	string get_name() { return name; }

};