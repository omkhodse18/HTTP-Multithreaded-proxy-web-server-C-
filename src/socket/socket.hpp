#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>
#include <utility>

class Socket {
private:
    SOCKET socketHandle;

public:
    explicit Socket() : socketHandle(INVALID_SOCKET) { }
    explicit Socket(SOCKET _socket) : socketHandle(_socket) { }

    // Delete copy constructor and copy assignment operator.
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Move constructor.
    Socket(Socket&& other) noexcept : socketHandle(std::move(other.socketHandle)) {
        other.socketHandle = INVALID_SOCKET;
    }

    Socket& operator=(Socket&& other) noexcept {
        if(this == &other) { return *this; }
        reset();    // Close this socket.
        socketHandle = other.socketHandle;
        other.socketHandle = INVALID_SOCKET;
        return *this;
    }

    SOCKET get() const;   // return raw socket.
    bool valid() const;   // Is socket valid or not.
    void reset();   // Close the connection.


    ~Socket(){
        reset();
    }
};