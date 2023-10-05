#pragma once
#include<iostream>
#include<winsock2.h>
#include<string>
#include "member.h"
#include "Room.h"
#include<vector>
#pragma comment(lib,"ws2_32.lib") 
using namespace std;

vector<SOCKET> client_sockets;
int room_num = 2;
int room_members = 5;
SOCKET listen_socket;

vector<Room> rooms(room_num);


string Mess_send(int type,string mess) {
		string send_mess = "";
		if (type == 0) {

			//发送status:0/5,1/5
			send_mess.append("status:");
			for (int i = 0;i < room_num-1;i++)
				send_mess += to_string(rooms[i].get_status()) + "/" + to_string(room_members) + ",";
			send_mess += to_string(rooms[room_num-1].get_status()) + "/" + to_string(room_members);
			//send(s, send_mess.c_str(), send_mess.size(), 0);
		}
		else if (type == 1) {
			//发送广播 chat:username:mess:time
			send_mess = mess;
		}
		else 
			//发送广播exit:username:time:room_num  enter:username:time:room_num
			send_mess = mess + ":" + to_string(type);

//		cout << send_mess << endl;
	//	cout << send_mess.size();
		return send_mess;
	}

void BroadcastMessageInRoom(Room& room, member& sender, string& message) {

	for (member& client : room.members) {
		/*if (client.get_socket() != sender.get_socket()) {
			send(client.get_socket(), formattedMessage.c_str(), formattedMessage.size()+1, 0);
		}*/
		send(client.get_socket(), message.c_str(), message.size() + 1, 0);
	}
}

vector<string> split(string& s, char delimiter) {
	//将string 按照;:, 划分
	vector<string> tokens;
	string token;
	for (int i = 0; i < s.length(); i++) {
		if (s[i] != delimiter) {
			token += s[i];
		}
		else {
			tokens.push_back(token);
			token.clear();
		}
	}
	if (!token.empty()) {  
		tokens.push_back(token);
	}
	return tokens;
}

time_t getCurrentTime() {

	time_t rawtime;
	time(&rawtime);

	return rawtime;
}

DWORD WINAPI ClientHandler(LPVOID param) {
	SOCKET client_socket = (SOCKET)param;
	char buffer[256];
	 //bool client_status = 1;
	cout<<"线程打开！";
	// ... 发送房间状态 ...
	string mess = Mess_send(0, "");
	send(client_socket,mess.c_str(),mess.size()+1,0);
	cout << "Server send：" << mess << "to " << int(client_socket)<<endl;

	int roomNumber=-1;  
	string username="default";
	member thisClient = { client_socket, username };




	while (true) {
		int bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
		cout << "Server recive： " << buffer<<endl;
		if (bytesReceived <= 0) { printf("客户端断开或者错误"); break; }  

		string bufferString = string(buffer);
		vector<string> tokens = split(bufferString, ':');
		
		if (tokens[0] == "choice") {
			//choice:1:username
			roomNumber = stoi(tokens[1]);
			username = tokens[2];
			thisClient.set_name(username);

			// ... 添加客户端到所选房间 ...
			rooms[roomNumber].add_member(thisClient);
			int p = rooms[roomNumber].get_status();
			string message = "enter:" + tokens[1]+ ":" + username +":" + to_string(int(getCurrentTime()))+":"+to_string(p);
			BroadcastMessageInRoom(rooms[roomNumber], thisClient, message);
			cout << "room No." << roomNumber << ":" << username << " entered! "<< endl;
			
		}
		else if (tokens[0] == "chat") {
			string message(buffer, bytesReceived);	
			BroadcastMessageInRoom(rooms[roomNumber], thisClient, message);
			cout << "room No." << roomNumber << ":" << username << " saying:" << buffer << endl;
		}
		else if (tokens[0] == "exit") {
			bufferString += ":" + to_string(int(getCurrentTime())) + ":"+ to_string(rooms[roomNumber].get_status());
			BroadcastMessageInRoom(rooms[roomNumber], thisClient, bufferString);
			cout<<"room No." << roomNumber <<": " << username << "has exited! " << endl;
			break;
		}

		
	}
	
//	徐翊航  xu yi hang
	// 客户端断开连接后，从房间中移除/
	rooms[roomNumber].remove(thisClient);
	closesocket(thisClient.get_socket());

	return 0;
}


class Server {

public:
	Server(int port) {
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(sockVersion, &wsaData) != 0)
		{
			printf("Error");
		}

		//members = new member[255];
		listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in sock_addr;
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_port = htons(port);
		sock_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

		if (bind(listen_socket, (sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
			printf("Server Bind defeat！");
			closesocket(listen_socket);
		}
		cout << "Fond-Chat Server Has Started in port: " << port<<" ..."  << "\n";

	}


	SOCKET _listen() {
		SOCKET sClient;
		sockaddr_in remote_addr;
		int lenOfaddr = sizeof(remote_addr);

		//printf("等待连接!");
		sClient = accept(listen_socket, (SOCKADDR*)&remote_addr, &lenOfaddr);
		if (sClient == INVALID_SOCKET) {
			printf("accept error\n");
		}
		printf("%d has connected \n",(int)sClient);
		return sClient;
	}
	

	int start_listen() {
		if (listen(listen_socket, 5) == SOCKET_ERROR)
		{
			printf("listen Server Error !");
			return SOCKET_ERROR;
		}
		cout << "Start Listen..." << endl;

		while (true) {
			//char* room_status = get_room_status();
			SOCKET sClient = _listen();
			if (sClient != INVALID_SOCKET) 
				CreateThread(NULL, 0, ClientHandler, (LPVOID)sClient, 0, NULL);

		
		}


		return 1;
	}


};


