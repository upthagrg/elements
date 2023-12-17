#pragma once
#pragma comment(lib, "wsock32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <errno.h>
#include <winsock.h>

using std::string;
using std::map;
using std::vector;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
using std::isblank;
using std::transform;

namespace EZNetwork {
	void LocalHarness(string page, int port, bool debug) {
        SOCKET init_socket;
        SOCKET new_socket;
        WSADATA data;
        struct sockaddr_in server;
        int server_length;
        int buffer_size = 30720;

        cout << "Initializing Web Harness..." << endl;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            cout << "Failed WSAStartup" << endl;
            exit(1);
        }

        cout << "Creating Socket..." << endl;
        init_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (init_socket == INVALID_SOCKET) {
            cout << "Failed to create socket" << endl;
            exit(2);
        }

        cout << "Binding Socket..." << endl;
        server.sin_family = AF_INET;
        string IPAddr;
        IPAddr = "127.0.0.1";//loopback IP
        server.sin_addr.s_addr = inet_addr(IPAddr.c_str());
        server.sin_port = htons(port);
        server_length = sizeof(server);

        if (bind(init_socket, (SOCKADDR*)&server, server_length) != 0) {
            cout << "Failed to bind socket" << endl;
            exit(3);
        }

        cout << "Listening to socket..." << endl;
        if (listen(init_socket, 20) != 0) {
            cout << "Could not start listening" << endl;
            exit(4);
        }

        cout << "Web Harness Started on " << IPAddr <<":" << port << endl;
        cout << "Launch: http://localhost:"<< port << "/" << endl;
        if (debug) {
            cout << "Page Date:" << endl;
            cout << page << endl;
        }

        int BytesRead = 0;
        int BytesSent = 0;
        int TotalBytesSent = 0;
        while (true) {
            new_socket = accept(init_socket, (SOCKADDR*)&server, &server_length);
            if (new_socket == INVALID_SOCKET) {
                cout << "Failed to create new socket" << endl;
                exit(5);
            }
            char buff[30720] = { 0 };
            BytesRead = recv(new_socket, buff, buffer_size, 0);
            if (BytesRead < 0) {
                cout << "Failed to read from new socket" << endl;
                exit(6);
            }
            cout << "Web Harness Read: " << endl;
            cout << buff << endl;

            string server_message = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
            server_message.append(to_string(page.size()));
            server_message.append("\n\n");
            server_message.append(page);
            BytesSent = 0;
            TotalBytesSent = 0;
            while (TotalBytesSent < server_message.size()) {
                BytesSent = send(new_socket, server_message.c_str(), server_message.size(), 0);
                if (BytesSent < 0) {
                    cout << "Failed to send" << endl;
                    exit(7);
                }
                TotalBytesSent += BytesSent;
            }
            closesocket(new_socket);
        }
        closesocket(init_socket);
        WSACleanup();
	}
}