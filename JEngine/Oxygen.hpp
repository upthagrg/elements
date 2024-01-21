/*
File: Oxygen.hpp
Author: Glenn Upthagrove
Last Updated: 01/21/2024
Description: This header file contains the Oxygen networking library for the Elements application framework. 
This currently allows for the creation of an O2Socket object, which facilitiates inter-process and networked communication. 
*/


#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <signal.h>
#include <errno.h>
#include <winsock.h>
#include "Hydrogen.hpp"

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

namespace O2 {

    class O2Socket {
    private:
        WSADATA WSA_data;
        SOCKET Open_Socket;
        SOCKET Usable_Socket;
        char Socket_Error[64] = { 0 };
        int Socket_Error_Size = 64;
        int Socket_AF;
        int Socket_Type;
        int Socket_Protocol;
        int Socket_Port;
        bool Socket_Bound;
        struct sockaddr_in Socket_Addr_In;
        int Socket_Addr_In_Size;
        string Socket_IPAddr;
        int Allowed_Backlog;
        int Buffer_Size;
        char* Buffer;
        bool Is_Valid;

    public:
        O2Socket();
        O2Socket(WORD, int, int, int, int);
        void PortBind(string, int);
        void PortListen(int);
        char* Recieve(bool);
        void SocketSend(string);
        void CloseUsableSocket();
        void CloseSocket();
    };

    O2Socket::O2Socket() {
        Socket_Addr_In_Size = -1;
        Allowed_Backlog = -1;
        Socket_Bound = false;
        Socket_AF = -1;
        Socket_Type = -1;
        Socket_Protocol = -1;
        Buffer_Size = -1;
        Buffer = NULL;
        Is_Valid = false;
    }

    O2Socket::O2Socket(WORD WSAVersion, int Address_Family, int Type, int Protocol, int Read_Buffer_Size) {
        Socket_Addr_In_Size = 0;
        Allowed_Backlog = 0;
        Socket_Bound = false;
        Socket_AF = Address_Family;
        Socket_Type = Type;
        Socket_Protocol = Protocol;
        Buffer_Size = Read_Buffer_Size;
        Buffer = new char[Read_Buffer_Size];
        ErrorCheck0((WSAStartup(MAKEWORD(2, 2), &WSA_data)), 1, "Failed WSAStartup");
        Open_Socket = socket(Address_Family, Type, Protocol);
        if (Open_Socket == INVALID_SOCKET) {
            cout << "Failed to create open socket" << endl;
            exit(2);
        }
        Is_Valid = true;
    }

    void O2Socket::PortBind(string IPAddress, int Port) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        Socket_IPAddr = "127.0.0.1";//loopback IP
        if (IPAddress.length() > 0) {
            Socket_IPAddr = IPAddress;
        }
        Socket_Addr_In.sin_family = Socket_AF;
        Socket_Addr_In.sin_addr.s_addr = inet_addr(IPAddress.c_str());
        Socket_Port = Port;
        Socket_Addr_In.sin_port = htons(Port);
        Socket_Addr_In_Size = sizeof(Socket_Addr_In);

        ErrorCheck0((bind(Open_Socket, (SOCKADDR*)&Socket_Addr_In, Socket_Addr_In_Size)), 3, "Failed to bind socket");
        Socket_Bound = true;
    }

    void O2Socket::PortListen(int Backlog) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (!Socket_Bound) {
            ErrorAndDie(20, "Cannot listen to an unbound port");
        }
        Allowed_Backlog - Backlog;
        int Server_Backlog = Backlog;
        ErrorCheck0((listen(Open_Socket, Backlog)), 4, "Could not start listening to socket");
    }

    char* O2Socket::Recieve(bool Close) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        int BytesRead = 0;
        if (!Socket_Bound) {
            ErrorAndDie(21, "Cannot read from an unbound port");
        }
        Usable_Socket = accept(Open_Socket, (SOCKADDR*)&Socket_Addr_In, &Socket_Addr_In_Size);
        if (Usable_Socket == INVALID_SOCKET) {
            cout << "Failed to create usable socket" << endl;
            exit(5);
        }
        memset(Buffer, '\0', Buffer_Size);
        BytesRead = recv(Usable_Socket, Buffer, Buffer_Size, 0);
        if (BytesRead < 0) {
            ErrorAndDie(6, "Failed to read from socket");
        }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (Close && getsockopt(Usable_Socket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(Usable_Socket)), 23, "Failed to close usable socket");
        }
        return Buffer;
    }

    void O2Socket::SocketSend(string Message) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        int BytesSent = 0;
        int TotalBytesSent = 0;
        while (TotalBytesSent < Message.size()) {
            BytesSent = send(Usable_Socket, Message.c_str(), Message.size(), 0);
            if (BytesSent < 0) {
                ErrorAndDie(7, "Failed to send");
            }
            TotalBytesSent += BytesSent;
        }
    }

    void O2Socket::CloseUsableSocket() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (getsockopt(Usable_Socket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(Usable_Socket)), 23, "Failed to close usable socket");
        }
    }

    void O2Socket::CloseSocket() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (getsockopt(Open_Socket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(Open_Socket)), 23, "Failed to close open socket");
        }
        ErrorCheck0(WSACleanup(), 24, "WSA Cleanup failed");
    }
}