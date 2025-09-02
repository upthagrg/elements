/*
File: Oxygen.hpp
Author: Glenn Upthagrove
Last Updated: 02/18/2025
Description: This header file contains the Oxygen networking library for the Elements application framework.
This currently allows for the creation of an O2Socket object, which facilitiates inter-process and networked communication.
*/


#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include <signal.h>
#include <errno.h>
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
#pragma region O2Data
    class O2Data {
    private:
        vector<bin> data;
    public:
        O2Data();
        ~O2Data();
        O2Data(const O2Data&);
        int Size();
        bin& operator[](int);
        void operator=(const O2Data&);
        void AddData(bin);
        void AddData(string);
    };
    O2Data::O2Data() {}
    O2Data::~O2Data() { 
        data.clear(); 
    }
    O2Data::O2Data(const O2Data& src) {
        for (int i = 0; i < src.data.size(); i++) {
            data.push_back(src.data[i]);
        }
    }
    int O2Data::Size() {
        return data.size();
    }
    bin& O2Data::operator[](int i) {
        if (i < 0 || i > data.size()) {
            ErrorAndDie(1, "Array index outside the bounds of the array");
        }
        return data[i];
    }
    void O2Data::operator=(const O2Data& src) {
        for (int i = 0; i < src.data.size(); i++) {
            data.push_back(src.data[i]);
        }
    }
    void O2Data::AddData(bin in) {
        data.push_back(in);
    }
    void O2Data::AddData(string in) {
        bin tmp;
        tmp = in;
        data.push_back(tmp);
    }
#pragma endregion 

