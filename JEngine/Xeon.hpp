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
//#include "curl/curl.h"

namespace Xeon {
#pragma region Enum
    enum DataTypes
    {
        HTML = 0,
        JPEG = 1
    };
#pragma endregion 
#pragma region XeonReponse
    struct XeonReponse {
        string headers;
        int datalen;
        char* data;
    };
#pragma endregion 
#pragma region Base
    static const string SERVER_VERSION = "Xeon/0.3.1 (Win64)";
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
        virtual XeonReponse BuildResponse(string) { XeonReponse a;  return a; }
        virtual char* Recieve(O2::O2SocketID);
        virtual void SendResponse(O2::O2SocketID, string);
        O2::O2SocketID* GetNextRequest();
        void CloseConnectedSocket(O2::O2SocketID);
        void Lock();
        void Unlock();
        void DebugMessage(string);
        void ToggleDebug();
        void SetPollTimeout(int);
        void SetConnectionTimeout(int);
        CONDITION_VARIABLE* GetCondition();
        CRITICAL_SECTION* GetLock();

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
        std::mutex ObjectLock;
        int WorkerThreads;
        vector<std::thread> Workers;
        int PollTimeout;
        int ConnectionTimeout;
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
        XDebug = false;
        Server_Socket = new O2::O2Socket();
        WorkerThreads = 0;
        PollTimeout = 1;
    }
    //Constructor of a usable Xeon_Base object
    Xeon_Base::Xeon_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads) {
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
        PollTimeout = 10;
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
                Server_Socket->Select(PollTimeout, &Run);
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
            if (XDebug) {
                cout << "Xeon Read: " << endl;
                cout << Request << endl;
            }
            //This server's O2Socket will send out the data from this server's respond function which is determined by the incoming request.

            XeonReponse Response = BuildResponse(Request);
            Server_Socket->Send(ID, Response.headers, Response.data, Response.datalen);
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
        if (XDebug){
            cout << Message << endl;
        }
    }
    //Turn on debug
    void Xeon_Base::ToggleDebug() {
        XDebug = !XDebug;
    }
    void Xeon_Base::SetPollTimeout(int seconds) {
        PollTimeout = seconds;
    }
    void Xeon_Base::SetConnectionTimeout(int seconds) {
        ConnectionTimeout = seconds;
    }
    CONDITION_VARIABLE* Xeon_Base::GetCondition() {
        return Server_Socket->GetCondition();
    }
    CRITICAL_SECTION* Xeon_Base::GetLock() {
        return Server_Socket->GetLock();
    }

    //non-member function for a worker thread to process member data
    void Handle_Request(Xeon_Base* b) {
        O2::O2SocketID* ID = NULL;
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
                    //if (XDebug) {
                    //    cout << "<--------Request-------->" << endl;
                    //    cout << IncomingMessage << endl;
                    //    cout << "<--------End-------->" << endl;
                    //}
                    b->SendResponse((*ID), IncomingMessage);
                    IncomingMessage = "";
                }
                //Dequeued ID, delete the data holding ID on the heap
                delete ID;
            }
        }
    }
#pragma endregion

