#ifndef SERVER_H
#define SERVER_H

#include <iostream>

int createServerSocket();
void bindSocket(int serverSocket);
void listenOnSocket(int serverSocket);
int acceptClient(int serverSocket);
bool receiveMessage(int clientSocket);
void closeSocket(int socketFD);
void sendMessage(int clientSocket, const std::string& message);

#endif
