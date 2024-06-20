//#pragma once
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <algorithm>
//#include <map>
//#include <vector>
//#include <errno.h>
//#include <windows.h>
//#include <thread>
//#include <mutex>
//#include <windows.h>
//#include "garbage.hpp"
//#include "JSON.hpp"
//#include "Hydrogen.hpp"
//#include "Oxygen.hpp"
//#include "Xeon.hpp"
//#include "WebDoc.hpp"
//
//#pragma region tools
//int* buildrandomdata(int size) {
//    int* data = new int[size];
//    for (int i = 0; i < size; i++) {
//        data[i] = rand();
//    }
//    return data;
//}
//#pragma endregion
//
//#pragma region condition variables
//CONDITION_VARIABLE cv;
//CRITICAL_SECTION m;
//
//void testvar(int id) {
//    //fake consumer
//    while (true) {
//        cout << "consumer " << id << " grabbing lock" << endl;
//        EnterCriticalSection(&m);
//        cout << "consumer " << id << " got lock, working" << endl;
//        cout << "consumer " << id << " cant do work, sleeping on condition" << endl;
//        SleepConditionVariableCS(&cv, &m, INFINITE);
//        cout << "consumer " << id << " woke up, working" << endl;
//        Sleep(5000);
//        cout << "consumer " << id << " releasing lock" << endl;
//        LeaveCriticalSection(&m);
//    }
//}
//
//void testconditionvar() {
//    //fake producer
//    InitializeConditionVariable(&cv);
//
//    InitializeCriticalSection(&m);
//
//    cout << "producer making consumers" << endl;
//    std::thread consumer_thread(testvar, 1);
//    std::thread consumer_thread2(testvar, 2);
//
//    while (true) {
//        cout << "producer grabbing lock" << endl;
//        EnterCriticalSection(&m);
//        cout << "producer got lock, making data" << endl;
//        Sleep(10100);
//        cout << "producer realeasing lock" << endl;
//        LeaveCriticalSection(&m);
//        cout << "producer waking other threads" << endl;
//        WakeConditionVariable(&cv);
//    }
//}
//#pragma endregion 
//
//#pragma region acceleration tests
//void no_simd_single_thread_bad_cache(int* data, int datasize, int iterations) {
//    //loop accross iterations for average
//    double elapsed_sum = 0.0;
//    double elapsed;
//    if (iterations < 1) {
//        ErrorAndDie(1, "bad number of iterations");
//    }
//    for (int it = 0; it < iterations; it++) {
//        //start timer
//        mybase.StartTimer();
//        //fake data processing load
//        for (int i = 0; i < datasize; i++) {
//            data[i] += rand();
//        }
//        //end timer
//        elapsed_sum += mybase.EndTimer();
//    }
//
//    elapsed = elapsed_sum / iterations;
//    cout << "avg. elapsed time: " << elapsed << endl;
//}
//void no_simd_multi_thread_bad_cache(int* data, int datasize, int threads) {
//    exit(1); //TODO: implement this function
//}
//#pragma endregion
//
//#pragma region Data Structures
//void teststack() {
//    string* days = new string[7];
//    days[0] = "Mon";
//    days[1] = "Tue";
//    days[2] = "Wed";
//    days[3] = "Thu";
//    days[4] = "Fri";
//    days[5] = "Sat";
//    days[6] = "Sun";
//
//    STACK daystack;
//
//    for (int i = 0; i < 7; i++) {
//        cout << "Stacking: " << days[i] << endl;
//        daystack.Stack((void*)&(days[i]), true);
//    }
//    for (int i = 0; i < 7; i++) {
//        cout << "Popped: " << *(string*)daystack.Pop() << endl;
//    }
//}
//
//void testqueue() {
//    string* days = new string[7];
//    days[0] = "Mon";
//    days[1] = "Tue";
//    days[2] = "Wed";
//    days[3] = "Thu";
//    days[4] = "Fri";
//    days[5] = "Sat";
//    days[6] = "Sun";
//
//    QUEUE dayqueue;
//
//    for (int i = 0; i < 7; i++) {
//        cout << "Queueing: " << days[i] << endl;
//        dayqueue.Enqueue((void*)&(days[i]), false);
//    }
//    for (int i = 0; i < 7; i++) {
//        cout << "Dequeued: " << *(string*)dayqueue.Dequeue() << endl;
//    }
//}
//
//void testJSON() {
////vector<string> data;
////vector<string> filter;
////string JSON = "{\"Age\":\"28\", \n  \"Sex\":\"Male\", \"Weight\":\"220\", \"Cars\":[\"Ford\", \"BMW\", \"Fiat\"], \"Son\":{\"Age\":\"2\", \"Name\":\"Gabe\"}}";
////cout << "Data: " << JSON << endl;
////data = TokenizeString(JSON, ":,\"   \n", filter);
////for (int i = 0; i < data.size(); i++) {
////    cout << data[i] << endl;
////}
////JSONObject("{\"Age\":\"28\", \n  \"Sex\":\"Male\", \"Age\":\"29\"}");
//}
//#pragma endregion
