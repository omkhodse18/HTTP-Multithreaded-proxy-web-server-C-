#include "http_parser.hpp"
#include <string_view>

namespace {
    bool isTokenChar(unsigned char c) 
    {
        return (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '!' || c == '#' || c == '$' || c == '%' ||
            c == '&' || c == '\'' || c == '*' || c == '+' ||
            c == '-' || c == '.' || c == '^' || c == '_' ||
            c == '`' || c == '|' || c == '~';
    }

    char lowerAscii(char c) {
        if (c >= 'A' && c <= 'Z')
            return static_cast<char>(c + ('a' - 'A'));
        return c;
    }

    std::string_view trimOws(std::string_view s) {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
            s.remove_prefix(1);

        while (!s.empty() && (s.back() == ' ' || s.back() == '\t'))
            s.remove_suffix(1);

        return s;
    }
}

bool parseHeaders(const std::string& rawRequest, HeaderMap& outHeaders){
    std::string_view request{rawRequest};
    
    // Finds the end of the request line | Searched for the first \r\n. If missing, return false.
    size_t idxRequestLineEnd = rawRequest.find("\r\n");
    if (idxRequestLineEnd == std::string::npos) {
        return false;
    }

    // Finds the end of the header block | Searched for \r\n\r\n, starting from the position in b. If missing, return false.
    size_t idxHeaderBlockEnd = rawRequest.find("\r\n\r\n");
    if (idxHeaderBlockEnd == std::string::npos) {
        return false;
    }

    // No headers, If the \r\n\r\n is found at the same position as the first \r\n, there are no headers. Return true with an empty map.
    if(idxRequestLineEnd == idxHeaderBlockEnd){
        outHeaders.clear();
        return true;
    }

    std::string_view headerBlock = request.substr(idxRequestLineEnd+2, idxHeaderBlockEnd - (idxRequestLineEnd + 2));
   
    size_t pos = 0;
    size_t headerCount = 0;
    HeaderMap tempHeaderMap;

    while(pos < headerBlock.size())
    {
        // Find the end of the current header line.
        size_t idxlineEnd = headerBlock.find("\r\n", pos);

        std::string_view currentHeaderLine;

        if(idxlineEnd == std::string::npos){
            // Last line of header
            currentHeaderLine = headerBlock.substr(pos);
            pos = headerBlock.size();
        }
        else{
            currentHeaderLine = headerBlock.substr(pos, idxlineEnd-pos);
            pos = idxlineEnd + 2;
        }

        headerCount++;

        if(headerCount > kMaxHeaderCount){
            return false;
        }

        size_t colonIdx = currentHeaderLine.find(":");
        if(colonIdx == std::string::npos) { return false; }

        std::string_view name = currentHeaderLine.substr(0, colonIdx);
        std::string_view value = currentHeaderLine.substr(colonIdx+1);
            
        // The name must be non-empty.
        if(name.empty()) { return false; }

        // validate name | Name should only contain letters, digits and these characters: ! # $ % & ' * + - . ^ _ | ~`.
        for(char c:name){
            if(!isTokenChar(c))
                return false;
        }

        // Trim optional whitespace from value.
        value = trimOws(value);

        // Value control-character check
        for (unsigned char c : value) {
            if ((c < 0x20 && c != '\t') || c == 0x7F)
                return false;
        }    
        
        // Normalize the value
        std::string normalizedName;
        normalizedName.reserve(name.size());

        for (char c : name){
            normalizedName.push_back(lowerAscii(c));
        }

        // Insert into headerMap
        auto it = tempHeaderMap.find(normalizedName);
        if (it == tempHeaderMap.end()) 
        {
            tempHeaderMap[std::move(normalizedName)] = std::string(value);
        } 
        else if (normalizedName == "host" || normalizedName == "content-length") {
            // duplicate of a singleton header → reject
            return false;
        } 
        else {
            it->second.append(", ");
            it->second.append(value);
        }
    }

    outHeaders = std::move(tempHeaderMap);

    return true;
}


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
    