#pragma once
/*
File: Hydrogen.hpp
Author: Glenn Upthagrove
Last Updated: 01/20/2024
Description: This header file contains the most basic building blocks for the Elements framework. 
This is the root of architecture and will contatin things like data structures, algorythms, helper functions, etc.
*/

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
#include <windows.h>
#include <errno.h>
#include "JSON.hpp"

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

void engine_test();
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
    std::mutex QLock;
    struct Node* First;
    struct Node* Last;
    int size;
public:
    QUEUE();
    void Enqueue(void*);
    void* Dequeue();
    bool IsEmpty();
    void Lock();
    void Unlock();
    void print();
    int GetSize();
};
QUEUE::QUEUE() {
    First = NULL;
    Last = NULL;
    size = 0;
}
void QUEUE::Enqueue(void* in) {
    //Lock the queue
    this->Lock();
    string input = *((string*)in);
    //New Node
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
    this->Unlock();
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
    }
    //decrement size;
    size--;
    //Unlock the queue
    this->Unlock();
    //Return the requested data.
    return data;
}

bool QUEUE::IsEmpty() {
    return First == NULL;
}

void QUEUE::Lock() {
    QLock.lock();
}

void QUEUE::Unlock() {
    QLock.unlock();
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
#pragma endregion



//Miscellanious helper functions{
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
            if(i + 3 < Tokens.size()){
                IP = Tokens[i + 3];
                break;
            }
        }
    }
    return IP;
}
//}