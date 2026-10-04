#pragma once
#include "socket.hpp"

class NetworkSession {
private:
    WSADATA wsaData;
    int wsaStartupResult;

public:
    NetworkSession() {
        wsaStartupResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    }

    // Delete copy constructor and copy assignment operator.
    NetworkSession(const NetworkSession&) = delete;
    NetworkSession& operator=(const NetworkSession&) = delete;

    bool isWSAInitialized() {
        return (wsaStartupResult == 0);
    }

    int wsaStartUpResult() const { return wsaStartupResult; }

    ~NetworkSession(){
        if(isWSAInitialized()){
            WSACleanup();
        }
    }

};