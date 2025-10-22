#include <iostream>
#include <winsock.h>
#include<thread>
#include <string>
using namespace std;

#define BROADCAST_PORT 9910
#define SERVER_PORT 9909

// function to auto-detect server IP address Using UDP broadcast
string detectServerIP() {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        cout << "UDP Socket creation failed. \n";
        return "";
    }

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(BROADCAST_PORT);
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udpSocket, (sockaddr*)&recvAddr, sizeof(recvAddr)) < 0) {
        cout << "UDP bind failed. \n";
        closesocket(udpSocket);
        return "";
    }
    cout << "Searching for server on LAN...\n";

    char buffer[256];
    sockaddr_in sender;
    int senderLen = sizeof(sender);
    int bytes = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&sender, &senderLen);
    closesocket(udpSocket);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        string msg(buffer);
        size_t pos = msg.find("SERVER_IP:");
        if (pos != string::npos) {
            string ip = msg.substr(pos + 10);
            cout << "Server Found at : " << ip << endl;
            return ip;
        }
    }

    cout << "Server Not Found . Try Again.....\n";
    return "";
}
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
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        cout << "WSA initialization failed.\n";
        return -1;
    }

    string serverIP = detectServerIP();
    if (serverIP.empty()) {
        WSACleanup();
        return -1;
    }
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in srv;   // Server Address structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(SERVER_PORT);
    srv.sin_addr.s_addr = inet_addr(serverIP.c_str());

    cout << "Connecting to Server...\n";
    int nRet = connect(clientSocket, (sockaddr*)&srv, sizeof(srv));
    if (nRet <0) {
        cout << "Failed to connect to server.\n";
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    cout << "Connected Successfully!\n";

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
