#pragma once

#include <string>
#include <map>

struct RequestLine {
    std::string method;
    std::string target;
    std::string version;
};

constexpr size_t kMaxTargetLength = 8192;
bool parseRequestLine(const std::string& rawRequest, RequestLine& outLine);

constexpr size_t kMaxHeaderCount = 100;
// typedef std::map<std::string, std::string> HeaderMap;
using HeaderMap = std::map<std::string, std::string>;
bool parseHeaders(const std::string& rawRequest, HeaderMap& outHeaders);