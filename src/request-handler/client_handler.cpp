#include "net_io.hpp"

void handleClient(const Socket& clientSocket, std::string& outputString){



    RequestResult result = receiveRequest(clientSocket, outputString);

    if(result != RequestResult::Complete){
        std::cout << "Request is not complete." << std::endl;
        return;
    }

    std::cout << outputString << std::endl;

    std::string responseBody;
    std::string responseHeader;
    std::string fullResponse;

    responseBody = R"=====(
        <html>
            <body>
                <h1>Hello, Omkar</h1>
                <p>Hello from my C++ server</p>
            </body>
        </html>
    )=====";
    
    std::string contentLength = std::to_string(responseBody.size());
    responseHeader =  {
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + contentLength + "\r\n"
        "Connection: close\r\n"
        "\r\n"
    };
    
    fullResponse = responseHeader + responseBody;
    

    bool isSuccess = sendAll(clientSocket, fullResponse);

    if(isSuccess == false){
        std::cout << "Bytes not sent." << std::endl;
        return;
    }

    std::cout << "Bytes sent : " << fullResponse.size() << std::endl;
}