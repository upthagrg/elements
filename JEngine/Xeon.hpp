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
    class Xeon_Base {
    protected:
        O2::O2Socket* Server_Socket;
        bool Run;
        bool OpenToLANAddress;
        bool OpenToLANPort;
        string LAN_Address;
        string Addr;
        int Port;
        int Allowed_Backlog;
        int Buffer_Size;
        bool Debug;

    public:
        explicit Xeon_Base();
        explicit Xeon_Base(string, string, int, int, int, bool);
        virtual void Start();
        virtual void Stop();
        virtual void SendResponse(string);
        virtual string Respond(string) { return ""; }
        void DebugMessage(string);
    };
    //non-member function for a worker thread to process member data
    void Handle_Request(QUEUE* q, Xeon_Base* b) {
        string* ptr;
        while (true) {
            //limit queue poll to 100 times a second
            Sleep(10);
            ptr = (string*)(q->Dequeue());
            if (ptr != NULL) {
                b->SendResponse(*ptr);
            }
        }
    }
    //Default constructor of Xeon_Base objects, this would be an unusable object in this state
    Xeon_Base::Xeon_Base() {
        Run = false;
        OpenToLANAddress = false;
        OpenToLANPort = false;
        Addr = "";
        Port = -1;
        Allowed_Backlog = -1;
        Buffer_Size = -1;
        Debug = false;
        Server_Socket = new O2::O2Socket();
    }
    //Constructor of a usable Xeon_Base object
    Xeon_Base::Xeon_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, bool debug) {
        //signal(SIGINT, SignalHandler);
        Run = true;
        LAN_Address = lanaddress;
        OpenToLANAddress = addr._Equal("0.0.0.0");
        OpenToLANPort = (port == 80 || port == 443);
        Addr = addr;
        Port = port;
        Allowed_Backlog = allowed_backlog;
        Buffer_Size = buffer_size;
        Debug = debug;
        Server_Socket = new O2::O2Socket(MAKEWORD(2, 2), AF_INET, SOCK_STREAM, IPPROTO_TCP, Buffer_Size);
    }

    //(virtual) Default start function, developers can override this
    void Xeon_Base::Start() {
        //TODO: Get curl to link 
        //signal(SIGINT, SignalHandler);
        //CURL* req = curl_easy_init();
        cout << "<--------------------------Xeon-------------------------->" << endl << endl;
        cout << "Binding Socket..." << endl;
        Server_Socket->PortBind(Addr, Port);

        cout << "Listening to Socket..." << endl;
        Server_Socket->PortListen(Allowed_Backlog);

        cout << "Started on " << Addr << ":" << Port << endl;

        if (LAN_Address != "" && OpenToLANAddress && OpenToLANPort) {
            cout << "Any machine on local network can access at: " << LAN_Address << endl;
        }

        cout << "Launch: http://localhost:" << Port << "/" << endl;

        //String to hold the incoming client request data 
        string* request; 
        //Queue of requests to process
        QUEUE requests;
        //Spawn a worker thread to process the requests
        std::thread worker(Handle_Request, &requests, this); 
        while (Run) {
            request = new string;
            request->append(Server_Socket->Recieve(false));
            if (Debug) {
                cout << "Xeon Read: " << endl;
                cout << *request << endl;
            }
            //Listening thread adds request to work queue for the worker thread to process
            requests.Enqueue((void*)request); 
            request = NULL;
            //SBuff = (string*)requests.Dequeue();
            //this->SendResponse(*SBuff);
        }
        cout << "Safely Stopping Xeon..." << endl;
        Server_Socket->CloseSocket();
        cout << "Xeon Stopped." << endl;
    }
    //(virtual) Condensed response to a request with sanity check. Developers can override this.
    void Xeon_Base::SendResponse(string request) {
        //sanity check
        if (!request.empty()) {
            //this server's O2SOcket will send out the data from this server's respond function which is determined by the incoming request. 
            Server_Socket->SocketSend(Respond(request));
            //close the usable connection
            Server_Socket->CloseUsableSocket();
        }
    }
    //(virtual) Stop the object. Developers can override this.
    void Xeon_Base::Stop() {
        Run = false;
    }
    //print the input if the object is in debug mode
    void Xeon_Base::DebugMessage(string Message) {
        if (Debug) {
            cout << Message << endl;
        }
    }
