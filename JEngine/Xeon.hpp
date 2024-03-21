#pragma once
/*
File: Xeon.hpp
Author: Glenn Upthagrove
Last Updated: 01/21/2024
Description: This header file contains the Xeon Web server and local harness of the Elements appication framework. 
These are built off off the Oxygen networking library. And the Hydrogen archiecture library.
This can act as a web server for a website, local testing of the website, or as a local GUI for an Elements application. 
This will allow for much more control over what the web server can do allowing a developer to extend past the basics of a web server and inject their own alterations to returned content. 
This will be extended in several ways to include many many more capabilitites and improvements. 
One of these future abilities will be a dynamicly genreated HTML engine (maybe it will be carbon, currently it is WebDoc.hpp) which will allow a web application that is not simply a delivery system of HTML or PHP.
*/

#include <regex>
#include <thread>
#include "Hydrogen.hpp"
#include "Oxygen.hpp"
#include "curl/curl.h"

namespace Xeon {
#pragma region Base
    static const string SERVER_VERSION = "Xeon/0.3.1 (Win64)";
    class Xeon_Base;
    void Handle_Request(Xeon_Base*);
    class Xeon_Base {
    public:
        explicit Xeon_Base();
        explicit Xeon_Base(string, string, int, int, int, int, bool);
        ~Xeon_Base();
        virtual void Start();
        virtual void Stop();
        virtual bool IsRunning() { return Running; }
        bool IsStarted() { return Run; }
        virtual string BuildResponse(string) { return ""; }
        virtual char* Recieve(O2::O2SocketID);
        virtual void SendResponse(O2::O2SocketID, string);
        O2::O2SocketID* GetNextRequest();
        void CloseConnectedSocket(O2::O2SocketID);
        void Lock();
        void Unlock();
        void DebugMessage(string);

