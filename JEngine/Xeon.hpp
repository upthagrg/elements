#pragma once
/*
File: Xeon.hpp
Author: Glenn Upthagrove
Last Updated: 06/19/2024
Description: This header file contains the Xeon Web server and local harness of the Elements appication framework.
These are built off off the Oxygen networking library. And the Hydrogen archiecture library.
This can act as a web server for a website, local testing of the website, or as a local GUI for an Elements application.
This will allow for much more control over what the web server can do allowing a developer to extend past the basics of a web server and inject their own alterations to returned content.
This will be extended in several ways to include many many more capabilitites and improvements.
One of these future abilities will be a dynamicly genreated HTML engine (maybe it will be carbon, currently it is WebDoc.hpp) which will allow a web application that is not simply a delivery system of HTML or PHP.
*/

#include "Oxygen.hpp"

namespace Xeon {
#pragma region Enum
    enum DataTypes
    {
        HTML = 0,
        JPEG = 1,
        PNG = 2,
        ICO = 3,
        MP4 = 4
    };
#pragma endregion
#pragma region Base
    static const string SERVER_VERSION = "Xeon/0.0.1 (Win64)";
    class Xeon_Base;
    void Handle_Request(Xeon_Base*);
    class Xeon_Base {
    public:
        explicit Xeon_Base();
        explicit Xeon_Base(string, string, int, int, int, int);
        ~Xeon_Base();
        virtual void Start();
        virtual void Stop();
        virtual bool IsRunning() { return Running; }
        bool IsStarted() { return Run; }
        virtual O2::O2Data BuildResponse(string) { O2::O2Data a;  return a; }
        virtual char* Recieve(O2::O2SocketID);
        virtual void SendResponse(O2::O2SocketID, string);
        O2::O2SocketID* GetNextRequest();
        void CloseConnectedSocket(O2::O2SocketID);
        void Lock();
        void Unlock();
        void DebugMessage(string);
        void ToggleDebug();
        void SetPollTimeout(int);
        CONDITION_VARIABLE* GetCondition();
        CRITICAL_SECTION* GetLock();
        char* GetSocketMessage(O2::O2SocketID);
        void SetPatch(string);
        virtual string BuildIndex(string);

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
        bool XDebug;
        CRITICAL_SECTION ObjectLock;
        int WorkerThreads;
        vector<std::thread> Workers;
        int PollTimeout;
        string Path;
    };
    //Default constructor of Xeon_Base objects, this would be an unusable object in this state
    Xeon_Base::Xeon_Base() {
        InitializeCriticalSection(&ObjectLock);
        Run = false;
        Running = false;
        OpenToLANAddress = false;
        OpenToLANPort = false;
        Addr = "";
        Port = -1;
        Allowed_Backlog = -1;
        Buffer_Size = -1;
        XDebug = false;
        Server_Socket = new O2::O2Socket();
        WorkerThreads = 0;
        PollTimeout = 1;
        Path = ".\\";
    }
    //Constructor of a usable Xeon_Base object
    Xeon_Base::Xeon_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads) {
        InitializeCriticalSection(&ObjectLock);
        Run = true;
        Running = false;
        LAN_Address = lanaddress;
        OpenToLANAddress = addr._Equal("0.0.0.0");
        OpenToLANPort = (port == 80 || port == 443);
        Addr = addr;
        Port = port;
        Allowed_Backlog = allowed_backlog;
        Buffer_Size = buffer_size;
        XDebug = false;
        Server_Socket = new O2::O2Socket(MAKEWORD(2, 2), AF_INET, SOCK_STREAM, IPPROTO_TCP, Buffer_Size, (workerthreads <= 0));
        WorkerThreads = workerthreads;
        PollTimeout = 30;
        Path = ".\\";
    }
    //Xeon_Base destructor
    Xeon_Base::~Xeon_Base() {
        if (Workers.size() > 0) {
            Workers.clear();
        }
        delete Server_Socket;
    }

    void Xeon_Base::SetPatch(string NewPath) {
        Path = NewPath;
    }

    string Xeon_Base::BuildIndex(string pPath) {
        string index = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF - 8\"><title>Index</title><style>body{font-family:'Segoe UI',Tahoma,sans-serif;margin:0;padding:0;background:#f5f8fa;color:#333;display:flex;align-items:center;justify-content:center;height:100vh;}.container{background:#fff;padding:40px;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.1);max-width:500px;width:90%;}h1{text-align:center;color:#4a90e2;margin-bottom:25px;}ul{list-style:none;padding:0;margin:0;}li{margin:10px 0;}a{display:block;background:#f0f4f8;padding:12px 16px;border-radius:8px;text-decoration:none;color:#333;transition:background 0.2s,transform 0.1s;}a:hover{background:#4a90e2;color:#fff;transform:translateY(-2px);}</style></head><body><div class=\"container\"><h1>Index</h1><ul>";

        // This structure would distinguish a file from a
        // directory
        struct stat sb;

        // Looping until all the items of the directory are
        // exhausted
        bool failed = true;
        for (const auto& entry : std::filesystem::directory_iterator(pPath)) {

            // Converting the path to const char * in the
            // subsequent lines
            std::filesystem::path outfilename = entry.path();
            std::string outfilename_str = outfilename.string();
            const char* file_full_path = outfilename_str.c_str();

            string FullFile = file_full_path;
            string File = file_full_path;

            File = File.substr(pPath.length() +1, (File.length() - pPath.length() -1));
            if (!(stat(file_full_path, &sb) == 0 && !(sb.st_mode & S_IFDIR))) {
                //is a directory
                File.append("*");
            }
            index.append("<li><a href=\"" + File + "\">" + File + "</a></li>");
            failed = false;
        }

        index.append("</ul></div></body></html>");
        if (failed) {
            index = "";
        }
        string ret;
        ret = index;
        return ret;
    }

    //(virtual) Default start function, developers can override this
    void Xeon_Base::Start() {
        Lock();
        MyBase.Display("<--------------------------Xeon-------------------------->");
        MyBase.Display("");
        MyBase.Display("Binding Socket...");
        Server_Socket->PortBind(Addr, Port);

        MyBase.Display("Listening to Socket...");
        Server_Socket->PortListen(Allowed_Backlog);

        if (WorkerThreads > 0) {
            MyBase.Display("Starting Worker(s)...");
            for (int i = 0; i < WorkerThreads; i++) {
                Workers.push_back(std::thread(Handle_Request, this));
            }
        }

        string Message = "";
        Message.append("Started on ");
        Message.append(Addr);
        Message.append(":");
        Message.append(to_string(Port));
        MyBase.Display(Message);
        Message = "";
        if (LAN_Address != "" && OpenToLANAddress && OpenToLANPort) {
            Message.append("Any machine on local network can access at: ");
            Message.append(LAN_Address);
            MyBase.Display(Message);
            Message = "";
        }
        Message.append("Launch: http://localhost:");
        Message.append(to_string(Port));
        Message.append("/");
        MyBase.Display(Message);
        Message = "";

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
                Server_Socket->GetReadyRequests(PollTimeout, &Run);
                //TODO: if no maintenance thread, run maintenance
            }
            else {
                //No worker threads, must process on listener thread
                //Get New connections
                NewConnection = new O2::O2SocketID;
                *NewConnection = Server_Socket->AcceptNewConnection();

                if (*NewConnection > 0) {
                    IncomingMessage.append(Recieve(*NewConnection));
                    if (XDebug) {
                        MyBase.Display("<--------Request-------->\n\n");
                        MyBase.Display(IncomingMessage);
                        MyBase.Display("<--------End-------->\n\n");
                    }
                    SendResponse((*NewConnection), IncomingMessage);
                    IncomingMessage = "";
                }
            }
        }
        Running = false;
        MyBase.Display("Xeon Stopped.");
    }
    void Xeon_Base::CloseConnectedSocket(O2::O2SocketID ID) {
        //Close the connection
        Server_Socket->CloseConnectedSocket(ID);
    }
    //(virtual) Condensed response to a request with sanity check on specified connection. Developers can override this.
    void Xeon_Base::SendResponse(O2::O2SocketID ID, string Request) {
        if (!Request.empty()) {
            if (XDebug) {
                MyBase.Display("Xeon Read: ");
                MyBase.Display(Request);
            }
            //This server's O2Socket will send out the data from this server's respond function which is determined by the incoming request.

            O2::O2Data Response = BuildResponse(Request);
            Server_Socket->Send(ID, Response);
        }
    }
    //(virtual) Stop the object. Developers can override this.
    void Xeon_Base::Stop() {
        Lock();
        MyBase.Display("Safely Stopping Xeon...");
        Run = false;
        MyBase.Display("Stopping Worker(s)...");
        for (int i = 0; i < Workers.size(); i++) {
            if (Workers[i].joinable()) {
                Workers[i].join();
            }
        }
        MyBase.Display("Worker(s) stopped");
        MyBase.Display("Closing Socket(s)...");
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
        EnterCriticalSection(&ObjectLock);
    }
    //Unlock for object operations
    void Xeon_Base::Unlock() {
        LeaveCriticalSection(&ObjectLock);
    }
    //print the input if the object is in debug mode
    void Xeon_Base::DebugMessage(string Message) {
        if (XDebug){
            MyBase.Display(Message);
        }
    }
    //Turn on debug
    void Xeon_Base::ToggleDebug() {
        XDebug = !XDebug;
        Server_Socket->ToggleDebug();
    }
    void Xeon_Base::SetPollTimeout(int seconds) {
        PollTimeout = seconds;
    }
    CONDITION_VARIABLE* Xeon_Base::GetCondition() {
        return Server_Socket->GetCondition();
    }
    CRITICAL_SECTION* Xeon_Base::GetLock() {
        return Server_Socket->GetLock();
    }
    char* Xeon_Base::GetSocketMessage(O2::O2SocketID ID) {
        return Server_Socket->GetSocketMessage(ID);
    }

    //non-member function for a worker thread to process member data
    void Handle_Request(Xeon_Base* b) {
        O2::O2SocketID* ID;
        string IncomingMessage;
        char* IncommingMessageCstr;
        DWORD timeout(100);
        while (b->IsStarted()) {
            ID = NULL;
            //limit queue poll to 100 times a second
            //Sleep(10);
            EnterCriticalSection(b->GetLock());
            while (ID == NULL) {
                ID = b->GetNextRequest();
                
                SleepConditionVariableCS(b->GetCondition(), b->GetLock(), timeout);
                if (!b->IsStarted()) {
                    MyBase.Display("Worker Stopping");
                    MyBase.Display("Worker cleaning up data");
                    MyBase.DeletePointer((void*)ID);
                    delete ID;
                    MyBase.Display("Worker data clean");
                    MyBase.Display("Worker releasing lock");
                    LeaveCriticalSection(b->GetLock());
                    MyBase.Display("Worker leaving function");
                    return;
                }
            }
            IncommingMessageCstr = NULL;
            IncommingMessageCstr = b->GetSocketMessage(*ID);
            if (IncommingMessageCstr != NULL) {
                IncomingMessage.append(IncommingMessageCstr);
                b->SendResponse((*ID), IncomingMessage);
                IncomingMessage = "";
            }
            //Dequeued ID, delete the data holding ID on the heap
            MyBase.DeletePointer((void*)ID);
            delete ID;
            LeaveCriticalSection(b->GetLock());
        }
    }