#pragma endregion

#pragma region Servers
    class Server_Base : public Xeon_Base {
    public:
        Server_Base();
        Server_Base(string, string, int, int, int, bool);
        string Respond(string);
    };
    Server_Base::Server_Base() : Xeon_Base() {}

    Server_Base::Server_Base(string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, debug) {}

    string Server_Base::Respond(string Request) {
        string Line;
        string File;
        string Path = "C:\\ServerFiles\\";
        std::regex File_Extension_Regex("^[^\s]+\.(html|php|js)$");
        string Requested_File;
        vector<string> Filter;
        vector<string> Tokens = TokenizeString(Request, "\n", Filter);
        if (Tokens.size() > 0) {
            Tokens = TokenizeString(Tokens[0], " /", Filter);
            for (int i = 0; i < Tokens.size(); i++) {
                if (std::regex_search(Tokens[i], File_Extension_Regex)) {
                    Requested_File.append(Path);
                    Requested_File.append(Tokens[i]);
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

        string server_message = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
        ifstream ReadFile(Requested_File);
        if (ReadFile.is_open()) {
            //TODO: Make this able to read any length with a while bytes read > 0 and set string to the cstr and add to a vector then append all elemetns of vector to one string
            while (getline(ReadFile, Line)) {
                File.append(Line);
                File.append("\n");
            }
            ReadFile.close();
            server_message.append(to_string(strlen(File.c_str()) + 1));
            server_message.append("\n\n");
            server_message.append(File);
            if (Debug) {
                cout << "Sending:\n" << server_message << endl;
            }
        }
        else {
            ErrorAndDie(404, "File not found"); //TODO: respond to client with 404 not found.
        }
        return server_message;
    }

    class LocalServer : public Server_Base {
    public:
        LocalServer();
        LocalServer(int, int, bool, bool);
    };

    LocalServer::LocalServer() : Server_Base("", "127.0.0.1", 8080, 20, 30720, false) {}

    LocalServer::LocalServer(int allowed_backlog, int buffer_size, bool auto_start, bool debug) : Server_Base("", "127.0.0.1", 8080, allowed_backlog, buffer_size, debug) {
        if (auto_start) {
            this->Start();
        }
    }

    class WebServer : public Server_Base {
    public:
        WebServer();
        WebServer(int, int, bool, bool);
    };

    WebServer::WebServer() : Server_Base(GetIP(), "0.0.0.0", 80, 20, 30720, false) {}

    WebServer::WebServer(int allowed_backlog, int buffer_size, bool auto_start, bool debug) : Server_Base(GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, debug) {
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
        Harness_Base(string, string, string, int, int, int, bool);
        string Respond(string);
    };
    Harness_Base::Harness_Base() : Xeon_Base() {
        Page = "";
    }

    Harness_Base::Harness_Base(string page, string lanaddress, string addr, int port, int allowed_backlog, int buffer_size, bool debug) : Xeon_Base(lanaddress, addr, port, allowed_backlog, buffer_size, debug) {
        Page = page;
    }

    string Harness_Base::Respond(string Request) {
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
        LocalHarness(string, int, int, bool, bool);
    };
    LocalHarness::LocalHarness() : Harness_Base("", "", "127.0.0.0", 8080, 20, 30720, false) {}

    LocalHarness::LocalHarness(string page, int allowed_backlog, int buffer_size, bool auto_start, bool debug) : Harness_Base(page, "", "127.0.0.0", 8080, allowed_backlog, buffer_size, debug) {
        if (auto_start) {
            this->Start();
        }
    }



    class WebHarness : public Harness_Base {
    public:
        WebHarness();
        WebHarness(string, int, int, bool, bool);
    };
    WebHarness::WebHarness() : Harness_Base("", GetIP(), "0.0.0.0", 80, 20, 30720, false) {}

    WebHarness::WebHarness(string page, int allowed_backlog, int buffer_size, bool auto_start, bool debug) : Harness_Base(page, GetIP(), "0.0.0.0", 80, allowed_backlog, buffer_size, debug) {
        if (auto_start) {
            this->Start();
        }
    }

#pragma endregion
}