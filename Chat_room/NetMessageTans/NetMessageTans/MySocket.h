#pragma once
#include<iostream>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") 
using namespace std;
class BaseSocket {
private:
    char* ServerName;
    SOCKET serverSocket;
    SOCKET* clinetSockets = new SOCKET[255];
    struct sockaddr_in serverAddress;


public:
    BaseSocket(int port, char* name) {
        ServerName = name;
        ServerName[strlen(name)] = '\0';
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating server socket");
            exit(EXIT_FAILURE);
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            perror("Error binding server socket");
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            perror("Error listening on server socket");
            exit(EXIT_FAILURE);
        }
        printf("Server %s has seted up; \n Port: %n , Current Nums:0", ServerName, port);
    }

    void acceptConnection() {
        SOCKET clientSocket;
        sockaddr_in clientAddress;
        int  clientAddressLen = sizeof(clientAddress);

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket == SOCKET_ERROR) {
            perror("Error accepting client connection");
            exit(EXIT_FAILURE);
        }
        PushClient(clientSocket);
        cout << "Accepted a connection from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;
    }
    void PushClient(SOCKET clientSocket) {

        
    }
    /*
    void sendMessage(const char* message) {
        send(clientSocket, message, strlen(message), 0);
    }

    std::string receiveMessage() {
        char buffer[1024] = { 0 };
        recv(clientSocket, buffer, sizeof(buffer), 0);
        return std::string(buffer);
    }

    void closeConnection() {
        closesocket(sClient);
        closesocket(serverSocket);
    }
    */


};