    protected:
        O2::O2Socket* Server_Socket;
        bool Run;
        bool Running;
        bool OpenToLANAddress;
        bool OpenToLANPort;
        string LAN_Address;
        string Addr;
        int Port;
        int Allowed_Backlog;
        int Buffer_Size;
        bool Debug;
        std::mutex ObjectLock;
        int WorkerThreads;
        vector<std::thread> Workers;
    };
    //Default constructor of Xeon_Base objects, this would be an unusable object in this state
    Xeon_Base::Xeon_Base() {
        Run = false;
        Running = false;
        OpenToLANAddress = false;
        OpenToLANPort = false;
        Addr = "";
        Port = -1;
        Allowed_Backlog = -1;
        Buffer_Size = -1;
        Debug = false;
        Server_Socket = new O2::O2Socket();
        WorkerThreads = 0;
    }
    //Constructor of a usable Xeon_Base object
    Xeon_Base::Xeon_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads, bool debug) {
        Run = true;
        Running = false;
        LAN_Address = lanaddress;
        OpenToLANAddress = addr._Equal("0.0.0.0");
        OpenToLANPort = (port == 80 || port == 443);
        Addr = addr;
        Port = port;
        Allowed_Backlog = allowed_backlog;
        Buffer_Size = buffer_size;
        Debug = debug;
        Server_Socket = new O2::O2Socket(MAKEWORD(2, 2), AF_INET, SOCK_STREAM, IPPROTO_TCP, Buffer_Size, (workerthreads <= 0));
        WorkerThreads = workerthreads;
    }
    //Xeon_Base destructor
    Xeon_Base::~Xeon_Base() {
        if (Workers.size() > 0) {
            Workers.clear();
        }
        delete Server_Socket;
    }

    //(virtual) Default start function, developers can override this
    void Xeon_Base::Start() {
        Lock();
        cout << "<--------------------------Xeon-------------------------->" << endl << endl;
        cout << "Binding Socket..." << endl;
        Server_Socket->PortBind(Addr, Port);

        cout << "Listening to Socket..." << endl;
        Server_Socket->PortListen(Allowed_Backlog);

        if (WorkerThreads > 0) {
            cout << "Starting Worker(s)..." << endl;
            for (int i = 0; i < WorkerThreads; i++) {
                Workers.push_back(std::thread(Handle_Request, this));
            }
        }

        cout << "Started on " << Addr << ":" << Port << endl;
        if (LAN_Address != "" && OpenToLANAddress && OpenToLANPort) {
            cout << "Any machine on local network can access at: " << LAN_Address << endl;
        }
        cout << "Launch: http://localhost:" << Port << "/" << endl;

        //String to hold the incoming client request data 
        string IncomingMessage; 
        //Set running flag
        Running = true;
        //Unlock the object for updates
        Unlock();
        //Begin loop listening for nrw connections
        O2::O2SocketID* NewConnection;
        while (Run) {
            if (WorkerThreads > 0) {
                //Add request to the queue
                Server_Socket->Select(30);
            }
            else {
                //No worker threads, must process on listener thread
                //Get New connections
                NewConnection = new O2::O2SocketID;
                *NewConnection = Server_Socket->AcceptNewConnection();

                if (*NewConnection != -1) {
                    IncomingMessage.append(Recieve(*NewConnection));
                    if (debug) {
                        cout << "<--------Request-------->\n\n" << endl;
                        cout << IncomingMessage << endl;
                        cout << "<--------End-------->\n\n" << endl;
                    }
                    SendResponse((*NewConnection), IncomingMessage);
                    IncomingMessage = "";
                }
            }
        }
        Running = false;
        cout << "Xeon Stopped." << endl;
    }
    void Xeon_Base::CloseConnectedSocket(O2::O2SocketID ID) {
        //Close the connection
        Server_Socket->CloseConnectedSocket(ID);
    }
    //(virtual) Condensed response to a request with sanity check on specified connection. Developers can override this.
    void Xeon_Base::SendResponse(O2::O2SocketID ID, string Request) {
        if (!Request.empty()) {
            if (Debug) {
                cout << "Xeon Read: " << endl;
                cout << Request << endl;
            }
            //This server's O2Socket will send out the data from this server's respond function which is determined by the incoming request. 
            Server_Socket->Send(ID, BuildResponse(Request));
        }
    }
    //(virtual) Stop the object. Developers can override this.
    void Xeon_Base::Stop() {
        Lock();
        cout << "Safely Stopping Xeon..." << endl;
        Run = false;
        cout << "Stopping Worker(s)..." << endl;
        int Worker_Stop_Attempts;
        for (int i = 0; i < Workers.size(); i++) {
            Worker_Stop_Attempts = 0;
            while (!(Workers[i].joinable()) && Worker_Stop_Attempts < 100) {
                Sleep(10);
                Worker_Stop_Attempts++;
            }
            if (Worker_Stop_Attempts >= 100) {
                cout << "Failed to Stop Worker(s)" << endl;
            }
            else {
                Workers[i].join();
            }
        }
        cout << "Closing Socket(s)..." << endl;
        Server_Socket->Close();
        Unlock();
    }
    O2::O2SocketID* Xeon_Base::GetNextRequest() {
        return Server_Socket->GetNextRequest();
    }
    //Retrieve string from the specified connetion
    char* Xeon_Base::Recieve(O2::O2SocketID ID) {
        return Server_Socket->Recieve(ID);
    }
    //Lock for object operations
    void Xeon_Base::Lock() {
        ObjectLock.lock();
    }
    //Unlock for object operations
    void Xeon_Base::Unlock() {
        ObjectLock.unlock();
    }
    //print the input if the object is in debug mode
    void Xeon_Base::DebugMessage(string Message) {
        if (Debug) {
            cout << Message << endl;
        }
    }
    //non-member function for a worker thread to process member data
    void Handle_Request(Xeon_Base* b) {
        O2::O2SocketID* ID;
        string IncomingMessage;
        char* IncommingMessageCstr;
        while (b->IsStarted()) {
            //limit queue poll to 100 times a second
            Sleep(10);
            ID = b->GetNextRequest();
            if (ID != NULL) {
                IncommingMessageCstr = NULL;
                IncommingMessageCstr = b->Recieve((*ID));
                if (IncommingMessageCstr != NULL) {
                    IncomingMessage.append(IncommingMessageCstr);
                    if (debug) {
                        cout << "<--------Request-------->" << endl;
                        cout << IncomingMessage << endl;
                        cout << "<--------End-------->" << endl;
                    }
                    b->SendResponse((*ID), IncomingMessage);
                    IncomingMessage = "";
                }
                //else {
                //    ErrorAndDie(408, "Open connection could not return data");
                //}
                //Close the connection
                b->CloseConnectedSocket(*ID);
            }
        }
    }
#pragma endregion

