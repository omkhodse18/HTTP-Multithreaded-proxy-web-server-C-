#include "http_parser.hpp"
#include <iostream>

bool parseRequestLine(const std::string& rawRequest, RequestLine& outLine)
{
    size_t pos1 = rawRequest.find("\r\n");

    if (pos1 == std::string::npos) {
        return false;
    }

    // (Request line = GET /index.html HTTP/1.1)
    // Find the first space | 
    size_t first_space = rawRequest.find(' ');
    
    if (first_space == std::string::npos) {
        return false;
    }
    
    std::string_view method_part = rawRequest.substr(0, first_space);

    if(method_part.empty()) {
        return false;
    }

    // method part should be UPPERCASE only.
    for(char c:method_part){
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 'A' || uc > 'Z') {
            return false;
        }
    }

    // Find the second space (starting the search after the first space) | target part
    size_t  second_space = rawRequest.find(' ', first_space + 1);
    
    if(second_space == std::string::npos) {
        return false;
    }

    std::string_view target_part = rawRequest.substr(first_space + 1, (second_space - first_space + 1));

    // Validate Target: No control characters (< 0x20 or 0x7F)
    for (char c : target_part) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || uc == 0x7F) {
            return false;
        }
    }


    // find third space if exist then return false.
    size_t third_space = rawRequest.find(' ', second_space + 1);
    
    if(third_space != std::string::npos){
        return false;
    }

    std::string_view version_part = rawRequest.substr(second_space + 1);

    if (version_part != "HTTP/1.0" && version_part != "HTTP/1.1") {
        return false;
    }

    outLine.method = method_part;
    outLine.target = target_part;
    outLine.version = version_part;

    return true;

}
    