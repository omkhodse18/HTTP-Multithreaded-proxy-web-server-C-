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

        bool isParseSuccessful = parseRequestLine(outputString, outline);
    
        if(!isParseSuccessful) 
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

        // Print request line
        if(isParseSuccessful)
            std::cout << "Request line -> "<< outline.method << " " << outline.target << " " << outline.version << std::endl;
    }

    bool isSuccess = sendAll(clientSocket, fullResponse);

    if(isSuccess == false){
        std::cout << "Bytes not sent." << std::endl;
        return;
    }

    std::cout << "Bytes sent : " << fullResponse.size() << std::endl;
}