#pragma region Servers
    class Server_Base : public Xeon_Base {
    public:
        Server_Base();
        Server_Base(string, string, int, int, int, int, bool);
        string BuildResponse(string);
    };
    Server_Base::Server_Base() : Xeon_Base() {}

    Server_Base::Server_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads, debug) {}

    string Server_Base::BuildResponse(string Request) {
        int DataTypeEnm = 0;
        string compare = "";
        string Line;
        string File;
        string Path = "C:\\ServerFiles\\";
        std::regex File_Extension_Regex("^[^\s]+\.(html|jpg)$");
        std::regex File_Extension_jpg_Regex("^[^\s]+\.(jpg)$");
        string Requested_File;
        vector<string> Filter;
        vector<string> Tokens = TokenizeString(Request, "\n", Filter);
        if (Tokens.size() > 0) {
            Tokens = TokenizeString(Tokens[0], " /", Filter);
            for (int i = 0; i < Tokens.size(); i++) {
                compare = Tokens[i];
                if (std::regex_search(compare, File_Extension_Regex)) {
                    Requested_File.append(Path);
                    Requested_File.append(Tokens[i]);
                }
                if (std::regex_search(compare, File_Extension_jpg_Regex)) {
                    DataTypeEnm = 1;
                }
            }
        }
        if (Requested_File == "") {
            Requested_File.append(Path);
            Requested_File.append("index.html");
        }
        if (Debug) {
            cout << "File: " << Requested_File << endl;
        }

        string server_message = "HTTP/1.1 200 OK\n";
        server_message.append("Date: ");
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
        server_message.append(TimeString);
        server_message.append("GMT\n");
        server_message.append("Server: ");
        server_message.append(SERVER_VERSION);
        server_message.append("\n");
        server_message.append("Content-Type: ");
        if (DataTypeEnm == 0) { //HTML
            server_message.append("text/html");
            server_message.append("\nContent - Length: ");
            ifstream ReadFile(Requested_File);
            if (ReadFile.is_open()) {
                while (getline(ReadFile, Line)) {
                    //if (std::regex_search(Line, Img_Tag_Regex)) {
                    //    //cout << "Image Requested!" << endl;
                    //    //cout << Line << endl;
                    //    //vector<string> LineTokens = TokenizeString(Line, "\n", Filter);
                    //    //for (int li = 0; li < LineTokens.size(); li++) {
                    //    //    cout << "Line Token: " << LineTokens[li] << endl;
                    //    //}
                    //}
                    File.append(Line);
                    File.append("\n");
                }
                ReadFile.close();
                server_message.append(to_string(strlen(File.c_str()) + 1));
                //server_message.append("\nConnection: Closed");
                server_message.append("\nConnection: keep-alive");
                server_message.append("\n\n");
                server_message.append(File);
                if (Debug) {
                    cout << "<--------Sending-------->" << endl;
                    cout << server_message << endl;
                    cout << "<--------End-------->" << endl;
                }
            }
            else {
                ErrorAndDie(404, "File not found"); //TODO: respond to client with 404 not found.
            }
        }
        else if (DataTypeEnm == 1) { //jpeg
            server_message.append("image/jpeg");
            server_message.append("\nContent - Length: ");
            FILE* fp;
            fp = fopen(Requested_File.c_str(), "r");
            char buf[32000];
            if (fp == NULL) {
                ErrorAndDie(404, "file not found");
            }
            int ret = fread(buf, 1, 32000, fp);
            fclose(fp);
            server_message.append(to_string(ret));
            //server_message.append("\nConnection: Closed");
            server_message.append("\nConnection: keep-alive");
            server_message.append("\n\n");
            server_message.append(buf);
        }
        else{
            ErrorAndDie(104, "Unknown content type");
        }
        return server_message;
    }

    class LocalServer : public Server_Base {
    public:
        LocalServer();
        LocalServer(int, int, int, bool, bool);
    };

    LocalServer::LocalServer() : Server_Base("", "127.0.0.1", 8080, 20, KB(32), 0, false) {}

    LocalServer::LocalServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start, bool debug) : Server_Base("", "127.0.0.1", 8080, allowed_backlog, buffer_size, workerthreads, debug) {
        if (auto_start) {
            this->Start();
        }
    }

    class WebServer : public Server_Base {
    public:
        WebServer();
        WebServer(int, int, int, bool, bool);
    };

    WebServer::WebServer() : Server_Base(GetIP(), "0.0.0.0", 80, 20, KB(32), 0, false) {}

    WebServer::WebServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start, bool debug) : Server_Base(GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, workerthreads, debug) {
        if (auto_start) {
            this->Start();
        }
    }

#pragma endregion

#pragma region Harnesses
    class Harness_Base : public Xeon_Base {
    private:
        string Page;
    public:
        Harness_Base();
        Harness_Base(string, string, string, int, int, int, int, bool);
        string BuildResponse(string);
    };
    Harness_Base::Harness_Base() : Xeon_Base() {
        Page = "";
    }

    Harness_Base::Harness_Base(string page, string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads, debug) {
        Page = page;
    }

    string Harness_Base::BuildResponse(string Request) {
        //TODO: build web doc engine and respond with its response to this request
        string server_message = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
        server_message.append(to_string(Page.size()));
        server_message.append("\n\n");
        server_message.append(Page);

        return server_message;
    }

    class LocalHarness : public Harness_Base {
    public:
        LocalHarness();
        LocalHarness(string, int, int, int, bool, bool);
    };
    LocalHarness::LocalHarness() : Harness_Base("", "", "127.0.0.0", 8080, 20, KB(32), 0, false) {}

    LocalHarness::LocalHarness(string page, int allowed_backlog, int buffer_size, int workerthreads, bool auto_start, bool debug) : Harness_Base(page, "", "127.0.0.0", 8080, allowed_backlog, buffer_size, workerthreads, debug) {
        if (auto_start) {
            this->Start();
        }
    }

    class WebHarness : public Harness_Base {
    public:
        WebHarness();
        WebHarness(string, int, int, int, bool, bool);
    };
    WebHarness::WebHarness() : Harness_Base("", GetIP(), "0.0.0.0", 80, 20, KB(32), 0, false) {}

    WebHarness::WebHarness(string page, int allowed_backlog, int buffer_size, int workerthreads, bool auto_start, bool debug) : Harness_Base(page, GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, workerthreads, debug) {
        if (auto_start) {
            this->Start();
        }
    }

#pragma endregion
}