#pragma once
#include <string>

struct RequestLine {
    std::string method;
    std::string target;
    std::string version;
};

constexpr size_t kMaxTargetLength = 8192;

bool parseRequestLine(const std::string& rawRequest, RequestLine& outLine);
