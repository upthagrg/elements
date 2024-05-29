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
        QUEUE Requests;
        WSADATA WSA_data;
        SOCKET ListenerSocket;
        std::unordered_map<O2SocketID, struct Connected_Socket> Connected_Sockets;
        std::mutex ObjectLock;
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
        bool selectstop;
        struct timeval Timeout;
        struct fd_set master_set;
        struct fd_set working_set;
        SOCKET MaxSocket;
        O2SocketID Get_Next_ID();

    public:
        O2Socket();
        O2Socket(WORD, int, int, int, int, bool);
        ~O2Socket();
        void PortBind(string, int);
        void PortListen(int);
        O2SocketID AcceptNewConnection();
        char* Recieve(O2SocketID);
        O2SocketID* GetNextRequest();
        bool Select(int, bool*);
        void Send(O2SocketID, string);
        void CloseConnectedSocket(O2SocketID);
        void CloseConnectedSockets();
        void Close();
        void Lock();
        void Unlock();
        CONDITION_VARIABLE* GetCondition();
        CRITICAL_SECTION* GetLock();
    };
    //Default constructor of am O2Socket object. The object is unusable in this state
    O2Socket::O2Socket() : HydrogenArchBase() {
        Socket_Addr_In_Size = -1;
        Allowed_Backlog = -1;
        Socket_Bound = false;
        Socket_AF = -1;
        Socket_Type = -1;
        Socket_Protocol = -1;
        Buffer_Size = -1;
        Buffer = NULL;
        Timeout.tv_sec = 0;
        Timeout.tv_usec = 0;
        Is_Valid = false;
    }
    //Constructor of a usable O2Socket object
    O2Socket::O2Socket(WORD WSAVersion, int Address_Family, int Type, int Protocol, int Read_Buffer_Size, bool Blocking) : HydrogenArchBase() {
        Socket_Addr_In_Size = 0;
        Allowed_Backlog = 0;
        Socket_Bound = false;
        Socket_AF = Address_Family;
        Socket_Type = Type;
        Socket_Protocol = Protocol;
        Buffer_Size = Read_Buffer_Size;
        Buffer = new char[Read_Buffer_Size];
        ErrorCheck0((WSAStartup(MAKEWORD(2, 2), &WSA_data)), 1, "Failed WSAStartup");
        ListenerSocket = socket(Address_Family, Type, Protocol);
        if (ListenerSocket == INVALID_SOCKET) {
            ErrorAndDie(2, "Failed to create Socket");
        }
        //Set Listener socket to reusable
        int on = 1;
        int rc = setsockopt(ListenerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
        //Make Socket non-blocking
        if (!Blocking) {
            unsigned long ul = 1;
            int nRet;
            nRet = ioctlsocket(ListenerSocket, FIONBIO, (unsigned long*)&ul);
            if (nRet == SOCKET_ERROR) {
                ErrorAndDie(406, "Failed to put Socket in non-blocking mode");
            }
        }
        Timeout.tv_sec = 0;
        Timeout.tv_usec = 0;
        Is_Valid = true;
    }
    //O2Socket Destructor
    O2Socket::~O2Socket() {
        //Delete internal buffer
        if (Buffer != NULL) {
            delete Buffer;
        }
        if (Connected_Sockets.size() > 0) {
            for (int i = 0; i < Connected_Sockets.size(); i++) {
                //delete every socket struct's buffer
                if (Connected_Sockets[i].SocBuff != NULL) {
                    delete Connected_Sockets[i].SocBuff;
                }
            }
            //clear the map of sockets
            Connected_Sockets.clear();
        }
    }

    //Bind the object's internal Socket to the port
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

        ErrorCheck0((bind(ListenerSocket, (SOCKADDR*)&Socket_Addr_In, Socket_Addr_In_Size)), 3, "Failed to bind socket");
        Socket_Bound = true;
    }
    //Begin listening on the object's internal Socket
    void O2Socket::PortListen(int Backlog) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (!Socket_Bound) {
            ErrorAndDie(20, "Cannot listen to an unbound port");
        }
        Allowed_Backlog = Backlog;
        int Server_Backlog = Backlog;
        ErrorCheck0((listen(ListenerSocket, Backlog)), 4, "Could not start listening to socket");
    }

    //Return message from a sepcified open connection
    char* O2Socket::Recieve(O2SocketID ID) {
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(11, ErrorMessage);
        }
        char* Return = NULL;
        Connected_Sockets[ID].LastAccessed = time(NULL);

        memset(Connected_Sockets[ID].SocBuff, '\0', Buffer_Size);
        int BytesRead;
        BytesRead = recv(Connected_Sockets[ID].Soc, Connected_Sockets[ID].SocBuff, Buffer_Size, 0);
        if (BytesRead <= 0) {
            //Connection no longer valid < 0 or closed by client == 0 
            CloseConnectedSocket(ID);
            Return = NULL;
        }
        else {
            Return = Connected_Sockets[ID].SocBuff;
        }
        return Return;
    }

    O2SocketID* O2Socket::GetNextRequest() {
        return (O2SocketID*)Requests.Dequeue();
    }
    //Select wrapper, returns false if timeout. Errors internally if there was an error from select. 
    bool O2Socket::Select(int timeout, bool* extern_stop) {
        //set timeout
        Timeout.tv_sec = timeout;
        //flags
        selectstop = false;
        //Initialize
        FD_ZERO(&master_set);
        MaxSocket = ListenerSocket;
        FD_SET(ListenerSocket, &master_set);
        do {
            //Copy the master fd_set over to the working fd_set.
            memcpy(&working_set, &master_set, sizeof(master_set));
            //Call select() and wait the  timeout interval for it to complete.
            int ret = select(MaxSocket + 1, &working_set, NULL, NULL, &Timeout);
            //Handle return
            switch (ret) {
            case -1:
                //Error
                ErrorAndDie(405, "Error on select");
            case 0:
                //Timeout
                return false;
            default:
                //Sucess
                //One or more descriptors are readable. Need to determine which ones they are.
                int desc_ready = ret;
                O2SocketID* newid = NULL;
                Requests.Lock();
                for (int i = 0; i <= MaxSocket && desc_ready > 0; ++i)
                {
                    //Check to see if this descriptor is ready
                    if (FD_ISSET(i, &working_set))
                    {
                        //Found a readable file, the number we need to find in total is now one less
                        desc_ready -= 1;
                        //Check to see if this is the listening socket
                        if (i == ListenerSocket) {
                            //New connection found
                            do {
                                //Accept each incoming connection. If accept fails with EWOULDBLOCK, 
                                //then we have accepted all of them. Any otherfailure on accept will cause us to end the server.
                                newid = new O2SocketID;
                                (*newid) = this->AcceptNewConnection();
                                if (*newid < 0) {
                                    if (*newid != EWOULDBLOCK) {
                                        selectstop = true;
                                    }
                                    break;
                                }
                                else {
                                    //enqueue the newid
                                    Requests.Enqueue((void*)newid, false);
                                    //add to master read set
                                    FD_SET((*newid), &master_set);
                                    //update max id if larger than the current max id
                                    if ((*newid) > MaxSocket) {
                                        MaxSocket = (*newid);
                                    }
                                }
                            } while (*newid != -1); //loop back and accept another incoming connection
                        }
                        else {
                            //This is an open connection,  so an existing connection is readable. We do not need to call accept.
                            //enqueue existing descriptor again
                            if (Connected_Sockets.find(i) != Connected_Sockets.end()) {
                                Connected_Sockets[i].LastAccessed = time(NULL);
                                Requests.Enqueue((void*)&(Connected_Sockets[i].Soc), false);
                            }

                        }
                    }
                }
                Requests.Unlock();
                //Requests.Signal(false);
                return true;
            }
            if (extern_stop != NULL && *extern_stop == true) {
                selectstop = true;
                break;
            }
        } while (selectstop == false);
    }

    //This can build a new Connected_Socket in the Connected_Sockets map
    //That new Connected_Socket has its own buffer
    //This then returns the O2SocketID (int) back
    O2SocketID O2Socket::AcceptNewConnection() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        int BytesRead = 0;
        if (!Socket_Bound) {
            ErrorAndDie(21, "Cannot read from an unbound port");
        }
        SOCKET New_Socket = accept(ListenerSocket, (SOCKADDR*)&Socket_Addr_In, &Socket_Addr_In_Size);
        if (New_Socket < 0) {
            return New_Socket;
        }
        //Build the new Connected_Socket
        struct Connected_Socket NewConnectedSocket;
        char* SBuff = new char[Buffer_Size];
        NewConnectedSocket.Created = time(NULL);
        NewConnectedSocket.LastAccessed = NewConnectedSocket.Created;
        NewConnectedSocket.Soc = New_Socket;
        NewConnectedSocket.SocBuff = SBuff;
        NewConnectedSocket.State = -1; //TODO: Implement socket states
        O2SocketID NewID = New_Socket;
        //Add the new Connected_Socket to Connected_Sockets
        Connected_Sockets[NewID] = NewConnectedSocket;
        return NewID;
    }

    //Send message over an open connection.
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

    //Close an open connection
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
            ErrorCheck0((closesocket(Connected_Sockets[ID].Soc)), 24, "Failed to close an open connection");
        }
        //free the socket buffer
        delete Connected_Sockets[ID].SocBuff;
        //remove record from the map of sockets
        Connected_Sockets.erase(ID);
        //remove from the Master set
        FD_CLR(ID, &master_set);
        //Find new max socket
        if (ID == MaxSocket) {
            Lock();//Do not let multiple threads update max socket at once
            while (FD_ISSET(MaxSocket, &master_set) == FALSE) {
                MaxSocket -= 1;
            }
            Unlock();
        }

    }
    //Close all open connections
    void O2Socket::CloseConnectedSockets() {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        //Close all open sockets
        for (int i = 0; i < Connected_Sockets.size(); i++) {
            if (getsockopt(Connected_Sockets[i].Soc, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
                ErrorCheck0((closesocket(Connected_Sockets[i].Soc)), 23, "Failed to close an open connection");
            }
            if (Connected_Sockets[i].SocBuff != NULL) {
                delete Connected_Sockets[i].SocBuff;
            }
        }
        Connected_Sockets.clear();
    }
    //Close the socket. Closes all open connections, then closes the listening socket, then calls WSACleanup. 
    void O2Socket::Close() {
        //TODO: figure out perfect closing
        //if (Connected_Sockets.size() > 0) {
        //    CloseConnectedSockets();
        //}
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (getsockopt(ListenerSocket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(ListenerSocket)), 23, "Failed to close open socket");
        }
        ErrorCheck0(WSACleanup(), 24, "WSA Cleanup failed");
    }
    //Get the next unused Socket ID
    O2SocketID O2Socket::Get_Next_ID() {
        O2SocketID NewID;
        do {
            NewID = std::rand();
        } while (Connected_Sockets.find(NewID) != Connected_Sockets.end());
        return NewID;
    }
    //Lock for object operations
    void O2Socket::Lock() {
        ObjectLock.lock();
    }
    //Unlock for object operations
    void O2Socket::Unlock() {
        ObjectLock.unlock();
    }
    CONDITION_VARIABLE* O2Socket::GetCondition() {
        return Requests.GetCondition();
    }
    CRITICAL_SECTION* O2Socket::GetLock() {
        return Requests.GetLock();
    }
}