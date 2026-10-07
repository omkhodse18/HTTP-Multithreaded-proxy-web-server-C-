#include <iostream>
#include <string>

#include "net_io.hpp"
#include "http/http_parser.hpp"

namespace {
    std::string makeResponse(int statusCode, const std::string& reason, const std::string& body)
    {
        return "HTTP/1.1 " + std::to_string(statusCode) + " " + reason + "\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + std::to_string(body.size()) + "\r\n"
           "Connection: close\r\n"
           "\r\n"
           + body;
    }
}

void handleClient(const Socket& clientSocket, std::string& outputString){

    std::string fullResponse;

    RequestResult result = receiveRequest(clientSocket, outputString);

    if(result != RequestResult::Complete){
        std::cout << "Request is not complete." << std::endl;

        if(result == RequestResult::TooLarge){
            fullResponse = makeResponse(
                431,
                "Request Header Fields Too Large",
                "<html><body><h1>431 Request Header Fields Too Large</h1></body></html>"
            );
        }
        else{
            return;
        }
    }
    else{
        RequestLine outline;
        HeaderMap outHeaders;
        bool isBadRequest = false;
        bool isHeaderParsed = false;

        bool isRequestLineParsed = parseRequestLine(outputString, outline);
        
        if(isRequestLineParsed){
            // Print request line if parsed successfully.
            std::cout << "Request line -> "<< outline.method << " " << outline.target << " " << outline.version << std::endl;
            isHeaderParsed = parseHeaders(outputString, outHeaders);
        }
        
        if(isHeaderParsed) {
            if(outline.version == "HTTP/1.1" && outHeaders.find("host")==outHeaders.end() ){
                isBadRequest = true;
            }
            else{
                auto it = outHeaders.find("host"); 
                if(it != outHeaders.end()){
                    std::cout << it->first << " " << it->second << std::endl; 
                }
            }
        }
        
        if(!isRequestLineParsed || !isHeaderParsed){
            isBadRequest = true;
        }
        
        if(isBadRequest) 
        {
            fullResponse = makeResponse(
                400,
                "Bad Request", 
                "<html><body><h1>400 Bad Request</h1></body></html>"
            );
        }
        else if(outline.method != "GET")
        {
            fullResponse = makeResponse(
                501,
                "Not Implemented", 
                "<html><body><h1>501 Not Implemented</h1></body></html>"
            );
        }
        else if(outline.target == "/"){
            fullResponse = makeResponse(
                200,
                "OK",
                "<html><body><h1>Hello, Omkar</h1><p>Hello from my C++ server</p></body></html>"
            );
        }
        else
        {
            fullResponse = makeResponse(
                404,
                "Not Found",
                "<html><body><h1>404 Not Found</h1></body></html>"
            );
        }
    }   

    bool isSuccess = sendAll(clientSocket, fullResponse);

    if(isSuccess == false){
        std::cout << "Bytes not sent." << std::endl;
        return;
    }

    std::cout << "Bytes sent : " << fullResponse.size() << std::endl;
}