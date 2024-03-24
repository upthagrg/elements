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

CONDITION_VARIABLE cv;
CRITICAL_SECTION m;


void testvar(int id) {
    //fake consumer
    while (true) {
        cout << "consumer " << id << " grabbing lock" << endl;
        EnterCriticalSection(&m);
        cout << "consumer " << id << " got lock, working" << endl;
        cout << "consumer " << id << " cant do work, sleeping on condition" << endl;
        SleepConditionVariableCS(&cv, &m, INFINITE);
        //(*cvar).wait(*ulm);
        cout << "consumer " << id << " woke up, working" << endl;
        Sleep(5000);
        cout << "consumer " << id << " releasing lock" << endl;
        LeaveCriticalSection(&m);
        //(*ulm).unlock();
    }
}

void testconditioncar() {
    //fake producer
    InitializeConditionVariable(&cv);

    InitializeCriticalSection(&m);

    cout << "producer making consumers" << endl;
    std::thread consumer_thread(testvar, 1);
    std::thread consumer_thread2(testvar, 2);

    while (true) {
        cout << "producer grabbing lock" << endl;
        EnterCriticalSection(&m);
        cout << "producer got lock, making data" << endl;
        Sleep(10100);
        cout << "producer realeasing lock" << endl;
        LeaveCriticalSection(&m);
        cout << "producer waking other threads" << endl;
        WakeConditionVariable(&cv);
    }
}

int main(){
    Xeon::Test_Xeon();
    exit(0);
    return 0;
}

