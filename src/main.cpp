#include "server.h"
#include <iostream>

int main() {
    int serverSocket = createServerSocket();
    bindSocket(serverSocket);
    listenOnSocket(serverSocket);

    while (true) {
        std::cout << "Esperando conexión de cliente..." << std::endl;
        int clientSocket = acceptClient(serverSocket);
        if (clientSocket >= 0) {
            std::cout << "Cliente conectado.\n";
            while (true) {
                bool result = receiveMessage(clientSocket);
                if (!result) {
                    std::cout << "Cliente desconectado.\n";
                    break;
                }
            }
            closeSocket(clientSocket);
        }
    }

    return 0;
}
