#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

constexpr auto MAX_BUFFER = 512;
constexpr char SERVER_PORT[] = "27015";
constexpr char SERVER_ADDRESS[] = "127.0.0.1";

void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw runtime_error("Winsock initialization failed.");
    }
}

SOCKET createAndConnectSocket(const addrinfo* addr) {
    SOCKET sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        throw runtime_error("Failed to create socket.");
    }

    if (connect(sock, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        throw runtime_error("Failed to connect to server.");
    }
    return sock;
}

void cleanUp(SOCKET sock) {
    closesocket(sock);
    WSACleanup();
}

int main() {
    try {
        initializeWinsock();
        struct addrinfo hints{}, *serverInfo;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &serverInfo) != 0) {
            WSACleanup();
            throw runtime_error("Failed to get server address info.");
        }

        SOCKET clientSocket = createAndConnectSocket(serverInfo);
        freeaddrinfo(serverInfo);

        string userInput;
        char serverResponse[MAX_BUFFER];
        while (true) {
            cout << "Enter message: ";
            getline(cin, userInput);

            if (send(clientSocket, userInput.c_str(), static_cast<int>(userInput.length()), 0) == SOCKET_ERROR) {
                throw runtime_error("Failed to send message.");
            }

            int bytesReceived = recv(clientSocket, serverResponse, MAX_BUFFER, 0);
            if (bytesReceived > 0) {
                serverResponse[bytesReceived] = '\0';
                cout << "Received from server: " << serverResponse << endl;
            } else if (bytesReceived == 0) {
                cout << "Server closed the connection." << endl;
                break;
            } else {
                throw runtime_error("Failed to receive message.");
            }
        }

        cleanUp(clientSocket);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}