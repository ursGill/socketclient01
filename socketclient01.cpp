#include <iostream>
#include <winsock.h>
#include <thread>
#include <string>
using namespace std;

#define SERVER_IP "10.75.100.192"
#define PORT 9909

void ReceiveMessages(SOCKET clientSocket) {
    char buffer[512];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int nRet = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (nRet <= 0) {
            cout << "\nDisconnected from server or error occurred.";
            closesocket(clientSocket);
            exit(0);
        }
        cout << "\n\n>> " << buffer << "\nYou: ";
        fflush(stdout);
    }
}

int main() {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << "WSA initialization failed.\n";
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr(SERVER_IP);

    cout << "Connecting to chat server...\n";
    int nRet = connect(clientSocket, (sockaddr*)&srv, sizeof(srv));
    if (nRet == SOCKET_ERROR) {
        cout << "Failed to connect to server.\n";
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    cout << "Connected successfully!\n";

    // Start a thread to receive messages
    thread recvThread(ReceiveMessages, clientSocket);
    recvThread.detach();

    // Main loop for sending messages
    string message;
    cout << "Type your messages below.\n";
    cout << "Use '<id>:<message>' for private messages.\n";
    cout << "Just type your message for broadcast.\n\n";

    while (true) {
        cout << "You: ";
        getline(cin, message);

        if (message == "exit" || message == "quit") {
            cout << "Closing connection...\n";
            closesocket(clientSocket);
            WSACleanup();
            exit(0);
        }

        int sent = send(clientSocket, message.c_str(), message.size(), 0);
        if (sent <= 0) {
            cout << "Failed to send message.\n";
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