#pragma region O2Socket
    typedef int O2SocketID;
    enum SocketState {
        Unknown = 0,
        ReadyToRead = 1,
        DataSent = 2,
        TimedOut = 3
    };

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
        CRITICAL_SECTION ObjectLock;
        char Socket_Error[64] = { 0 };
        int Socket_Error_Size = 64;
        int Socket_AF;
        int Socket_Type;
        int Socket_Protocol;
        int Socket_Port;
        bool Socket_Bound;
        bool WSA_Started;
        struct sockaddr_in Socket_Addr_In;
        int Socket_Addr_In_Size;
        string Socket_IPAddr;
        int Allowed_Backlog;
        int Buffer_Size;
        bool Is_Valid;
        bool EndServer;
        bool O2Debug;
        struct timeval Timeout;
        struct fd_set master_set;
        struct fd_set working_set;
        SOCKET MaxSocket;

    public:
        O2Socket();
        O2Socket(WORD, int, int, int, int, bool);
        ~O2Socket();
        void PortBind(string, int);
        void PortListen(int);
        O2SocketID AcceptNewConnection();
        char* Recieve(O2SocketID);
        O2SocketID* GetNextRequest();
        void GetReadyRequests(int, bool*);
        void Send(O2SocketID, O2Data);
        void CloseConnectedSocket(O2SocketID);
        void CloseConnectedSockets();
        void Close();
        void Lock();
        void Unlock();
        CONDITION_VARIABLE* GetCondition();
        CRITICAL_SECTION* GetLock();
        char* GetSocketMessage(O2SocketID);
        void ToggleDebug();
    };
    //Default constructor of am O2Socket object. The object is unusable in this state
    O2Socket::O2Socket() : HydrogenArchBase() {
        InitializeCriticalSection(&ObjectLock);
        O2Debug = false;
        Socket_Addr_In_Size = -1;
        Allowed_Backlog = -1;
        Socket_Bound = false;
        WSA_Started = false;
        Socket_AF = -1;
        Socket_Type = -1;
        Socket_Protocol = -1;
        Buffer_Size = -1;
        Timeout.tv_sec = 0;
        Timeout.tv_usec = 0;
        Is_Valid = false;
    }
    //Constructor of a usable O2Socket object
    O2Socket::O2Socket(WORD WSAVersion, int Address_Family, int Type, int Protocol, int Read_Buffer_Size, bool Blocking) : HydrogenArchBase() {
        InitializeCriticalSection(&ObjectLock);
        O2Debug = false;
        Socket_Addr_In_Size = 0;
        Allowed_Backlog = 0;
        Socket_Bound = false;
        Socket_AF = Address_Family;
        Socket_Type = Type;
        Socket_Protocol = Protocol;
        Buffer_Size = Read_Buffer_Size;
        ErrorCheck0((WSAStartup(MAKEWORD(2, 2), &WSA_data)), 1, "Failed WSAStartup");
        WSA_Started = true;
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
    //Toggle Debug Flag
    void O2Socket::ToggleDebug() {
        O2Debug = !O2Debug;
    }

    //Bind the object's internal Socket to the port
    void O2Socket::PortBind(string IPAddress, int Port) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        Socket_IPAddr = "127.0.0.1";//loopback IP
        if (IPAddress.length() > 0) {
            Socket_IPAddr = IPAddress;
        }
        Socket_Addr_In.sin_family = Socket_AF;
        InetPton(AF_INET, IPAddress.c_str(), &Socket_Addr_In.sin_addr.s_addr);
        Socket_Port = Port;
        Socket_Addr_In.sin_port = htons(Port);
        Socket_Addr_In_Size = sizeof(Socket_Addr_In);
        if (!WSA_Started) {
            ErrorCheck0((WSAStartup(MAKEWORD(2, 2), &WSA_data)), 1, "Failed WSAStartup");
            WSA_Started = true;
        }
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
        int BytesRead;
        bool Retry;
        int RetryCount = 0;
        string Message = "";
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(11, ErrorMessage);
        }

        char* Return;
        Connected_Sockets[ID].LastAccessed = time(NULL);
        memset(Connected_Sockets[ID].SocBuff, '\0', Buffer_Size);
        Return = NULL;
        Retry = false;
        BytesRead = 0;
        Lock();
        BytesRead = recv(Connected_Sockets[ID].Soc, Connected_Sockets[ID].SocBuff, Buffer_Size, 0);
        Unlock();
        if (O2Debug) {
            Message.append("Recieve got ");
            Message.append(to_string(BytesRead));
            Message.append(" bytes from recv on ");
            //Sleep(500);//Use this sleep to slow down the server and diagnose issues
            Message.append(to_string(ID));
            MyBase.Display(Message);
            Message = "";
        }
        if (BytesRead <= 0) {
            int WSAError = WSAGetLastError();
            if (WSAError != WSAEWOULDBLOCK && Connected_Sockets[ID].State == DataSent) {
                if (O2Debug) {
                    MyBase.Display("recv returned 0 bytes, Data has been sent, and error is not WSAEWOULDBLOCK closing connection on server");
                }
                CloseConnectedSocket(ID);
            }
            Return = NULL;
        }
        else {
            Return = Connected_Sockets[ID].SocBuff;
        }

        char* ret = Return;
        return ret;
    }

    O2SocketID* O2Socket::GetNextRequest() {
        return (O2SocketID*)Requests.Dequeue();
    }

    //Select wrapper, returns false if timeout. Errors internally if there was an error from select. 
    void O2Socket::GetReadyRequests(int timeout, bool* extern_run) {
        int ret;
        bool recieve;
        string Message;
        O2SocketID newid;
        O2SocketID* q;
        if (O2Debug) {
            MyBase.Display("Started GetReadyRequests");
            Message.append("Listener Socket is ");
            Message.append(to_string(ListenerSocket));
            MyBase.Display(Message);
            Message = "";
        }
        //set timeout
        Timeout.tv_sec = timeout;
        char* enqueueing;
        //flags
        EndServer = false;
        //Initialize
        FD_ZERO(&master_set);
        MaxSocket = ListenerSocket;
        FD_SET(ListenerSocket, &master_set);

        do {
            recieve = false;
            Message = "";
            ret = 0;
            q = NULL;
            //Copy the master fd_set over to the working fd_set.
            memcpy(&working_set, &master_set, sizeof(master_set));
            //Call select() and wait the  timeout interval for it to complete.
            if (O2Debug) {
                MyBase.Display("Selecting");
            }
            ret = select(MaxSocket + 1, &working_set, NULL, NULL, &Timeout);
            //Handle return
            if (ret < 0) {
                //Error
                if (O2Debug) {
                    Message.append("Select failed with ");
                    Message.append(to_string(errno));
                    MyBase.Display(Message);
                    Message = "";
                }
                break;
                //ErrorAndDie(405, "Error on select");
            }
            else if (ret == 0) {
                //Timeout
                if (O2Debug) {
                    MyBase.Display("select timed out");
                }
            }
            else {
                //Sucess
                //Lock all requests
                Requests.Lock();
                //One or more descriptors are readable. Need to determine which ones they are.
                int desc_ready = ret; //Get the number of ready sockets
                for (int i = 0; i <= MaxSocket && desc_ready > 0; ++i) {
                    if (FD_ISSET(i, &working_set)) //This is in the working set, thus select said it is ready
                    {
                        //Reduce the number of descriptors to check by 1
                        desc_ready -= 1;
                        //This is the listener socket, that means we have new connections to accept
                        if (i == ListenerSocket) {
                            if (O2Debug) {
                                Message.append("New Connection to accept on ");
                                Message.append(to_string(i));
                                MyBase.Display(Message);
                                Message = "";
                            }
                            do {
                                newid = AcceptNewConnection();
                                if (newid <= 0)
                                {
                                    int error = WSAGetLastError();
                                    if (error != WSAEWOULDBLOCK)
                                    {
                                        if (O2Debug) {
                                            Message.append("AcceptNewConnection failed with ");
                                            Message.append(to_string(error));
                                            MyBase.Display(Message);
                                            Message = "";
                                        }
                                        EndServer = true;
                                    }
                                    break;
                                }
                                //Add the new socket to the master set
                                FD_SET(newid, &master_set);
                                //Update the max socket
                                if (newid > MaxSocket) {
                                    MaxSocket = newid;
                                }
                                //Loop back and accept another connections
                            } while (newid != -1);
                        }
                        //This is not the listener, thus we can recieve data instead of accept
                        else {
                            recieve = true;
                            do {
                                //Recieve data from the connection
                                char* enqueueing = Recieve(i);
                                recieve = false;
                                //If NULL, stop loop and do not enqueue the socket
                                if (enqueueing == NULL) {
                                    break;
                                }
                                else {
                                    //This is delted in the handle request after dequeue
                                    q = new O2SocketID;
                                    MyBase.AddPointer((void*)q, "Oxygen O2Socket::GetReadyRequests");
                                    *q = i;
                                    Requests.Enqueue((void*)q, false);
                                    break;
                                }
                            } while (recieve);
                        }
                    }
                }
                //Signal a worker thread
                Requests.Signal(false);
                Requests.Unlock();
            }
        } while (!EndServer && *extern_run);
    }

    //This can build a new Connected_Socket in the Connected_Sockets map
    //That new Connected_Socket has its own buffer
    //This then returns the O2SocketID (int) back
    O2SocketID O2Socket::AcceptNewConnection() {
        Lock();
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (!Socket_Bound) {
            ErrorAndDie(21, "Cannot read from an unbound port");
        }
        //SOCKET New_Socket = accept(ListenerSocket, (SOCKADDR*)&Socket_Addr_In, &Socket_Addr_In_Size);
        SOCKET New_Socket = accept(ListenerSocket, NULL, NULL);
        if (New_Socket == INVALID_SOCKET ||New_Socket <= 0 || New_Socket > MAXGETHOSTSTRUCT) {
            //Most likely -1, indicating would block, and thus no new connections
            return New_Socket;
        }
        //Build the new Connected_Socket
        struct Connected_Socket NewConnectedSocket;
        NewConnectedSocket.Created = time(NULL);
        NewConnectedSocket.LastAccessed = NewConnectedSocket.Created;
        NewConnectedSocket.Soc = New_Socket;
        NewConnectedSocket.SocBuff = new char[Buffer_Size]; //This gets deleted with delete[] when the connection is closed
        MyBase.AddPointer((void*)NewConnectedSocket.SocBuff, "Oxygen O2Socket::AcceptNewConnection");
        NewConnectedSocket.State = ReadyToRead;
        //Add the new Connected_Socket to Connected_Sockets map
        Connected_Sockets[New_Socket] = NewConnectedSocket;
        Unlock();
        O2SocketID ret = New_Socket;
        return ret;
    }

    //Send message over an open connection.
    void O2Socket::Send(O2SocketID ID, O2Data Data) {
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        if (Connected_Sockets.find(ID) == Connected_Sockets.end()) {
            if (O2Debug) {

            }
            string ErrorMessage = "No socket of ID: ";
            ErrorMessage.append(to_string(ID));
            ErrorMessage.append(" found in Connected_Sockets");
            ErrorAndDie(13, ErrorMessage);
        }
        int BytesSent;
        int TotalBytesSent;
        bool CanSend = true;
        string Message;

        for (int i = 0; i < Data.Size() && CanSend; i++) {
            BytesSent = 0;
            TotalBytesSent = 0;
            while (TotalBytesSent < Data[i].GetLength() && CanSend) {
                BytesSent = send(Connected_Sockets[ID].Soc, (char*)Data[i].GetData(), Data[i].GetLength(), 0);
                int error = WSAGetLastError();
                if (BytesSent < 0) {
                    if (error == WSAECONNABORTED) {
                        //Send error was caused by the client forcably closign the connection, we can ignore and drop this socket
                        if (O2Debug) {
                            Message = "";
                            Message.append("Client closed connection, cannot send data, closing conenction ");
                            Message.append(to_string(ID));
                        }
                        CloseConnectedSocket(ID);
                        CanSend = false;
                    }
                    else {
                        if (O2Debug) {
                            Message = "";
                            Message.append("Send failed\n");
                            Message.append("Error: ");
                            Message.append(to_string(error));
                            Message.append("\nID: ");
                            Message.append(to_string(ID));
                            Message.append("\nData Size: ");
                            Message.append(to_string(Data.Size()));
                            Message.append("\nI: ");
                            Message.append(to_string(i));
                            Message.append("\nData length: ");
                            Message.append(to_string(Data[i].GetLength()));
                            Message.append("\nData: ");
                            Message.append((char*)Data[i].GetData());
                            Message.append("\nTotalBytesSent: ");
                            Message.append(to_string(TotalBytesSent));
                            Message.append("\nBytesSent: ");
                            Message.append(to_string(BytesSent));
                            MyBase.Display(Message);
                        }
                        ErrorAndDie(7, "Failed to send");
                    }
                }
                TotalBytesSent += BytesSent;
            }
        }
        Connected_Sockets[ID].State = DataSent;
    }

    //Close an open connection
    void O2Socket::CloseConnectedSocket(O2SocketID ID) {
        Lock();
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
        MyBase.DeletePointer((void*)Connected_Sockets[ID].SocBuff);
        MyBase.Display("Deleting a socket buffer");
        uint64_t temp = (uint64_t)Connected_Sockets[ID].SocBuff;
        MyBase.Display(to_string(temp));
        delete[] Connected_Sockets[ID].SocBuff;
        MyBase.Display("Deleted");
        //remove record from the map of sockets
        Connected_Sockets.erase(ID);
        //remove from the Master set
        if (O2Debug) {
            string message = "";
            message.append("Removing ");
            message.append(to_string(ID));
            message.append(" From set");
            MyBase.Display(message);
        }
        FD_CLR(ID, &master_set);
        //Find new max socket
        if (ID == MaxSocket) {
            if (O2Debug) {
                MyBase.Display("max socket was ");
                MyBase.Display(to_string(MaxSocket));
            }
            while (FD_ISSET(MaxSocket, &master_set) == FALSE) {
                MaxSocket -= 1;
            }
            if (O2Debug) {
                MyBase.Display("new max socket ");
                MyBase.Display(to_string(MaxSocket));
            }
        }
        Unlock();
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
        if (!Is_Valid) { ErrorAndDie(100, "Bad Socket"); }
        memset(Socket_Error, '\0', Socket_Error_Size);
        if (getsockopt(ListenerSocket, SOL_SOCKET, SO_ERROR, Socket_Error, &Socket_Error_Size)) {
            ErrorCheck0((closesocket(ListenerSocket)), 23, "Failed to close open socket");
        }
        ErrorCheck0(WSACleanup(), 24, "WSA Cleanup failed");
        WSA_Started = false;
    }
    //Lock for object operations
    void O2Socket::Lock() {
        EnterCriticalSection(&ObjectLock);
    }
    //Unlock for object operations
    void O2Socket::Unlock() {
        LeaveCriticalSection(&ObjectLock);
    }
    CONDITION_VARIABLE* O2Socket::GetCondition() {
        return Requests.GetCondition();
    }
    CRITICAL_SECTION* O2Socket::GetLock() {
        return Requests.GetLock();
    }
    char* O2Socket::GetSocketMessage(O2SocketID ID) {
        if (Connected_Sockets.find(ID) != Connected_Sockets.end()) {
            return Connected_Sockets[ID].SocBuff;
        }
        else {
            //ErrorAndDie(1002, "Cannot find socket in set of sockets to get message from");
            return NULL;
        }
    }
