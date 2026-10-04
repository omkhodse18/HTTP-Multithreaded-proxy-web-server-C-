#include "socket.hpp"

void Socket::reset()
{
    if(valid()){
        closesocket(socketHandle);
        socketHandle = INVALID_SOCKET;
    }
}

SOCKET Socket::get() const
{
    return socketHandle;
}

bool Socket::valid() const
{
    if(socketHandle == INVALID_SOCKET){
        return false;
    }
    else{
        return true;
    }
}