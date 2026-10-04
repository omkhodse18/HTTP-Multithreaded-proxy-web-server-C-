#include <iostream>
#include <cstdlib>
#include <string>

#include <winsock2.h>   // Declares the socket functions (socket, bind, closesocket, ...) and types (SOCKET, WSADATA).
#include <WS2tcpip.h>   // extra helpers for IP addresses and name lookup | getaddrinfo, inet_ntop


constexpr size_t kMaxRequestSize = 16384;

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

enum class RequestResult {Complete, ClientClosed, Error, TooLarge};


/*
SendAll()
*/
bool sendAll(SOCKET clientSocket, const std::string& data){
    int iBytesSentCnt = 0;
    int iDataSize = static_cast<int>(data.size());

    while(iBytesSentCnt < iDataSize) 
    {
        const char* remainingDataPtr = data.c_str() + iBytesSentCnt;
        int remainingLength = static_cast<int>(iDataSize - iBytesSentCnt);

        int result = send(clientSocket, remainingDataPtr, remainingLength, 0);
        
        if (result == SOCKET_ERROR) {
            std::cerr << "Winsock error occurred: " << WSAGetLastError() << std::endl;
            return false;
        }

        iBytesSentCnt += result;
    }

    return true;
}

/*
receiveRequest()
*/
RequestResult receiveRequest(SOCKET clientSocket, std::string& outputString){
    char receiveBuffer[4096];

    while(true){
        int iBytesReceived = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);

        if (iBytesReceived == SOCKET_ERROR) {
            std::cerr << "Winsock error occurred: " << WSAGetLastError() << std::endl;
            return RequestResult::Error;
        }
        
        if(iBytesReceived == 0)
        {   
            std::cout << "Client closed the connection" << std::endl;
            return RequestResult::ClientClosed;
        }
        
        outputString.append(receiveBuffer, iBytesReceived);

        if(outputString.find("\r\n\r\n") != std::string::npos)
        {
            return RequestResult::Complete;
        }

        if (outputString.size() > kMaxRequestSize) {
            return RequestResult::TooLarge;
        }
    }
}

/*
HandleClient()
*/
void handleClient(SOCKET clientSocket, std::string& outputString){
    RequestResult result = receiveRequest(clientSocket,outputString);

    if(result != RequestResult::Complete){
        std::cout << "Request is not complete." << std::endl;
        return;
    }

    std::cout << outputString << std::endl;

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
    

    bool isSuccess = sendAll(clientSocket, fullResponse);

    if(isSuccess == false){
        std::cout << "Bytes not sent." << std::endl;
        return;
    }

    std::cout << "Bytes sent : " << fullResponse.size() << std::endl;
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

    // Handle many requests..
    bool bKeepRunning = true;

    while(bKeepRunning)
    {
        // Accepting..
        sockaddr_in clientAddress{};
        sockaddr* clientAddressPtr = reinterpret_cast<sockaddr*>(&clientAddress);
        int iClientAddressLength = sizeof(clientAddress);
        
        // Client socket created.
        SOCKET clientSocket = accept(serverSocket, clientAddressPtr, &iClientAddressLength);
    
        // Invalid socket
        if(clientSocket == INVALID_SOCKET){
            std::cout << "Invalid client socket. | Error -> " << WSAGetLastError() << std::endl;
            continue;
        }
        
        // Client connected
        std::cout << "Client connected." << std::endl;
        
        std::string outputString;
        handleClient(clientSocket, outputString);
        closesocket(clientSocket);        
    }

    std::cout << "Press enter to exit..." << std::endl;
    std::cin.get();
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}