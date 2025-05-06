#include "server.h"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int createServerSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void bindSocket(int serverSocket) {
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
}

void listenOnSocket(int serverSocket) {
    listen(serverSocket, 5);
}

int acceptClient(int serverSocket) {
    return accept(serverSocket, nullptr, nullptr);
}

bool receiveMessage(int clientSocket) {
    char buffer[1024] = { 0 };
    int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return false;  // cliente cerró conexión o error

    std::string mensaje(buffer);
    std::cout << "Mensaje recibido: " << mensaje << std::endl;

    const char* response = nullptr;

    if (mensaje == "STEP") {
        response = R"({
            "metricas": [["Ciclos", 124], ["Fallas", 3], ["Ejecutadas", 76]],
            "pe": [[0, "LOAD A, #5"], [1, "ADD R1, R2"], [2, "STORE R3, [0x01]"]]
        })";
    } else if (mensaje == "CLK") {
        response = R"({
            "metricas": [["Ciclos", 125], ["Fallas", 3], ["Ejecutadas", 77]],
            "pe": [[0, "ADD R1, R2"], [1, "STORE R3, [0x01]"]]
        })";
    } else if (mensaje == "RESET") {
        response = R"({
            "metricas": [["Ciclos", 0], ["Fallas", 0], ["Ejecutadas", 0]],
            "pe": [[0, "NOP"], [1, "NOP"], [2, "NOP"]]
        })";
    }

    if (response) {
        send(clientSocket, response, strlen(response), 0);
    }

    return true;
}


void closeSocket(int socketFD) {
    close(socketFD);
}
