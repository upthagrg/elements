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
#include "garbage.hpp"
#include "JSON.hpp"
#include "Hydrogen.hpp"
#include "Oxygen.hpp"
#include "Xeon.hpp"
#include "JEngine.hpp"
#include "WebDoc.hpp"

void Test_Xeon() {
    int Backlog = 20;
    int Buffer = 30720;
    //WebDoc::home()
    Xeon::WebServer MyWebServer(Backlog, Buffer, true, false);
    //Xeon::LocalServer MyLocalServer(Backlog, Buffer, true, true);
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

