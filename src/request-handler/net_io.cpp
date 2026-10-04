#include "request-handler/net_io.hpp"

bool sendAll(const Socket& clientSocket, const std::string& data){
    int iBytesSentCnt = 0;
    int iDataSize = static_cast<int>(data.size());

    while(iBytesSentCnt < iDataSize) 
    {
        const char* remainingDataPtr = data.c_str() + iBytesSentCnt;
        int remainingLength = static_cast<int>(iDataSize - iBytesSentCnt);

        int result = send(clientSocket.get(), remainingDataPtr, remainingLength, 0);
        
        if (result == SOCKET_ERROR) {
            std::cerr << "Winsock error occurred: " << WSAGetLastError() << std::endl;
            return false;
        }

        iBytesSentCnt += result;
    }

    return true;
}

RequestResult receiveRequest(const Socket& clientSocket, std::string& outputString){
    char receiveBuffer[4096];

    while(true){
        int iBytesReceived = recv(clientSocket.get(), receiveBuffer, sizeof(receiveBuffer), 0);

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
