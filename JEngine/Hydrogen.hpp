#pragma once
/*
File: Hydrogen.hpp
Author: Glenn Upthagrove
Last Updated: 01/20/2024
Description: This header file contains the most basic building blocks for the Elements framework.
This is the root of architecture and will contatin things like data structures, algorythms, helper functions, etc.
*/

#pragma comment(lib, "ws2_32.lib")
#include <WinSock.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <array>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <errno.h>
#include <ctime>
#include <time.h>
#include <thread>
#include <mutex>
#include <regex>
//#include "JSON.hpp"

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
using std::getline;

bool Architecture_Initialized = false;
std::mutex ArchLock;

namespace Hydrogen {
    class HydrogenArchBase {
    private:
        time_t timer_var;
    public:
        HydrogenArchBase();
        void StartTimer();
        double EndTimer();
    };
    HydrogenArchBase::HydrogenArchBase() {
        if (!Architecture_Initialized) {
            ArchLock.lock();
            std::srand(time(NULL));
            Architecture_Initialized = true;
            ArchLock.unlock();
        }
    }
    void HydrogenArchBase::StartTimer() {
        timer_var = time(NULL);
    }
    double HydrogenArchBase::EndTimer() {
        return timer_var - time(NULL);
    }
}

Hydrogen::HydrogenArchBase mybase;

bool str_equals(string, string);
bool str_equals(char*, char*);
bool str_equals(string, char*);
bool str_equals(char*, string);
string to_upper(string);

//Set of basic string functions{
//Set of basic string comparison functions{
//Compare string object to string object
bool str_equals(string str1, string str2) {
    transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
    return str1 == str2;
}
//Compare cstring to cstring
bool str_equals(char* str1, char* str2) {
    string s1, s2;
    s1.assign(str1);
    s2.assign(str2);
    return str_equals(s1, s2);
}
//Compare string object to cstring
bool str_equals(string str1, char* str2) {
    string s2;
    s2.assign(str2);
    return str_equals(str1, s2);
}
//Compare cstring to string object
bool str_equals(char* str1, string str2) {
    string s1;
    s1.assign(str1);
    return str_equals(s1, str2);
}
//}
//returns the all upper case version of a string
string to_upper(string input) {
    string str1 = input;
    transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    return str1;
}
//returns the all lower case version of a string
string to_lower(string input) {
    string str1 = input;
    transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    return str1;
}
//split a string on some set of delimiter charaters. The additional filter will remove any matching strings from the result vector
vector<string> TokenizeString(string input, const char* delim, vector<string> filter) {
    string token_string;
    int length = input.length() + 1;
    char* token;
    char* next_token = new char[length];
    char* buffer = new char[length];
    char* arr_buffer = new char[length];
    memset(next_token, '\0', length);
    memset(buffer, '\0', length);
    memset(arr_buffer, '\0', length);
    errno_t ret = strcpy_s(buffer, input.length() + 1, input.c_str());
    token = strtok_s(buffer, delim, &next_token);
    bool remove;
    bool continue_tokenizing = true;
    vector<string> tokens;
    while (continue_tokenizing) {
        remove = false;
        token_string = token;
        if (filter.size() > 0) {
            for (int j = 0; j < filter.size(); j++) {
                if (str_equals(token, filter[j])) {
                    remove = true;
                    break;
                }
                if (!remove) {
                    tokens.push_back(token_string);
                }
            }
        }
        else {
            tokens.push_back(token_string);
        }
        token = strtok_s(NULL, delim, &next_token);
        if (token == NULL) {
            continue_tokenizing = false;
        }
    }
    vector<string> newtokens;
    newtokens = tokens;
    return newtokens;
}
//}

