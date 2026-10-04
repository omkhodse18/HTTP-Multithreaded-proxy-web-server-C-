#include <iostream>
#include <cstdlib>
#include <string>

#include <winsock2.h>   // Declares the socket functions (socket, bind, closesocket, ...) and types (SOCKET, WSADATA).
#include <WS2tcpip.h>   // extra helpers for IP addresses and name lookup | getaddrinfo, inet_ntop


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

    // START
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


        std::string responseBody;
        std::string responseHeader;
        std::string fullResponse;

        responseBody = R"=====(
            <html>
                <body>
                    <h1>Hello, Omkar</h1>
                    <p>Hello from my C++ server</p>
                </body>
            </html>
        )=====";

        std::string contentLength = std::to_string(responseBody.size());
        responseHeader =  {
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + contentLength + "\r\n"
            "Connection: close\r\n"
            "\r\n"
        };

        fullResponse = responseHeader + responseBody;

        int fullResponseLen = static_cast<int>(fullResponse.size());
        int iBytesSent = send(clientSocket, fullResponse.c_str(), fullResponseLen, 0);
        
        if(fullResponseLen != iBytesSent) {
            std::cout << "[WARNING] | Buffer and bytes sent size is different." << std::endl;
        }

        if(iBytesSent == SOCKET_ERROR){
            std::cout << "Bytes not sent. | Error -> " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            std::cin.get();
            return 1;
        }

        std::cout << "Bytes sent : " << iBytesSent << std::endl;

    }


    std::cout << "Press enter to exit..." << std::endl;
    closesocket(clientSocket);
    std::cin.get();
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}