#pragma once
#include<iostream>
#include<winsock2.h>
#include<vector>
#include "member.h"
#pragma comment(lib,"ws2_32.lib") 
using namespace std;


class Room{
public:
	int members_index[5];
	int max_num = 5;
	int current_num=0;
	vector<member> members;

	Room() {}
	Room(int mx) {
		max_num = mx;
		current_num = 0;

		for (int i = 0;i < 5;i++)
			members_index[i] = -1;
		//reciBuffers = new char* [max_num];
		//for (int i = 0;i < max_num;i++) 
			//reciBuffers[i] = new char[255];

	}
	int send_toAll() {
		
		for (int i = 0;i < current_num;i++) {
			//(members+i)->_send(reciBuffer, 255);
		}
	}
	int get_status() {
		return current_num;
	}
	void add_member(member m ) {
		members.push_back(m);
		
		current_num++;
	}
	void remove(member m) {
		//auto& clientsInRoom = rooms[roomNumber].members;
		members.erase(remove_if(members.begin(), members.end(),
			[&](member& client) { return client.get_socket() == m.get_socket(); }),
			members.end());
		current_num--;
	}

};