#pragma endregion

#pragma region Servers
    class Server_Base : public Xeon_Base {
    public:
        Server_Base();
        Server_Base(string, string, int, int, int, int);
        O2::O2Data BuildResponse(string);
    };
    Server_Base::Server_Base() : Xeon_Base() {}

    Server_Base::Server_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads) {}

    O2::O2Data Server_Base::BuildResponse(string Request) {
        int FileLength = 0;
        bin Content;
        int DataTypeEnm = HTML;
        string compare = "";
        string Line;
        bool IsGETRequest = false;
        O2::O2Data Response;
        bool ReturnIndexHtml = false;
        bool ReturnFolder = false;
        FILE* File;
        int FileSize = 0;
        string RequestedPath = "";

        std::regex File_Extension_Regex("\\.(?:html|jpg|jpeg|png|ico|mp4)$");
        std::regex File_Extension_jpg_Regex("\\.(?:jpg|jpeg)$");
        std::regex File_Extension_png_Regex("\\.(?:png)$");
        std::regex File_Extension_ico_Regex("\\.(?:ico)$");
        std::regex File_Extension_mp4_Regex("\\.(?:mp4)$");

        string Requested_File;
        vector<string> Filter;
        O2::HTTPEngine HTTPEngine;
        HTTPEngine.SetServerVersion(SERVER_VERSION);
        HTTPEngine.SetStatus(O2::HTTPStatus::OK);
        HTTPEngine.SetContentType(O2::HTTPContentType::HTML);
        vector<string> Tokens = TokenizeString(Request, "\n", Filter);
        //TODO: build a find for an ID for the session, if not found build one and redirect, maybe better for harness

        if (Tokens.size() > 0) {
            Tokens = TokenizeString(Tokens[0], " /", Filter);
            for (int i = 0; i < Tokens.size(); i++) {
                compare = Tokens[i];
                if (to_upper(compare) == "GET") {
                    IsGETRequest = true;
                }
                if (std::regex_search(compare, File_Extension_Regex)) {
                    Requested_File.append(Path);
                    Requested_File.append(Tokens[i]);
                }
                if (std::regex_search(compare, File_Extension_jpg_Regex)) {
                    HTTPEngine.SetContentType(O2::HTTPContentType::JPEG);
                }
                else if (std::regex_search(compare, File_Extension_png_Regex)) {
                    HTTPEngine.SetContentType(O2::HTTPContentType::PNG);
                }
                else if (std::regex_search(compare, File_Extension_ico_Regex)) {
                    HTTPEngine.SetContentType(O2::HTTPContentType::ICO);
                }
                else if (std::regex_search(compare, File_Extension_mp4_Regex)) {
                    HTTPEngine.SetContentType(O2::HTTPContentType::MP4);
                }
            }
        }

        if (IsGETRequest) {
            if (Requested_File == "") {
                for (int i = 1; i < Tokens.size(); i++) {
                    if (Tokens[i] != "HTTP") {
                        RequestedPath.append(Tokens[i]);
                    }
                    else {
                        ReturnFolder = true;
                        break;
                    }
                }
                //attempt to load index.html
                Requested_File.append(Path);
                if (RequestedPath == "") {
                    ReturnIndexHtml = true;
                    ReturnFolder = false;
                    Requested_File.append("index.html");
                }

                //if that does not exist, attempt to build an index

            }
            if (XDebug) {
                string msg = "";
                msg.append("File: ");
                msg.append(Requested_File);
                MyBase.Display(msg);
            }

            HTTPEngine.SetConnection(O2::HTTPConnection::CLOSED);

            //Get Content from file
            if (!ReturnFolder) {
                File = fopen(Requested_File.c_str(), "rb");
                if (!File && ReturnIndexHtml) {
                    ReturnFolder = true;
                }
                if (!ReturnFolder) {
                    if (!File) {
                        HTTPEngine.SetStatus(O2::HTTPStatus::NOTFOUND);
                        Requested_File = "";
                        Requested_File.append(Path);
                        Requested_File.append("ContentNotFound.html");
                        File = fopen(Requested_File.c_str(), "rb");
                    }
                    if (!File) {
                        string NotFoundPage = "<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The page you are looking for could not be found.</p></body></html>";
                        HTTPEngine.SetContentLength(NotFoundPage.length());
                        Response.AddData(HTTPEngine.BuildHttpMessage(true));
                        Content.SetData(NotFoundPage.c_str(), NotFoundPage.length()); //Convert data to binary
                        Response.AddData(Content);//Add the binary content to the reponse
                    }
                    else {
                        fseek(File, 0, SEEK_END);
                        FileSize = ftell(File);
                        fseek(File, 0, SEEK_SET);

                        HTTPEngine.SetContentLength(FileSize);

                        Response.AddData(HTTPEngine.BuildHttpMessage(true)); //Add the HTTP Headers to the response
                        char* FileData = new char[FileSize]; //TODO: this might be the memort leaks
                        memset(FileData, '\0', FileSize);

                        int BytesRead = 0;
                        do {
                            BytesRead = fread(FileData, 1, FileSize, File);
                        } while (BytesRead > 0);
                        fclose(File);

                        Content.SetData(FileData, FileSize); //Convert data to binary
                        delete[] FileData;
                        Response.AddData(Content);//Add the binary content to the reponse
                    }
                }
            }
            if (ReturnFolder) {
                Requested_File = Path;
                Requested_File.append(RequestedPath);
                string index = BuildIndex(Requested_File);
                if (index != "") {
                    HTTPEngine.SetContentLength(index.length());
                    Response.AddData(HTTPEngine.BuildHttpMessage(true));
                    Content.SetData(index.c_str(), index.length()); //Convert data to binary
                    Response.AddData(Content);//Add the binary content to the reponse
                    HTTPEngine.SetStatus(O2::HTTPStatus::OK);
                }
                else {
                    HTTPEngine.SetStatus(O2::HTTPStatus::NOTFOUND);
                    Requested_File = "";
                    Requested_File.append(Path);
                    Requested_File.append("ContentNotFound.html");
                    File = fopen(Requested_File.c_str(), "rb");
                    if (!File) {
                        string NotFoundPage = "<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The page you are looking for could not be found.</p></body></html>";
                        HTTPEngine.SetContentLength(NotFoundPage.length());
                        Response.AddData(HTTPEngine.BuildHttpMessage(true));
                        Content.SetData(NotFoundPage.c_str(), NotFoundPage.length()); //Convert data to binary
                        Response.AddData(Content);//Add the binary content to the reponse
                    }
                    else {
                        fseek(File, 0, SEEK_END);
                        FileSize = ftell(File);
                        fseek(File, 0, SEEK_SET);

                        HTTPEngine.SetContentLength(FileSize);

                        Response.AddData(HTTPEngine.BuildHttpMessage(true)); //Add the HTTP Headers to the response
                        char* FileData = new char[FileSize]; //TODO: this might be the memort leaks
                        memset(FileData, '\0', FileSize);

                        int BytesRead = 0;
                        do {
                            BytesRead = fread(FileData, 1, FileSize, File);
                        } while (BytesRead > 0);
                        fclose(File);

                        Content.SetData(FileData, FileSize); //Convert data to binary
                        delete[] FileData;
                        Response.AddData(Content);//Add the binary content to the reponse
                    }
                }
            }
        }
        else {
            ErrorAndDie(404, "Method not supported");
        }

        O2::O2Data FinalResponse = Response;
        return FinalResponse;
    }

    class LocalServer : public Server_Base {
    public:
        LocalServer();
        LocalServer(int, int, int, bool);
    };

    LocalServer::LocalServer() : Server_Base("", "127.0.0.1", 8080, 20, MB(512), 0) {}

    LocalServer::LocalServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start) : Server_Base("", "127.0.0.1", 8080, allowed_backlog, buffer_size, workerthreads) {
        if (auto_start) {
            this->Start();
        }
    }

    class WebServer : public Server_Base {
    public:
        WebServer();
        WebServer(int, int, int, bool);
    };

    WebServer::WebServer() : Server_Base(GetIP(), "0.0.0.0", 80, 20, MB(512), 0) {}

    WebServer::WebServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start) : Server_Base(GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, workerthreads) {
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
        O2::O2Data BuildResponse(string);
    };
    Harness_Base::Harness_Base() : Xeon_Base() {
        Page = "";
    }

    Harness_Base::Harness_Base(string page, string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads) {
        Page = page;
    }

    O2::O2Data Harness_Base::BuildResponse(string Request) {
        O2::O2Data Response;
        //TODO: build web doc engine and respond with its response to this request
        string headers = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
        headers.append(to_string(Page.size()));
        headers.append("\n\n");
        Response.AddData(headers);

        bin Content;
        Content.SetData(Page.c_str(), Page.size());
        Response.AddData(Content);

        return Response;
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

#pragma region Examples and tests
    void runserver(Xeon::WebServer* server) {
        server->Start();
    }

    void XeonSetupWizard() {
        string Input;
        int Backlog = 1;
        int Buffer = KB(32);
        int Threads = 0;
        bool Fail;
        char Next;
        int AdditionalLogging = -1;
        int PollTimeOut = 30;
        string NewPath = ".\\ServerFiles\\";
        MyBase.Display("Welcome to the Xeon Set Up Wizard");
        do {
            Fail = false;
            Threads = GetIntInput("How many worker threads would you like?", true, false, true);
            if (Threads < 0 || Threads > 8) {
                MyBase.Display("threads must be between 0 and 8"); //TOOD: make a hydrogen base CPU info object to get max threads
                Fail = true;
            }
        } while (Fail);
        do {
            Fail = false;
            AdditionalLogging = GetIntInput("Would you like additional logging?\n1-Yes\n2-No", true, false, false);
            if (AdditionalLogging > 2) {
                MyBase.Display("Please enter 1-Yes\nor\n2-No"); 
                Fail = true;
            }
        } while (Fail);
        PollTimeOut = GetIntInput("What should the timeout be in seconds?", true, false, false);
        Fail = false;
        bool changepath = GetBoolInput("Would you like to change the base path?");
        if (changepath) {
            NewPath = GetStringInput("Enter the new base path");
        }


        Xeon::WebServer MyWebServer(Backlog, Buffer, Threads, false);
        MyWebServer.SetPatch(NewPath);
        if (AdditionalLogging == 1) {
            MyWebServer.ToggleDebug();
        }
        MyWebServer.SetPollTimeout(PollTimeOut);

        std::thread server_thread(runserver, &MyWebServer);
        Sleep(100);
        while (MyWebServer.IsRunning()) {
            MyBase.Display("What would you like to do?");
            MyBase.Display("1 - Stop Server");
            MyBase.Display("2 - Restart Server");
            MyBase.Display("3 - Analyze Memory");
            MyBase.Display("");
            cin >> Input;
            if (Input == "1") {
                MyWebServer.Stop();
                MyBase.Display("Finished stopping, going to sleep");
                Sleep(3000);
                system("cls");
                break;
            }
            else if (Input == "2") {
                MyWebServer.Stop();
                Sleep(3000);
                MyWebServer.Start();
                Input = "";
                system("cls");
            }
            else if (Input == "3") {
                MyBase.PrintPointers();
            }
        }
        while (MyWebServer.IsRunning()) {
            Sleep(10);
            continue;
        }
        if (server_thread.joinable()) {
            server_thread.join();
        }
        exit(0);
    }
#pragma endregion
#pragma endregion
}