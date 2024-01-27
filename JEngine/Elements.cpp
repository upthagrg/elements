/*
File: Elements.cpp
Author: Glenn Upthagrove
Last Updated: 01/20/2024
Description: This file is the testing environment for the Elements application framework and the initial applications built off of it. 
*/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <errno.h>
#include <windows.h>
#include <thread>
#include <mutex>
#include <windows.h>
#include "garbage.hpp"
#include "JSON.hpp"
#include "Hydrogen.hpp"
#include "Oxygen.hpp"
#include "Xeon.hpp"
#include "JEngine.hpp"
#include "WebDoc.hpp"

void runserver(Xeon::WebServer* server) {
    server->Start();
}

void Test_Xeon() {
    string input;
    int Backlog = 20;
    int Buffer = KB(32);
    //WebDoc::home()
    Xeon::WebServer MyWebServer(Backlog, Buffer, 2, false, false);
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
}

void add_to_q(QUEUE* q) {
    string* ptr;
    for (int i = 0; i < 10; i++) {
        ptr = new string;
        *ptr = "This is iteration : ";
        ptr->append(to_string(i));
        q->Enqueue((void*)ptr);
    }
}

void test_queue() {
    QUEUE myq;
    std::thread t1(add_to_q, &myq);
    std::thread t2(add_to_q, &myq);
    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
    string* ptr;
    while (!myq.IsEmpty()) {
        ptr = (string*)myq.Dequeue();
        cout << *ptr << endl;
    }
}

int main(){
    Test_Xeon();
    return 0;
}