#pragma region Servers
    class Server_Base : public Xeon_Base {
    public:
        Server_Base();
        Server_Base(string, string, int, int, int, int);
        XeonReponse BuildResponse(string);
    };
    Server_Base::Server_Base() : Xeon_Base() {}

    Server_Base::Server_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads) {}

    XeonReponse Server_Base::BuildResponse(string Request) {
        XeonReponse Server_Response;
        int DataTypeEnm = HTML;
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
                    DataTypeEnm = JPEG;
                }
            }
        }
        if (Requested_File == "") {
            Requested_File.append(Path);
            Requested_File.append("index.html");
        }
        if (XDebug) {
            cout << "File: " << Requested_File << endl;
        }

        string headers = "HTTP/1.1 200 OK\n";
        headers.append("Date: ");
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
        headers.append(TimeString);
        headers.append("GMT\n");
        headers.append("Server: ");
        headers.append(SERVER_VERSION);
        headers.append("\n");
        headers.append("Content-Type: ");
        if (DataTypeEnm == HTML) {
            headers.append("text/html");
            headers.append("\nContent - Length: ");
            ifstream ReadFile(Requested_File);
            if (ReadFile.is_open()) {
                while (getline(ReadFile, Line)) {
                    File.append(Line);
                    File.append("\n");
                }
                ReadFile.close();
                headers.append(to_string(strlen(File.c_str()) + 1));
                headers.append("\nConnection: close");
                headers.append("\n\n");
                Server_Response.headers = headers;
                Server_Response.data = new char[strlen(File.c_str()) + 1];
                Server_Response.datalen = strlen(File.c_str()) + 1;

                strcpy(Server_Response.data, File.c_str());
                if (XDebug) {
                    cout << "<--------Sending-------->" << endl;
                    cout << Server_Response.headers << endl;
                    cout << Server_Response.data << endl;
                    cout << "<--------End-------->" << endl;
                }
            }
            else {
                ErrorAndDie(404, "File not found"); //TODO: respond to client with 404 not found.
            }
        }
        else if (DataTypeEnm == JPEG) {
            headers.append("image/jpeg");
            FILE* Image;
            int ImageSize = 0;
            Image = fopen(Requested_File.c_str(), "rb");
            if (!Image) {
                ErrorAndDie(404, "file not found");
            }
            fseek(Image, 0, SEEK_END);
            ImageSize = ftell(Image);
            fseek(Image, 0, SEEK_SET);

            headers.append("\nContent-Length: ");
            headers.append(to_string(ImageSize));
            headers.append("\nConnection: close");
            headers.append("\n\n");

            Server_Response.headers = headers;
            Server_Response.data = new char[ImageSize];
            Server_Response.datalen = ImageSize;
            memset(Server_Response.data, '\0', ImageSize);

            int BytesRead = 0;
            do {
                BytesRead = fread(Server_Response.data, 1, ImageSize, Image);
            } while (BytesRead > 0);
            fclose(Image);
        }
        else {
            ErrorAndDie(104, "Unknown content type");
        }
        XeonReponse Server_Response2;
        Server_Response2 = Server_Response;
        return Server_Response;
    }

    class LocalServer : public Server_Base {
    public:
        LocalServer();
        LocalServer(int, int, int, bool);
    };

    LocalServer::LocalServer() : Server_Base("", "127.0.0.1", 8080, 20, KB(32), 0) {}

    LocalServer::LocalServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start) : Server_Base("", "127.0.0.1", 8080, allowed_backlog, buffer_size, workerthreads) {
        if (auto_start) {
            this->Start();
        }
    }

    class WebServer : public Server_Base {
    public:
        WebServer();
        WebServer(int, int, int, bool, bool);
    };

    WebServer::WebServer() : Server_Base(GetIP(), "0.0.0.0", 80, 20, KB(32), 0) {}

    WebServer::WebServer(int allowed_backlog, int buffer_size, int workerthreads, bool auto_start, bool debug) : Server_Base(GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, workerthreads) {
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
        XeonReponse BuildResponse(string);
    };
    Harness_Base::Harness_Base() : Xeon_Base() {
        Page = "";
    }

    Harness_Base::Harness_Base(string page, string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, int workerthreads, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, workerthreads) {
        Page = page;
    }

    XeonReponse Harness_Base::BuildResponse(string Request) {
        XeonReponse Server_Response;
        Server_Response.data = NULL;
        Server_Response.datalen = 0;
        //TODO: build web doc engine and respond with its response to this request
        string headers = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
        headers.append(to_string(Page.size()));
        headers.append("\n\n");
        Server_Response.headers = headers;
        Server_Response.data = new char[Page.size()];
        Server_Response.datalen = Page.size();
        strcpy(Server_Response.data, Page.c_str());

        return Server_Response;
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

    void Test_Xeon() {
        string input;
        int Backlog = 1;
        int Buffer = KB(32);
        int threads = 0;
        bool fail;
        char next;
        //WebDoc::home();
        do {
            fail = false;
            threads = GetIntInput("How many worker threads would you like?", true, false, true);
            if (threads < 0 || threads > 8) {
                cout << "threads must be between 0 and 8" << endl; //TOOD: make a hydrogen base CPU info object to get max threads
                fail = true;
            }
        } while (fail);


        Xeon::WebServer MyWebServer(Backlog, Buffer, threads, false, false);

        //Xeon::LocalServer MyLocalServer(Backlog, Buffer, true, true);
        std::thread server_thread(runserver, &MyWebServer);
        Sleep(100);
        while (MyWebServer.IsRunning()) {
            cout << "What would you like to do?" << endl;
            cout << "1 - Stop Server" << endl;
            cout << "2 - Restart Server" << endl;
            cin >> input;
            if (input == "1") {
                MyWebServer.Stop();
                break;
            }
            else if (input == "2") {
                MyWebServer.Stop();
                MyWebServer.Start();
                input = "";
            }
            system("cls");
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