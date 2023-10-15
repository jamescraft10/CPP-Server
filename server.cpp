/*
By: JamesCraft
Compile: g++ -o main main.cpp -lws2_32
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <string>

std::string GetFileData(std::string File) {
    std::string oneLine;
    std::string fullData;
    std::ifstream ReadFile;

    ReadFile.open(File);

    while(getline(ReadFile, oneLine)) {
        fullData += oneLine;
    }

    ReadFile.close();

    return fullData;
}

std::string ChoosePage(std::string Path) {
    if (Path == "") {
        return "html/index.html";
    }
    if(Path == "style/style.css") {
        return "style/style.css";
    }
    if(Path == "style/404.css") {
        return "style/404.css";
    }
    if(Path == "html/index.html") {
        return "html/index.html";
    }
    return "html/404.html";
}

std::string ChooseResponse(std::string Path) {
    if (Path == "") {
        return "text/html";
    }
    if (Path.find(".html") != std::string::npos) {
        return "text/html";
    } else if (Path.find(".css") != std::string::npos) {
        return "text/css";
    } else if (Path.find(".js") != std::string::npos) {
        return "application/javascript";
    } else {
        return "text/html";
    }
}

int main() {
    std::cout << "Attempting to create a server" << "\n";

    SOCKET wsocket;
    SOCKET new_wsocket;
    WSADATA wsaData;
    struct sockaddr_in server;
    int server_len;
    int BUFFER_SIZE = 30720;

    // Initialize
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "Could not initialize" << "\n";
        return EXIT_FAILURE;
    }

    // Creating a socket
    wsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(wsocket == INVALID_SOCKET) {
        std::cout << "Could not create socket" << "\n";
        return EXIT_FAILURE;
    }

    // Bind socket to address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(3000);
    server_len = sizeof(server);

    if(bind(wsocket, (SOCKADDR *)&server, server_len) != 0) {
        std::cout << "Could not bind socket" << "\n";
        return EXIT_FAILURE;
    }

    // Listen to address
    if(listen(wsocket, 20) != 0) {
        std::cout << "Could not start listening" << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Listening on Port 3000" << "\n";

    int bytes = 0;
    while(true) {
        // Accept client request
        new_wsocket = accept(wsocket, (SOCKADDR *)&server, &server_len);
        if(new_wsocket == INVALID_SOCKET) {
            std::cout << "Could not accept" << "\n";
            return EXIT_FAILURE;
        }

        // Read request
        char buff[30720] = { 0 };
        bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
        if(bytes < 0) {
            std::cout << "Could not read client request" << "\n";
            return EXIT_FAILURE;
        }

        // Parse requested URL
        std::string request(buff);
        std::string path;
        size_t start = request.find("GET /") + 5;
        size_t end = request.find(" HTTP/");
        if(start != std::string::npos && end != std::string::npos) {
            path = request.substr(start, end - start);
        }

        // Response based on path
        std::string serverMessage = "HTTP/1.1 200 OK\nContent-Type: " + ChooseResponse(path) + "\nContent-Length: ";
        std::string filePath = ChoosePage(path);
        std::string response = GetFileData(filePath);
        serverMessage.append(std::to_string(response.size()));
        serverMessage.append("\n\n");
        serverMessage.append(response);

        // Send response
        int bytesSent = send(new_wsocket, serverMessage.c_str(), serverMessage.size(), 0);
        if (bytesSent < 0) {
            std::cout << "Could not send response" << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Sent response to client" << "\n";
        closesocket(new_wsocket);
    }

    closesocket(wsocket);
    WSACleanup();

    return EXIT_SUCCESS;
}