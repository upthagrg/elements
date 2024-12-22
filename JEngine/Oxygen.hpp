/*
File: Oxygen.hpp
Author: Glenn Upthagrove
Last Updated: 01/21/2024
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
        ICO = 3
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
        std::regex File_Extension_Regex;
        std::regex File_Extension_jpg_Regex;
        std::regex File_Extension_png_Regex;
        std::regex File_Extension_ico_Regex;

    public:
        HTTPEngine();
        ~HTTPEngine();
        HTTPEngine(const HTTPEngine&);
        void SetHTTPVersion(HTTPVersion);
        void SetServer(string);
        void SetStatus(HTTPStatus);
        void SetContentType(HTTPContentType);
        bool SetContentType(string);
        bool SetContentType(vector<string>);
        void SetContentLength(uint64_t);
        void SetConnection(HTTPConnection);
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
        File_Extension_Regex.assign("^[^\s]+\.(html|jpg|jpeg|png|ico)$");
        File_Extension_jpg_Regex.assign("^[^\s]+\.(jpg|jpeg)$");
        File_Extension_png_Regex.assign("^[^\s]+\.(png)$");
        File_Extension_ico_Regex.assign("^[^\s]+\.(ico)$");
    }
    HTTPEngine::~HTTPEngine() {

    }
    HTTPEngine::HTTPEngine(const HTTPEngine& src) {
        Version = src.Version;
        ServerVersion = src.ServerVersion;
        StatusOK = src.StatusOK;
        StatusNOTFOUND = src.StatusNOTFOUND;
        File_Extension_Regex = src.File_Extension_Regex;
        File_Extension_jpg_Regex = src.File_Extension_jpg_Regex;
        File_Extension_png_Regex = src.File_Extension_png_Regex;
        File_Extension_ico_Regex = src.File_Extension_ico_Regex;
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
        bool AllowInvalid = false;
    public:
        JSONObject();
        JSONObject(string);
        JSONObject(string, bool);
        JSONObject(JSONObject& obj);
        ~JSONObject();
        void Parse(string);

    };
    JSONObject::JSONObject() {}
    JSONObject::JSONObject(string data) {
        this->Parse(data);
    }
    JSONObject::JSONObject(string data, bool pAllowInvalid) {
        AllowInvalid = pAllowInvalid;
        this->Parse(data);
    }
    JSONObject::JSONObject(JSONObject& obj) {}
    JSONObject::~JSONObject() {}

    void JSONObject::Parse(string data) {
        string workingdata = data;
        //vector<string> Tokens;
        vector<StringTokenWithIndex> Tokens2;
        vector<string> Filter;
        Tokens2 = TokenizeStringWithIndex(workingdata, TokenDelims.c_str(), Filter);
        //Tokens = TokenizeString(data, TokenDelims.c_str(), Filter);
        if (Tokens2.size() > 0) {
            if (Tokens2[0].Token[0] == ObjectStart) {
                int i = 1;
                //Loop accross all Tokens
                while (i < Tokens2.size()) {
                    //look at the next item, if it isn't an object start or stop, this pair is a simple item value pair
                    if (i + 1 < Tokens2.size()) {
                        if ((Tokens2[i + 1].Token[0] != ObjectStart && Tokens2[i + 1].Token[0] != ObjectEnd)) {
                            Items[Tokens2[i].Token] = Tokens2[i + 1].Token;
                            i = i + 2;
                        }
                        //Object start detected, look for object end
                        else if (Tokens2[i + 1].Token[0] == ObjectStart) {
                            string SubObject;
                            int ObjectStartsCount = 1;
                            //the next Token is an object start, loop accross all tokens and their strings until the end is found. 
                            cout << "Object start found at " << Tokens2[i + 1].Index << endl;
                            for (int j = i + 2; j < Tokens2.size(); j++) {
                                //loop accross the string
                                for (int k = 0; k < Tokens2[j].Token.size(); k++) {
                                    cout << "Checking: " << Tokens2[j].Token[k] << endl;
                                    if (Tokens2[j].Token[k] == ObjectStart) {
                                        ObjectStartsCount++;
                                    }
                                    else if (Tokens2[j].Token[k] == ObjectEnd) {
                                        ObjectStartsCount--;
                                    }

                                    //End found
                                    if (ObjectStartsCount == 0) {
                                        cout << "end found at " << Tokens2[j].Index + k << endl;
                                        SubObject = workingdata.substr(Tokens2[i + 1].Index, ((Tokens2[j].Index + k + 1) - (Tokens2[i + 1].Index)));
                                        cout << "Object " << SubObject << endl;
                                        Items[Tokens2[i].Token] = SubObject;
                                        workingdata = workingdata.substr(Tokens2[j].Index + k + 1, workingdata.size() - (Tokens2[j].Index + k));
                                        Tokens2 = Tokens2 = TokenizeStringWithIndex(workingdata, TokenDelims.c_str(), Filter);
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
                if (!AllowInvalid) {
                    ErrorAndDie(1, "Invalid JSON Object");
                }
            }
        }
        else {
            if (!AllowInvalid) {
                ErrorAndDie(2, "Empty JSON Object");
            }
        }
    }
#pragma endregion
}