//Error functions{
//Standardized way to error. pass in an int for the error code and a string for the error message
void ErrorAndDie(int Error_Code, string Error) {
    if (Error.empty()) {
        cout << "The application encountered an error." << endl;
    }
    else {
        cout << Error << endl;
    }
    //0 should not be used as an error code in the Elements framework
    if (Error_Code == 0) {
        Error_Code = 1;
    }
    cout << "Exiting with error code: " << Error_Code << endl;
    fflush(stdout);
    exit(Error_Code);
}
//Check for a result being 0. This is used because a lot of functions return a status, and if it is not 0 there was an error. 
void ErrorCheck0(int Result, int Error_Code, string Error) {
    if (Result != 0) {
        ErrorAndDie(Error_Code, Error);
    }
}
//}

//OS interfaces{
//Execute a system command. 
string Exec(const char* cmd) {
    std::array<char, 128> buffer;
    string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        ErrorAndDie(1, "popen() failed!");
        //throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
//}

#pragma region Data Structures
struct Node {
    struct Node* Next;
    struct Node* Prev;
    void* Data;
};

class QUEUE {
private:
    CRITICAL_SECTION QLock;
    CONDITION_VARIABLE QConVar;
    struct Node* First;
    struct Node* Last;
    int size;
public:
    QUEUE();
    void Enqueue(void*, bool);
    void* Dequeue();
    bool IsEmpty();
    void Lock();
    void Unlock();
    void print();
    int GetSize();
    CONDITION_VARIABLE* GetCondition();
    CRITICAL_SECTION* GetLock();
    void Sleep();
    void Wake(bool);
};
QUEUE::QUEUE() {
    First = NULL;
    Last = NULL;
    size = 0;
    InitializeCriticalSection(&QLock);
    InitializeConditionVariable(&QConVar);
}
void QUEUE::Enqueue(void* in, bool lock) {
    //Lock the queue
    if (lock) {
        this->Lock();
    }
    struct Node* ptr = new struct Node;
    //New Node's next is NUll
    ptr->Next = NULL;
    //New Node's previous is current last.
    //If this is the first node, the last is NULL and this previous will now also point to NULL.
    ptr->Prev = Last;
    //Assign the input pointer to the new Node's Data.
    ptr->Data = in;
    //Current Last next has to point to the new Node.
    //If it is the first Node we have no last node to update, but instead we need to declare this as the first
    if (Last != NULL) {
        Last->Next = ptr;
    }
    else {
        First = ptr;
    }
    //Last will always be the new node
    Last = ptr;
    //increment size
    size++;
    //Unlock the queue
    if (lock) {
        this->Unlock();
    }
}

void* QUEUE::Dequeue() {
    //Lock the queue
    this->Lock();
    void* data = NULL;
    //Can only get data if there are Nodes
    if (First != NULL) {
        //Store the data off.
        data = First->Data;
        //Track the First Node
        struct Node* ptr = First;
        //Move First pointer to the next Node.
        First = First->Next;
        //free the old first Node pointer
        delete ptr;
        //if first is now null, the queue is empty and las must also point to null since they pointed to the same Node before dequeue
        if (First == NULL) {
            Last = NULL;
        }
        //decrement size;
        size--;
    }
    //Unlock the queue
    this->Unlock();
    //Return the requested data.
    return data;
}

bool QUEUE::IsEmpty() {
    return First == NULL;
}


CONDITION_VARIABLE* QUEUE::GetCondition() {
    return &QConVar;
}

CRITICAL_SECTION* QUEUE::GetLock() {
    return &QLock;
}
void QUEUE::Lock() {
    EnterCriticalSection(&QLock);
}

void QUEUE::Unlock() {
    LeaveCriticalSection(&QLock);
}
void QUEUE::Sleep() {
    SleepConditionVariableCS(&QConVar, &QLock, INFINITE);
}
void QUEUE::Wake(bool all) {
    if (all) {
        WakeAllConditionVariable(&QConVar);
    }
    else {
        WakeConditionVariable(&QConVar);
    }
}

void QUEUE::print() {
    struct Node* ptr = NULL;
    if (First != NULL) {
        ptr = First;
        while (ptr != NULL) {
            cout << *((string*)ptr->Data) << endl;
            ptr = ptr->Next;
        }
    }
}
int QUEUE::GetSize() {
    return size;
}

class STACK {
private:
    CRITICAL_SECTION QLock;
    CONDITION_VARIABLE QConVar;
    struct Node* First;
    struct Node* Last;
    int size;
public:
    STACK();
    void Stack(void*, bool);
    void* Pop();
    bool IsEmpty();
    void Lock();
    void Unlock();
    void print();
    int GetSize();
    CONDITION_VARIABLE* GetCondition();
    CRITICAL_SECTION* GetLock();
    void Sleep();
    void Wake(bool);
};
STACK::STACK() {
    First = NULL;
    Last = NULL;
    size = 0;
    InitializeCriticalSection(&QLock);
    InitializeConditionVariable(&QConVar);
}
void STACK::Stack(void* in, bool lock) {
    //Lock the queue
    if (lock) {
        this->Lock();
    }
    struct Node* ptr = new struct Node;
    //New Node's next is NUll
    ptr->Next = NULL;
    //New Node's previous is current last.
    //If this is the first node, the last is NULL and this previous will now also point to NULL.
    ptr->Prev = Last;
    //Assign the input pointer to the new Node's Data.
    ptr->Data = in;
    //Current Last next has to point to the new Node.
    //If it is the first Node we have no last node to update, but instead we need to declare this as the first
    if (Last != NULL) {
        Last->Next = ptr;
    }
    else {
        First = ptr;
    }
    //Last will always be the new node
    Last = ptr;
    //increment size
    size++;
    //Unlock the queue
    if (lock) {
        this->Unlock();
    }
}

void* STACK::Pop() {
    //Lock the queue
    this->Lock();
    void* data = NULL;
    //Can only get data if there are Nodes
    if (Last != NULL) {
        //Store the data off.
        data = Last->Data;
        //Track the First Node
        struct Node* ptr = Last;
        //Move First pointer to the next Node.
        Last = Last->Prev;
        //free the old first Node pointer
        delete ptr;
        //if first is now null, the queue is empty and las must also point to null since they pointed to the same Node before dequeue
        if (Last == NULL) {
            First = NULL;
        }
        //decrement size;
        size--;
    }
    //Unlock the queue
    this->Unlock();
    //Return the requested data.
    return data;
}

bool STACK::IsEmpty() {
    return First == NULL;
}


CONDITION_VARIABLE* STACK::GetCondition() {
    return &QConVar;
}

CRITICAL_SECTION* STACK::GetLock() {
    return &QLock;
}
void STACK::Lock() {
    EnterCriticalSection(&QLock);
}

void STACK::Unlock() {
    LeaveCriticalSection(&QLock);
}
void STACK::Sleep() {
    SleepConditionVariableCS(&QConVar, &QLock, INFINITE);
}
void STACK::Wake(bool all) {
    if (all) {
        WakeAllConditionVariable(&QConVar);
    }
    else {
        WakeConditionVariable(&QConVar);
    }
}

void STACK::print() {
    struct Node* ptr = NULL;
    if (First != NULL) {
        ptr = First;
        while (ptr != NULL) {
            cout << *((string*)ptr->Data) << endl;
            ptr = ptr->Next;
        }
    }
}
int STACK::GetSize() {
    return size;
}
#pragma endregion


#pragma region Miscellanious helper functions
//Get the IPv4 address of this machine
string GetIP() {
    //TODO: Allow lookup for IPv6
    //Allow lookup of different media connections if multiple exists
    //Make similar function with vecotr<string> return of all connected types and data and option to include disconnected maybe a class instead of vector
    string IP = "";
    string IPConfig = Exec("IPConfig");
    vector<string> filter;
    filter.push_back(".");
    vector<string> Tokens = TokenizeString(IPConfig, "  \n", filter);
    for (int i = 0; i < Tokens.size(); i++) {
        if (Tokens[i].substr(0, 4) == "IPv4") {
            if (i + 3 < Tokens.size()) {
                IP = Tokens[i + 3];
                break;
            }
        }
    }
    return IP;
}
#pragma region Data size functions
//Get number of Bytes in a given number of KiloBytes
int KB(int Number) {
    return Number * 1024;
}

//Get number of Bytes in a given number of MegaBytes
int MB(int Number) {
    return KB(Number) * 1024;
}

//Get number of Bytes in a given number of GigaBytes
int GB(int Number) {
    return MB(Number) * 1024;
}
#pragma endregion
#pragma region Input functions
string GetStringInput(string message) {
    bool fail;
    string input = "";
    char next;
    do {
        fail = false;
        cout << message << endl;
        cin.clear();
        cin >> input;

        if (cin.eof() || cin.bad() || cin.fail()) {
            fail = true;
            if (cin.fail()) {
                cin.clear();
                next = cin.get();
            }
        }
        if (fail) {
            cout << "Not a valid entry" << endl;
        }
    } while (fail);
    return input;
}

bool GetBoolInput(string message) {
    bool fail;
    bool input = false;
    char next;
    do {
        fail = false;
        cout << message << endl;
        cin.clear();
        cin >> input;

        if (cin.eof() || cin.bad() || cin.fail()) {
            fail = true;
            if (cin.fail()) {
                cin.clear();
                next = cin.get();
            }
        }
        if (fail) {
            cout << "Not a valid entry" << endl;
        }
    } while (fail);
    return input;
}

int GetIntInput(string message, bool positive, bool negative, bool zero) {
    bool fail;
    int input = -1;
    char next;
    if (!positive && !negative && !zero) {
        ErrorAndDie(1001, "No valid input option allowed");
    }
    do {
        fail = false;
        cout << message << endl;
        cin.clear();
        cin >> input;

        if (cin.eof() || cin.bad() || cin.fail()) {
            fail = true;
            if (cin.fail()) {
                cin.clear();
                next = cin.get();
            }
        }
        if (!fail) {
            if (input == 0 && !zero) {
                fail = true;
            }
            else if (input < 0 && !negative) {
                fail = true;
            }
            else if (input > 0 && !positive) {
                fail = true;
            }
        }
        if (fail) {
            cout << "Not a valid entry" << endl;
        }
    } while (fail);
    return input;
}

float GetFloatInput(string message, bool positive, bool negative, bool zero) {
    bool fail;
    float input = -1.0;
    char next;
    if (!positive && !negative && !zero) {
        ErrorAndDie(1001, "No valid input option allowed");
    }
    do {
        fail = false;
        cout << message << endl;
        cin.clear();
        cin >> input;

        if (cin.eof() || cin.bad() || cin.fail()) {
            fail = true;
            if (cin.fail()) {
                cin.clear();
                next = cin.get();
            }
        }
        if (!fail) {
            if (input == 0.0 && !zero) {
                fail = true;
            }
            else if (input < 0.0 && !negative) {
                fail = true;
            }
            else if (input > 0.0 && !positive) {
                fail = true;
            }
        }
        if (fail) {
            cout << "Not a valid entry" << endl;
        }
    } while (fail);
    return input;
}

double GetDoubleInput(string message, bool positive, bool negative, bool zero) {
    bool fail;
    double input = -1.0;
    char next;
    if (!positive && !negative && !zero) {
        ErrorAndDie(1001, "No valid input option allowed");
    }
    do {
        fail = false;
        cout << message << endl;
        cin.clear();
        cin >> input;

        if (cin.eof() || cin.bad() || cin.fail()) {
            fail = true;
            if (cin.fail()) {
                cin.clear();
                next = cin.get();
            }
        }
        if (!fail) {
            if (input == 0.0 && !zero) {
                fail = true;
            }
            else if (input < 0.0 && !negative) {
                fail = true;
            }
            else if (input > 0.0 && !positive) {
                fail = true;
            }
        }
        if (fail) {
            cout << "Not a valid entry" << endl;
        }
    } while (fail);
    return input;
}
#pragma endregion
#pragma endregion