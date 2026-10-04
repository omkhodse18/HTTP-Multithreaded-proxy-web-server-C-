#include "socket/socket.hpp"
#include "socket/platform.hpp"

#include "request-handler/client_handler.hpp"
#include "request-handler/net_io.hpp"

#include <iostream>
#include <utility>
#include <string>
#include <cstdlib>

bool tryParsePort(const char* text, unsigned short& outPort) {
    // 1. If the text is null or empty, return false.
    if (text == nullptr || text[0] == '\0') {
        return false;
    }

    // 2. Declare a char* named endPtr. Call std::strtol with the text, 
    // the address of endPtr, and base 10.
    char* endPtr;
    long parsedValue = std::strtol(text, &endPtr, 10);

    // 3. If the character at endPtr is not '\0', return false.
    // (Catches invalid trails like "abc" and "80abc")
    if (*endPtr != '\0') {
        return false;
    }

    // 4. If the parsed value is below 1 or above 65535, return false.
    if (parsedValue < 1 || parsedValue > 65535) {
        return false;
    }

    // 5. Otherwise convert it with static_cast<unsigned short>, 
    // store it in the output parameter, and return true.
    outPort = static_cast<unsigned short>(parsedValue);
    return true;
}

int main(int argc, char* argv[]) {

    // Default port number
    unsigned short port_number = 8888;

    // If port pass through command line
    if(argc == 2){
        bool isValid = tryParsePort(argv[1], port_number);
        if(!isValid) {
            std::cout << "Includes bad text." << std::endl;
            return 1;
        }
    }

    // WSA start
    NetworkSession wsaSession;

    // WSADATA not initialiaze
    if(!wsaSession.isWSAInitialized()){
        std::cout << "Error comes while initialising WSADATA. | Error -> " << wsaSession.wsaStartUpResult() << std::endl;
        std::cin.get();
        return 1;
    }

    Socket serverSocket(socket(AF_INET, SOCK_STREAM, 0));

    if(!serverSocket.valid()){
        std::cout << "Error comes while initialising server socket connection. | Error -> " << WSAGetLastError() << std::endl;
        std::cin.get();
        return 1;
    }

    // Binding...
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_number);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    sockaddr* serverAddressPtr = reinterpret_cast<sockaddr*>(&serverAddress);
    int iBindResult = bind(serverSocket.get(),serverAddressPtr, sizeof(serverAddress));

    // Bind fails
    if(iBindResult == SOCKET_ERROR){
        std::cout << "Error comes while binding socket connection. | Error -> " << WSAGetLastError() << std::endl;
        std::cin.get();
        return 1;
    }

    // Listening...
    int iListenResult = listen(serverSocket.get(), SOMAXCONN);
    if(iListenResult == SOCKET_ERROR){
        std::cout << "Error comes while listening socket connection. | Error -> " << WSAGetLastError() << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "Listening on 127.0.0.1:" << port_number << std::endl;
    std::cout << "Waiting for a client..." << std::endl;

    // Handle many requests..
    bool bKeepRunning = true;

    while(bKeepRunning)
    {
        // Accepting..
        sockaddr_in clientAddress{};
        sockaddr* clientAddressPtr = reinterpret_cast<sockaddr*>(&clientAddress);
        int iClientAddressLength = sizeof(clientAddress);
        
        // Client socket created. | Via Socket ctor.
        Socket clientSocket(accept(serverSocket.get(), clientAddressPtr, &iClientAddressLength));
    
        // Invalid socket
        if(!clientSocket.valid()){
            std::cout << "Invalid client socket. | Error -> " << WSAGetLastError() << std::endl;
            continue;
        }
        
        // Client connected
        std::cout << "Client connected." << std::endl;
        
        std::string outputString;
        handleClient(clientSocket, outputString);
        // closesocket(clientSocket.get());        
    }

    std::cout << "Press enter to exit..." << std::endl;
    std::cin.get();
    return 0;

}