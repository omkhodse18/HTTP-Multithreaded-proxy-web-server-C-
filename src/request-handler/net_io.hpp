#pragma once
#include "socket/socket.hpp"

#include <iostream>
#include <string>

constexpr size_t kMaxRequestSize = 16384;

enum class RequestResult {Complete, ClientClosed, Error, TooLarge};

bool sendAll(const Socket& clientSocket, const std::string& data);
RequestResult receiveRequest(const Socket& clientSocket, std::string& outputString);
void handleClient(const Socket& clientSocket, std::string& outputString);