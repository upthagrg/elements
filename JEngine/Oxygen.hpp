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
#include <unordered_map>
#include <vector>
#include <signal.h>
#include <errno.h>
#include <winsock.h>
#include <time.h>
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
    typedef int O2SocketID;

    struct Connected_Socket {
        SOCKET Soc;
        char* SocBuff;
        time_t Created;
        time_t LastAccessed;
        int State;
    };

    class O2Socket : public Hydrogen::HydrogenArchBase {
    private:
        WSADATA WSA_data;
        SOCKET Socket;
        //TODO: Implement timeout across socket map
        std::unordered_map<O2SocketID, struct Connected_Socket> Connected_Sockets;
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
        O2SocketID Get_Next_ID();

    public:
        O2Socket();
        O2Socket(WORD, int, int, int, int);
        void PortBind(string, int);
        void PortListen(int);
        O2SocketID AcceptNewConnection();
        char* Recieve(O2SocketID);
        void Send(O2SocketID, string);
        void CloseConnectedSocket(O2SocketID);
        void CloseConnectedSockets();
        void Close();
    };

    O2Socket::O2Socket() : HydrogenArchBase(){
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

    O2Socket::O2Socket(WORD WSAVersion, int Address_Family, int Type, int Protocol, int Read_Buffer_Size) : HydrogenArchBase() {
        Socket_Addr_In_Size = 0;
        Allowed_Backlog = 0;
        Socket_Bound = false;
        Socket_AF = Address_Family;
        Socket_Type = Type;
        Socket_Protocol = Protocol;
        Buffer_Size = Read_Buffer_Size;
        Buffer = new char[Read_Buffer_Size];
        ErrorCheck0((WSAStartup(MAKEWORD(2, 2), &WSA_data)), 1, "Failed WSAStartup");
        Socket = socket(Address_Family, Type, Protocol);
        if (Socket == INVALID_SOCKET) {
            cout << "Failed to create Socket" << endl;
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

        ErrorCheck0((bind(Socket, (SOCKADDR*)&Socket_Addr_In, Socket_Addr_In_Size)), 3, "Failed to bind socket");
        Socket_Bound = true;
    }

    void O2Socket::PortListen(int Backlog) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (!Socket_Bound) {
            ErrorAndDie(20, "Cannot listen to an unbound port");
        }
        Allowed_Backlog = Backlog;
        int Server_Backlog = Backlog;
        ErrorCheck0((listen(Socket, Backlog)), 4, "Could not start listening to socket");
    }
    //TODO: refactor so every connection holds a reading socket in Xeon_Base, you could extend this to an O2Socket_Set
    //TODO: do not simply use accept, use select and slisten to do this in a non binding way. 
    //Return message from a sepcified open connection
    char* O2Socket::Recieve(O2SocketID ID) {
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(11, ErrorMessage);
        }

        Connected_Sockets[ID].LastAccessed = time(NULL);

        memset(Connected_Sockets[ID].SocBuff, '\0', Buffer_Size);
        int BytesRead;
        BytesRead = recv(Connected_Sockets[ID].Soc, Connected_Sockets[ID].SocBuff, Buffer_Size, 0);
        if (BytesRead < 0) {
            ErrorAndDie(6, "Failed to read from socket");
        }
        return Connected_Sockets[ID].SocBuff;
    }

    //This can build a new Connected_Socket in the Connected_Sockets vector
    //That new Connected_Socket has its own buffer
    //This then returns the O2SocketID (int) back
    O2SocketID O2Socket::AcceptNewConnection() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        int BytesRead = 0;
        if (!Socket_Bound) {
            ErrorAndDie(21, "Cannot read from an unbound port");
        }
        SOCKET New_Socket = accept(Socket, (SOCKADDR*)&Socket_Addr_In, &Socket_Addr_In_Size);
        if (New_Socket == INVALID_SOCKET) {
            cout << "Failed to create usable socket" << endl;
            exit(5);
        }
        //Build the new Connected_Socket
        struct Connected_Socket NewConnectedSocket;
        char* SBuff = new char[Buffer_Size];
        NewConnectedSocket.Created = time(NULL);
        NewConnectedSocket.LastAccessed = NewConnectedSocket.Created;
        NewConnectedSocket.Soc = New_Socket;
        NewConnectedSocket.SocBuff = SBuff;
        NewConnectedSocket.State = -1; //TODO: Implement socket states
        O2SocketID NewID = Get_Next_ID();
        //Add the new Connected_Socket to Connected_Sockets
        Connected_Sockets[NewID] = NewConnectedSocket;
        return NewID;
    }

    //Basic send on the usable socket
    void O2Socket::Send(O2SocketID ID, string Message) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(13, ErrorMessage);
        }
        int BytesSent = 0;
        int TotalBytesSent = 0;
        while (TotalBytesSent < Message.size()) {
            BytesSent = send(Connected_Sockets[ID].Soc, Message.c_str(), Message.size(), 0);
            if (BytesSent < 0) {
                ErrorAndDie(7, "Failed to send");
            }
            TotalBytesSent += BytesSent;
        }
    }

    void O2Socket::CloseConnectedSocket(O2SocketID ID) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        //Find the socket in Usable_Sockets
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(12, ErrorMessage);
        }
        if (getsockopt(Connected_Sockets[ID].Soc, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(Connected_Sockets[ID].Soc)), 23, "Failed to close usable socket");
        }
        Connected_Sockets.erase(ID);
    }

    void O2Socket::CloseConnectedSockets() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        //Close all open sockets
        for (int i = 0; i < Connected_Sockets.size(); i++) {
            if (getsockopt(Connected_Sockets[i].Soc, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
                ErrorCheck0((closesocket(Connected_Sockets[i].Soc)), 23, "Failed to close usable socket");
            }
            if (Connected_Sockets[i].SocBuff != NULL) {
                delete Connected_Sockets[i].SocBuff;
            }
        }
        Connected_Sockets.clear();
    }

    void O2Socket::Close() {
        if (Connected_Sockets.size() > 0) {
            CloseConnectedSockets();
        }
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (getsockopt(Socket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(Socket)), 23, "Failed to close open socket");
        }
        ErrorCheck0(WSACleanup(), 24, "WSA Cleanup failed");
    }

    O2SocketID O2Socket::Get_Next_ID() {
        O2SocketID NewID;
        do {
            NewID = std::rand();
        } while (Connected_Sockets.find(NewID) != Connected_Sockets.end());
        return NewID;
    }
}