#pragma endregion

#pragma region O2HTTPEngine
    enum HTTPContentType
    {
        HTML = 0,
        JPEG = 1,
        PNG = 2,
        ICO = 3,
        MP4 = 4

    };

    enum HTTPStatus
    {
        OK = 200,
        NOTFOUND = 404
    };

    enum HTTPVersion
    {
        HTTP1 = 1,
        HTTP11 = 11,
        HTTP2 = 2
    };

    enum HTTPConnection
    {
        CLOSED = 1,
        KEEPALIVE = 2
    };

    class HTTPEngine {
    private:
        HTTPVersion CurrentVersion;
        string Version;
        string ServerVersion;
        HTTPStatus CurrentStatus;
        string StatusOK;
        string StatusNOTFOUND;
        HTTPContentType CurrentContentType;
        uint64_t ContentLength;
        HTTPConnection CurrentConnection;

    public:
        HTTPEngine();
        ~HTTPEngine();
        HTTPEngine(const HTTPEngine&);
        void SetHTTPVersion(HTTPVersion);
        void SetServerVersion(string);
        void SetStatus(HTTPStatus);
        void SetContentType(HTTPContentType);
        void SetContentLength(uint64_t);
        void SetConnection(HTTPConnection);
        string BuildHttpMessage();
        string BuildHttpMessage(bool);
    };

    HTTPEngine::HTTPEngine() {
        CurrentVersion = HTTP11;
        Version = "HTTP/1.1";
        ServerVersion = "Unknown";
        CurrentStatus = OK;
        StatusOK = "200 OK\n";
        StatusNOTFOUND = "404 Not Found\n";
        CurrentContentType = HTML;
        ContentLength = 0;
        CurrentConnection = CLOSED;

    }
    HTTPEngine::~HTTPEngine() {

    }
    HTTPEngine::HTTPEngine(const HTTPEngine& src) {
        CurrentVersion = src.CurrentVersion;
        Version = src.Version;
        ServerVersion = src.ServerVersion;
        StatusOK = src.StatusOK;
        StatusNOTFOUND = src.StatusNOTFOUND;
        CurrentStatus = src.CurrentStatus;
        CurrentContentType = src.CurrentContentType;
        ContentLength = src.ContentLength;
        CurrentConnection = src.CurrentConnection;
    }

    void HTTPEngine::SetHTTPVersion(HTTPVersion ver) {
        CurrentVersion = ver;
        switch (CurrentVersion) {
        case HTTP1:
            Version = "HTTP/1";
            break;
        case HTTP11:
            Version = "HTTP/1.1";
            break;
        case HTTP2:
            Version = "HTTP/2";
            break;
        default:
            CurrentVersion = HTTP11;
            Version = "HTTP/1.1";
            break;
        }
    }
    void HTTPEngine::SetServerVersion(string ver) {
        ServerVersion = ver;
    }
    void HTTPEngine::SetStatus(HTTPStatus status) {
        CurrentStatus = status;
    }
    void HTTPEngine::SetContentType(HTTPContentType type) {
        CurrentContentType = type;
    }
    void HTTPEngine::SetContentLength(uint64_t len) {
        ContentLength = len;
    }
    void HTTPEngine::SetConnection(HTTPConnection con) {
        CurrentConnection = con;
    }
    string HTTPEngine::BuildHttpMessage() {
        return BuildHttpMessage(false);
    }
    string HTTPEngine::BuildHttpMessage(bool SendHTML404PageBack) {
        //Start with the HTTP Version String
        string message = Version;
        message.append(" ");

        //Add Status
        switch (CurrentStatus) {
        case OK:
            message.append(StatusOK);
            break;
        case NOTFOUND:
            message.append(StatusNOTFOUND);
            break;
        default:
            message.append(StatusNOTFOUND);
            break;
        }

        //Add Date
        message.append("Date: ");
        //Capture current time
        string GMTTimeString = "";
        char* TimeString = new char[26];
        memset(TimeString, '\0', 26);
        tm GMTTime;
        time_t Now = time(NULL);
        gmtime_s(&GMTTime, &Now);
        asctime_s(TimeString, 26, &GMTTime);
        for (int i = 0; i < 26; i++) {
            if (TimeString[i] == '\n') {
                TimeString[i] = ' ';
                break;
            }
        }
        message.append(TimeString);
        delete[] TimeString;
        message.append("GMT\n");

        //Add Server Info
        message.append("Server: ");
        message.append(ServerVersion);
        message.append("\n");

        if (CurrentStatus != NOTFOUND || SendHTML404PageBack) {
            //Add Content Type
            message.append("Content-Type: ");
            switch (CurrentContentType) {
            case HTML:
                message.append("text/html");
                break;
            case JPEG:
                message.append("image/jpeg");
                break;
            case PNG:
                message.append("image/png");
                break;
            case ICO:
                message.append("image/vnd.microsoft.icon");
                break;
            case MP4:
                message.append("video/mp4");
                break;
            default:
                ErrorAndDie(104, "Unknown content type");
                break;
            }

            //Add Content Length
            message.append("\nContent-Length: ");
            message.append(to_string(ContentLength));

            //Add Connection
            if (CurrentConnection == KEEPALIVE) {
                message.append("\nConnection: keep-alive");
            }
            else {
                message.append("\nConnection: close");
            }
            message.append("\n\n");
        }
        string ret = message;
        return ret;
    }
