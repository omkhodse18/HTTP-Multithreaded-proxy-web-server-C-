#include <iostream>
#include <cstdlib>

#include <winsock2.h>   // Declares the socket functions (socket, bind, closesocket, ...) and types (SOCKET, WSADATA).
#include <WS2tcpip.h>   // extra helpers for IP addresses and name lookup | getaddrinfo, inet_ntop

unsigned short port_number = 8888;

int main(int argc, char* argv[]) {
    if(argc == 2){
        port_number = atoi(argv[1]);    // ./proxy 8080
    }

    WSADATA wsaData;                // initialises the Winsock library inside process
    int startupResult = WSAStartup(MAKEWORD(2,2), &wsaData);  //int WSAStartup(WORD wVersionRequired(IN), LPWSADATA lpWSAData(OUT));

    // WSADATA not initialiaze
    if(startupResult != 0) {
        std::cout << "Error comes while initialising WSADATA. | Error -> " << startupResult << std::endl;
        std::cin.get();
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);   // param = socket(int domain, int type, int protocol);

    if(serverSocket == INVALID_SOCKET) {
        std::cout << "Error comes while initialising socket connection. | Error -> " << WSAGetLastError() << std::endl;
        WSACleanup();
        std::cin.get();
        return 1;
    }

    // Binding...
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_number);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    sockaddr* serverAddressPtr = reinterpret_cast<sockaddr*>(&serverAddress);
    int iBindResult = bind(serverSocket,serverAddressPtr, sizeof(serverAddress));

    // Bind fails
    if(iBindResult == SOCKET_ERROR){
        std::cout << "Error comes while binding socket connection. | Error -> " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        std::cin.get();
        return 1;
    }

    // Listening...
    int iListenResult = listen(serverSocket, SOMAXCONN);
    if(iListenResult == SOCKET_ERROR){
        std::cout << "Error comes while listening socket connection. | Error -> " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        std::cin.get();
        return 1;
    }

    std::cout << "Listening on 127.0.0.1:" << port_number << std::endl;
    std::cout << "Waiting for a client..." << std::endl;

    // accepting
    sockaddr_in clientAddress{};
    sockaddr* clientAddressPtr = reinterpret_cast<sockaddr*>(&clientAddress);
    int iClientAddressLength = sizeof(clientAddress);
    
    SOCKET clientSocket = accept(serverSocket, clientAddressPtr, &iClientAddressLength);

    // Invalid socket
    if(clientSocket == INVALID_SOCKET){
        std::cout << "Invalid client socket. | Error -> " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        std::cin.get();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // Recv
    char receiveBuffer[4096];
    int iBytesReceived = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer)-1, 0);
    
    if(iBytesReceived == SOCKET_ERROR)
    {
        std::cout << "Bytes not received. | Error -> " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        std::cin.get();
        return 1;
    }
    else if(iBytesReceived == 0)
    {
        std::cout << "Client closed the connection" << std::endl;
    }
    else if(iBytesReceived > 0)
    {
        receiveBuffer[iBytesReceived] = '\0';
        std::cout << iBytesReceived << std::endl;
        std::cout << receiveBuffer << std::endl;
    }


    std::cout << "Press enter to exit..." << std::endl;
    std::cin.get();
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}