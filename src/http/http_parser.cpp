#include "http_parser.hpp"
#include <string_view>

bool parseRequestLine(const std::string& rawRequest, RequestLine& outLine)
{
    size_t pos1 = rawRequest.find("\r\n");
    
    if (pos1 == std::string::npos) {
        return false;
    }

    // std::string_view tempRequestLine = rawRequest.substr(0, pos1);
    std::string_view view{rawRequest}; 
    std::string_view tempRequestLine = view.substr(0, pos1); 

    // Parsing start
    // (Request line = GET /index.html HTTP/1.1)
    // Find the first space | 
    size_t first_space = tempRequestLine.find(' ');
    
    if (first_space == std::string::npos) {
        return false;
    }
    
    std::string_view method_part = tempRequestLine.substr(0, first_space);

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
    size_t  second_space = tempRequestLine.find(' ', first_space + 1);
    
    if(second_space == std::string::npos) {
        return false;
    }

    // target    
    std::string_view target_part = tempRequestLine.substr(first_space + 1, (second_space - first_space - 1));
    
    if(target_part.empty() || target_part.size() > kMaxTargetLength){
        return false;
    }

    // Validate Target: No control characters (< 0x20 or 0x7F)
    for (char c : target_part) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || uc == 0x7F) {
            return false;
        }
    }

    // find third space if exist then return false.
    size_t third_space = tempRequestLine.find(' ', second_space + 1);
    
    if(third_space != std::string::npos){
        return false;
    }

    // Version
    std::string_view version_part = tempRequestLine.substr(second_space + 1);

    if (version_part != "HTTP/1.0" && version_part != "HTTP/1.1") {
        return false;
    }

    outLine.method = method_part;
    outLine.target = target_part;
    outLine.version = version_part;

    return true;

}
    