#pragma endregion

#pragma region JSONObject
    class JSONObject {
    private:
        const char ObjectStart = '{';
        const char ObjectEnd = '}';
        const char PropertyValueSeperator = ':';
        const char Seperator = ',';
        const char ArrayStart = '[';
        const char ArrayEnd = ']';
        const string NullValue = "null";
        const string TokenDelims = ":,\"   \n";
        std::unordered_map<string, string> Items;
        vector<string> ItemsList;
    public:
        JSONObject();
        JSONObject(string);
        JSONObject(const JSONObject&);
        ~JSONObject();
        void Parse(string);
        void Parse(char*);
        void AddItem(string, string);
        vector<string> GetItems();
        string Stringify();
        void PrintItems();
        void PrintValues();
        void Print();
        string& operator[](string);
        JSONObject operator=(const JSONObject&);
        bool Find(string);
    };
    //Default constructor
    JSONObject::JSONObject() {}
    //Non-Default constructor
    JSONObject::JSONObject(string data) {
        this->Parse(data);
    }
    //Copy constructor
    JSONObject::JSONObject(const JSONObject& obj) {
        Items = obj.Items;
        ItemsList = obj.ItemsList;
    }
    //Destructor
    JSONObject::~JSONObject() {}

    //Build new object from string
    void JSONObject::Parse(string data) {
        string workingdata = data;
        //vector<string> Tokens;
        vector<StringTokenWithIndex> Tokens;
        vector<string> Filter;
        Items.clear();
        ItemsList.clear();
        Tokens = TokenizeStringWithIndex(workingdata, TokenDelims.c_str(), Filter);
        //Tokens = TokenizeString(data, TokenDelims.c_str(), Filter);
        if (Tokens.size() > 0) {
            if (Tokens[0].Token[0] == ObjectStart) {
                int i = 1;
                //Loop accross all Tokens
                while (i < Tokens.size()) {
                    //look at the next item, if it isn't an object start or stop orelse an array start or stop, this pair is a simple item value pair
                    if (i + 1 < Tokens.size()) {
                        if ((Tokens[i + 1].Token[0] != ObjectStart && Tokens[i + 1].Token[0] != ObjectEnd && Tokens[i + 1].Token[0] != ArrayStart && Tokens[i + 1].Token[0] != ArrayEnd)) {
                            //last end object character needs to be trimmed in some cases since it isnt a part of tokenizing
                            if (Tokens[i + 1].Token[Tokens[i + 1].Token.size()-1] == ObjectEnd) {
                                Tokens[i + 1].Token.pop_back();
                            }
                            AddItem(Tokens[i].Token, Tokens[i + 1].Token);
                            i = i + 2;
                        }
                        //Object start detected, look for object end
                        else if (Tokens[i + 1].Token[0] == ObjectStart) {
                            string SubObject;
                            int ObjectStartsCount = 1;
                            //the next Token is an object start, loop accross all tokens and their strings until the end is found. 
                            for (int j = i + 2; j < Tokens.size(); j++) {
                                //loop accross the string
                                for (int k = 0; k < Tokens[j].Token.size(); k++) {
                                    if (Tokens[j].Token[k] == ObjectStart) {
                                        ObjectStartsCount++;
                                    }
                                    else if (Tokens[j].Token[k] == ObjectEnd) {
                                        ObjectStartsCount--;
                                    }

                                    //End found
                                    if (ObjectStartsCount == 0) {
                                        SubObject = workingdata.substr(Tokens[i + 1].Index, ((Tokens[j].Index + k + 1) - (Tokens[i + 1].Index)));
                                        AddItem(Tokens[i].Token, SubObject);
                                        workingdata = workingdata.substr(Tokens[j].Index + k + 1, workingdata.size() - (Tokens[j].Index + k));
                                        Tokens = TokenizeStringWithIndex(workingdata, TokenDelims.c_str(), Filter);
                                        i = 0;
                                        break;
                                    }
                                }
                            }
                        }
                        //Array start detected, look for object end
                        else if (Tokens[i + 1].Token[0] == ArrayStart) {
                            string SubArray;
                            int ArrayStartsCount = 1;
                            //the next Token is an object start, loop accross all tokens and their strings until the end is found. 
                            for (int j = i + 2; j < Tokens.size(); j++) {
                                //loop accross the string
                                for (int k = 0; k < Tokens[j].Token.size(); k++) {
                                    if (Tokens[j].Token[k] == ArrayStart) {
                                        ArrayStartsCount++;
                                    }
                                    else if (Tokens[j].Token[k] == ArrayEnd) {
                                        ArrayStartsCount--;
                                    }

                                    //End found
                                    if (ArrayStartsCount == 0) {
                                        SubArray = workingdata.substr(Tokens[i + 1].Index, ((Tokens[j].Index + k + 1) - (Tokens[i + 1].Index)));
                                        AddItem(Tokens[i].Token, SubArray);
                                        workingdata = workingdata.substr(Tokens[j].Index + k + 1, workingdata.size() - (Tokens[j].Index + k));
                                        Tokens = TokenizeStringWithIndex(workingdata, TokenDelims.c_str(), Filter);
                                        i = 0;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else {
                        break;
                    }
                }
            }
            else {
                    ErrorAndDie(1, "Invalid JSON Object");
            }
        }
        else {
                ErrorAndDie(2, "Empty JSON Object");
        }
    }
    //Build new object from C string
    void JSONObject::Parse(char* Data) {
        string tmp = Data;
        Parse(tmp);
    }
    //Individual updates
    void JSONObject::AddItem(string Item, string Value) {
        Items[Item] = Value;
        ItemsList.push_back(Item);
    }
    //Getters
    vector<string> JSONObject::GetItems() {
        return ItemsList;
    }
    string JSONObject::Stringify() {
        std::unordered_map<string, string>::iterator It = Items.begin();
        string JSONBuilder = "{";
        while (It != Items.end()) {
            JSONBuilder.append("\"");
            JSONBuilder.append(It->first);
            JSONBuilder.append("\"");
            JSONBuilder.append(":");
            if (It->second[0] == ObjectStart || It->second[0] == ArrayStart) {
                JSONBuilder.append(It->second);
            }
            else {
                JSONBuilder.append("\"");
                JSONBuilder.append(It->second);
                JSONBuilder.append("\"");
            }
            JSONBuilder.append(",");
            It++;
        }
        if (JSONBuilder[JSONBuilder.size()-1] == ',') {
            JSONBuilder[JSONBuilder.size()-1] = '}';
        }
        else {
            JSONBuilder.append("}");
        }
        string FinalJSON = JSONBuilder;
        return FinalJSON;
    }
    //Other operations
    bool JSONObject::Find(string Item) {
        bool Found = false;
        for (int i = 0; i < ItemsList.size(); i++) {
            if (ItemsList[i] == Item) {
                Found = true;
                break;
            }
        }
        bool ret = Found;
        return ret;
    }
    //Printers
    void JSONObject::PrintItems() {
        for (int i = 0; i < ItemsList.size(); i++) {
            MyBase.Display(ItemsList[i]);
        }
    }
    void JSONObject::PrintValues() {
        std::unordered_map<string, string>::iterator It = Items.begin();
        while (It != Items.end()) {
            MyBase.Display(It->second);
            It++;
        }
    }
    void JSONObject::Print() {
        std::unordered_map<string, string>::iterator It = Items.begin();
        string Message;
        while (It != Items.end()) {
            Message = "";
            Message.append(It->first);
            Message.append(":");
            Message.append(It->second);
            MyBase.Display(Message);
            It++;
        }
    }

    //operators
    string& JSONObject::operator[](string Item) {
        if (!Find(Item)) {
            ErrorAndDie(3, "Invalid Item Access");
        }
        return Items[Item];
    }
    JSONObject JSONObject::operator=(const JSONObject& obj) {
        return JSONObject(obj);
    }
#pragma endregion

#pragma region JSONFile
    class JSONFile {
    private:
        JSONObject JSON;
        HFile File;
    public:
        JSONFile();
        JSONFile(string);
        JSONFile(JSONObject, string);
        JSONFile(const JSONFile&);
        ~JSONFile();
        string GetPath();
        void SetJSON(JSONObject);
        void SetJSON(string);
        void SetJSON(char*);
        void Write();
        JSONObject GetJSON();
    };
    //Creates a JSONFile
    JSONFile::JSONFile() {}
    //Creates a JSONFile that reads from the given file
    JSONFile::JSONFile(string pFile) {
        File = HFile(pFile);
        if ((File.DataString()).size() > 0) {
            JSON.Parse(File.DataString());
        }
    }
    //Creates a JSONFile with the given data and will use the given file, but does not read or write immediatly. 
    JSONFile::JSONFile(JSONObject Data, string pFile) {
        File = HFile();
        File.SetPath(pFile);
        JSON = Data;
    }
    //Copy Contructor
    JSONFile::JSONFile(const JSONFile& Data) {
        File = Data.File;
        JSON = Data.JSON;
    }
    //Destructor
    JSONFile::~JSONFile() {}
    //Returns the file path of this JSONFile object.
    string JSONFile::GetPath() {
        return File.GetPath();
    }
    void JSONFile::SetJSON(JSONObject Data) {
        JSON = Data;
    }
    void JSONFile::SetJSON(string Data) {
        JSON.Parse(Data);
    }
    void JSONFile::SetJSON(char* Data) {
        JSON.Parse(Data);
    }
    void JSONFile::Write() {
        File.Write(JSON.Stringify());
    }
    JSONObject JSONFile::GetJSON() {
        return JSON;
    }
#pragma endregion
#pragma region JSONArray
#pragma endregion
#pragma region JSONArrayFile
#pragma endregion
}