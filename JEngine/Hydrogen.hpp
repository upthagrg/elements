#pragma once
/*
File: Hydrogen.hpp
Author: Glenn Upthagrove
Last Updated: 12/10/2024
Description: This header file contains the most basic building blocks for the Elements framework.
This is the root of architecture and will contatin things like data structures, algorythms, helper functions, etc.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <errno.h>
#include <ctime>
#include <time.h>
#include <thread>
#include <regex>
#include <limits>
#include <map>
#include <unordered_map>
#include <filesystem>
#include <sys/stat.h>

using std::string;
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
CRITICAL_SECTION ArchLock;

namespace Hydrogen {
#pragma region Timer
    class Timer {
    private:
        time_t Begining;
        double Elapsed;
        CRITICAL_SECTION TimerLock;
    public:
        Timer();
        Timer(const Timer&);
        ~Timer();
        void Start();
        double Stop();
        void Reset();
    };
    Timer::Timer() {
        InitializeCriticalSection(&TimerLock);
        Begining = -1;
        Elapsed = 0;
    }
    Timer::Timer(const Timer& src) {
        InitializeCriticalSection(&TimerLock);
        Begining = src.Begining;
        Elapsed = src.Elapsed;
    }
    Timer::~Timer() {
        DeleteCriticalSection(&TimerLock);
    }
    void Timer::Start() {
        EnterCriticalSection(&TimerLock);
        Begining = time(NULL);
        LeaveCriticalSection(&TimerLock);
    }
    double Timer::Stop() {
        EnterCriticalSection(&TimerLock);
        Elapsed += (time(NULL) - Begining);
        LeaveCriticalSection(&TimerLock);
        return Elapsed;
    }
    void Timer::Reset() {
        EnterCriticalSection(&TimerLock);
        Elapsed = 0;
        LeaveCriticalSection(&TimerLock);
    }
#pragma endregion
#pragma region HydrogenArchBase
    class HydrogenArchBase {
    private:
        std::unordered_map<int, string> UUIDs;
        std::unordered_map<int, Timer> Timers;
        std::unordered_map<void*, string> Pointers;
        CRITICAL_SECTION ObjectLock;
        CRITICAL_SECTION ScreenLock;
        
    public:
        HydrogenArchBase();
        void StartTimer(int);
        double EndTimer(int);
        void ResetTimer(int);
        int CreateTimer();
        int GetNextID();
        int GetNextID(string);
        void Display(string);
        void Display(unsigned char *);
        int GenRandomInt(int, int);
        float GenRandomFloat(float, float);
        double GenRandomFloat(double, double);
        void AddPointer(void*, string);
        void DeletePointer(void*);
        void PrintPointers();
    };
    HydrogenArchBase::HydrogenArchBase() {
        if (!Architecture_Initialized) {
            InitializeCriticalSection(&ArchLock);
            EnterCriticalSection(&ArchLock);
            InitializeCriticalSection(&ScreenLock);
            InitializeCriticalSection(&ObjectLock);
            std::srand(time(NULL));
            Architecture_Initialized = true;
            LeaveCriticalSection(&ArchLock);
        }
    }
    void HydrogenArchBase::StartTimer(int Timer) {
        if (Timers.find(Timer) != Timers.end()) {
            Timers[Timer].Start();
        }
    }
    double HydrogenArchBase::EndTimer(int Timer) {
        if (Timers.find(Timer) != Timers.end()) {
            return Timers[Timer].Stop();
        }
        else {
            return -1;
        }
    }
    void HydrogenArchBase::ResetTimer(int Timer) {
        if (Timers.find(Timer) != Timers.end()) {
            Timers[Timer].Reset();
        }
    }
    int HydrogenArchBase::CreateTimer() {
        int TimerID = GetNextID("TIMERID");
        Timer NewTimer;
        Timers[TimerID] = NewTimer;
        return TimerID;
    }
    int HydrogenArchBase::GetNextID() {
        return GetNextID("");
    }
    int HydrogenArchBase::GetNextID(string IDType) {
        int NewID = -1;
        bool IDValid = false;
        do {
            NewID = rand();
            if (INT_MAX > RAND_MAX) {
                NewID = NewID << 16;
                NewID = NewID + rand();
            }
            if (UUIDs.find(NewID) == UUIDs.end() && NewID > 0) {
                UUIDs[NewID] = IDType;
                IDValid = true;
            }

        } while (!IDValid);
        int ret = NewID;
        return ret;
    }
    void HydrogenArchBase::Display(string Message) {
        EnterCriticalSection(&ScreenLock);
        cout << Message << endl;
        LeaveCriticalSection(&ScreenLock);
    }
    void HydrogenArchBase::Display(unsigned char* Message) {
        EnterCriticalSection(&ScreenLock);
        cout << Message << endl;
        LeaveCriticalSection(&ScreenLock);
    }
    int HydrogenArchBase::GenRandomInt(int min, int max) {
        bool finish = false;
        int rnum;
        int rnum2;
        int range;
        if (min >= max) {
            return 0;
        }
        range = (max + 1) - min;
        rnum = rand();
        if ((RAND_MAX < INT_MAX) && max > RAND_MAX) {
            rnum2 = GenRandomInt(0, 16);
            if (rnum2 != 0) {
                rnum = rnum << rnum2;
                rnum2 = GenRandomInt(1, RAND_MAX);
                rnum = rnum2 | rnum;
            }
        }
        rnum = ((rnum % range) + min);
        int ret = rnum;
        return ret;
    }
    float HydrogenArchBase::GenRandomFloat(float min, float max) {
        return 0.0;
    }
    double HydrogenArchBase::GenRandomFloat(double min, double max) {
        return 0.0;
    }
    void HydrogenArchBase::AddPointer(void* ptr, string from) {
        Pointers[ptr] = from;
    }
    void HydrogenArchBase::DeletePointer(void* ptr) {
        if (Pointers.find(ptr) != Pointers.end()) {
            Pointers.erase(ptr);
        }
    }
    void HydrogenArchBase::PrintPointers() {
        std::unordered_map<void*, string>::iterator it = Pointers.begin();
        string msg;
        uint64_t ptr;
        if (Pointers.size() == 0) {
            Display("All tracked pointers have been freed");
        }
        else {
            while (it != Pointers.end()) {
                msg = "";
                msg.append("pointer ");
                ptr = (uint64_t)(it->first);
                msg.append(to_string(ptr));
                msg.append(" was never freed, allocated by ");
                msg.append(it->second);
                Display(msg);
                it++;
            }
        }
    }
}

Hydrogen::HydrogenArchBase MyBase;
#pragma endregion

bool str_equals(string, string);
bool str_equals(char*, char*);
bool str_equals(string, char*);
bool str_equals(char*, string);
string to_upper(string);

//Function definitions
void ErrorAndDie(int, string);

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
    char* freedata;
    char* next_token = new char[length];
    char* buffer = new char[length];
    char* arr_buffer = new char[length];

    memset(next_token, '\0', length);
    memset(buffer, '\0', length);
    memset(arr_buffer, '\0', length);
    errno_t ret = strcpy_s(buffer, input.length() + 1, input.c_str());
    token = strtok_s(buffer, delim, &next_token);
    freedata = token;
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
//split a string on some set of delimiter charaters. The additional filter will remove any matching strings from the result vector. Include index of the original string
struct StringTokenWithIndex {
    string Token;
    int Index;
};
bool CharacterIn(const char* Chars, char check) {
    for (int i = 0; i < strlen(Chars); i++) {
        if (Chars[i] == check) {
            return true;
        }
    }
    return false;
}
vector<StringTokenWithIndex> TokenizeStringWithIndex(string input, const char* delim, vector<string> filter) {
    bool Done = false;
    StringTokenWithIndex Item;
    vector<StringTokenWithIndex> ReturnSet;
    if (filter.size() != 0) {
        ErrorAndDie(1, "Filter not yet supported");
    }
    int it = 0;
    while (!Done) {
        for (int i = it; i < input.size(); i++) {
            //if not in delim, add character to the current token
            if (!CharacterIn(delim, input[i])) {
                Item.Token.push_back(input[i]);
                if (i + 1 >= input.size()) {
                    Item.Index = it;
                    if (Item.Token != "") {
                        ReturnSet.push_back(Item);
                    }
                    Done = true;
                }
            }
            //if in delim, end token
            else {
                Item.Index = it;
                if (Item.Token != "") {
                    ReturnSet.push_back(Item);
                }
                if (i + 1 <= input.length()) {
                    it = i + 1;
                    Item.Token = "";
                }
                else {
                    Done = true;
                }
            }
        }
    }
    vector<StringTokenWithIndex> FinalReturnSet = ReturnSet;
    return FinalReturnSet;
}
//}

//Error functions{
//Standardized way to error. pass in an int for the error code and a string for the error message
void ErrorAndDie(int Error_Code, string Error) {
    if (Error.empty()) {
        MyBase.Display("The application encountered an error.");
    }
    else {
        MyBase.Display(Error);
    }
    //0 should not be used as an error code in the Elements framework
    if (Error_Code == 0) {
        Error_Code = 1;
    }
    string msg = "";
    msg.append("Exiting with error code: ");
    msg.append(to_string(Error_Code));
    MyBase.Display(msg);
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
    void Signal(bool);
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
    MyBase.AddPointer((void*)ptr, "Hydrogen QUEUE::Enqueue");
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
    void* ret;
    //Can only get data if there are Nodes
    if (First != NULL) {
        //Store the data off.
        data = First->Data;
        //Track the First Node
        struct Node* ptr = First;
        //Move First pointer to the next Node.
        First = First->Next;
        //free the old first Node pointer
        MyBase.DeletePointer((void*)ptr);
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
    ret = data;
    return ret;
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
void QUEUE::Signal(bool all) {
    if (all) {
        WakeAllConditionVariable(&QConVar);
    }
    else {
        WakeConditionVariable(&QConVar);
    }
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
            MyBase.Display(*((string*)ptr->Data));
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
    MyBase.AddPointer((void*)ptr, "Hydrogen STACK::Stack");
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
    void* ret;
    //Can only get data if there are Nodes
    if (Last != NULL) {
        //Store the data off.
        data = Last->Data;
        //Track the First Node
        struct Node* ptr = Last;
        //Move First pointer to the next Node.
        Last = Last->Prev;
        //free the old first Node pointer
        MyBase.DeletePointer((void*)ptr);
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
    ret = data;
    return ret;
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
            MyBase.Display(*((string*)ptr->Data));
            ptr = ptr->Next;
        }
    }
}
int STACK::GetSize() {
    return size;
}
#pragma endregion

#pragma region Data Types
class bin {
private:
    unsigned char* data;
    int length;
public:
    bin();
    ~bin();
    bin(const bin&);
    unsigned char& operator[](int);
    void operator=(const bin&);
    void operator=(const string&);
    void operator=(const char*&);
    void operator=(const unsigned char*&);
    unsigned char* GetData();
    int GetLength();
    void SetData(unsigned char*, int);
    void SetData(char*, int);
    void SetData(const char*, int);
    void SetData(string);
};
bin::bin() {
    data = NULL;
    length = 0;
}
bin::~bin() {
    MyBase.DeletePointer((void*)data);
    delete[] data;
}
bin::bin(const bin& src) {
    length = src.length;
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::bin");
    memcpy_s((void*)data, length, (void*)(src.data), length);
}
unsigned char& bin::operator[](int i) {
    if (i < 0 || i > length) {
        ErrorAndDie(1, "Array index outside the bounds of the array");
    }
    return data[i];
}
void bin::operator=(const bin& src) {
    length = src.length;
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::=bin");
    memcpy_s((void*)data, length, (void*)(src.data), length);
}
void bin::operator=(const string& src) {
    length = src.length();
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::=string");
    memcpy_s((void*)data, length, (void*)(src.c_str()), length);
}
void bin::operator=(const char*& src) {
    length = strlen(src);
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::=char*");
    memcpy_s((void*)data, length, (void*)(src), length);
}
void bin::operator=(const unsigned char*& src) {
    length = strlen((char*)src);
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::=unsigned char*");
    memcpy_s((void*)data, length, (void*)(src), length);
}
unsigned char* bin::GetData() {
    return data;
}
int bin::GetLength() {
    return length;
}
void bin::SetData(unsigned char* src, int srclen) {
    length = srclen;
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::SetData unsigned char*");
    memcpy_s((void*)data, length, (void*)(src), length);
}
void bin::SetData(char* src, int srclen) {
    length = srclen;
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    memcpy_s((void*)data, length, (void*)(src), length);
}
void bin::SetData(const char* src, int srclen) {
    length = srclen;
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::SetData char*");
    memcpy_s((void*)data, length, (void*)(src), length);
}
void bin::SetData(string src) {
    length = src.length();
    if (data != NULL) {
        MyBase.DeletePointer((void*)data);
        delete[] data;
    }
    data = new unsigned char[length];
    MyBase.AddPointer((void*)data, "Hydrogen bin::SetData string");
    memcpy_s((void*)data, length, (void*)(src.c_str()), length);
}

class HFile {
private:
    bool Binary = false; //Treated as text file by default
    string Path;
    FILE* File;
    int FileSize = 0;
    unsigned char* FileData;
    string FileDataString;

public:
    HFile();
    HFile(string);
    HFile(string, bool);
    ~HFile();
    HFile(const HFile&);
    int Size();
    unsigned char* Data();
    string DataString();
};
HFile::HFile(){
    File = NULL;
    FileSize = 0;
    FileData = NULL;
}
HFile::HFile(string Requested_File, bool IsBinary) {
    Binary = IsBinary;
    Path = Requested_File;
    int BytesRead = 0;
    File = fopen(Requested_File.c_str(), "rb");
    if (!File) {
        ErrorAndDie(404, "file not found");
    }
    fseek(File, 0, SEEK_END);
    if (Binary) {
        FileSize = ftell(File);
    }
    else {
        FileSize = ftell(File) + 1; //the +1 is good for text, may be an issue for binary
    }
    fseek(File, 0, SEEK_SET);

    FileData = new unsigned char[FileSize];
    MyBase.AddPointer((void*)FileData, "Hydrogen HFile");
    memset(FileData, '\0', FileSize);
    do {
        BytesRead = fread(FileData, 1, FileSize, File);
    } while (BytesRead > 0);

    fclose(File);

    ifstream inputFile(Requested_File.c_str());
    if (!inputFile.is_open()) {
        ErrorAndDie(404, "file not found");
    }
    string line;
    while (std::getline(inputFile, line)) {
        FileDataString.append(line);
    }
    inputFile.close();
}

HFile::HFile(string Requested_File) {
    Path = Requested_File;
    int BytesRead = 0;
    File = fopen(Requested_File.c_str(), "rb");
    if (!File) {
        ErrorAndDie(404, "file not found");
    }
    fseek(File, 0, SEEK_END);
    if (Binary) {
        FileSize = ftell(File);
    }
    else {
        FileSize = ftell(File) + 1; //the +1 is good for text, may be an issue for binary
    }
    fseek(File, 0, SEEK_SET);

    FileData = new unsigned char[FileSize];
    MyBase.AddPointer((void*)FileData, "Hydrogen HFile");
    memset(FileData, '\0', FileSize);
    do {
        BytesRead = fread(FileData, 1, FileSize, File);
    } while (BytesRead > 0);

    fclose(File);

    ifstream inputFile(Requested_File.c_str());
    if (!inputFile.is_open()) {
        ErrorAndDie(404, "file not found");
    }
    string line;
    while (std::getline(inputFile, line)) {
        FileDataString.append(line);
    }
    inputFile.close();
}
HFile::HFile(const HFile& Data) {
    Path = Data.Path;
    FileSize = Data.FileSize;
    FileData = new unsigned char[FileSize];
    MyBase.AddPointer((void*)FileData, "Hydrogen HFile Copy Constructor");
    memset(FileData, '\0', FileSize);
    for (int i = 0; i < FileSize; i++) {
        FileData[i] = Data.FileData[i];
    }
    FileDataString = Data.FileDataString;
}
HFile::~HFile() {
    MyBase.DeletePointer((void*)FileData);
    delete[] FileData;
}
int HFile::Size() {
    return FileSize;
}
unsigned char* HFile::Data() {
    return FileData;
}
string HFile::DataString() {
    return FileDataString;
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
    string ret = IP;
    return ret;
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
        MyBase.Display(message);
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
            MyBase.Display("Not a valid entry");
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
        MyBase.Display(message);
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
            MyBase.Display("Not a valid entry");
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
        MyBase.Display(message);
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
            MyBase.Display("Not a valid entry");
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
        MyBase.Display(message);
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
            MyBase.Display("Not a valid entry");
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
        MyBase.Display(message);
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
            MyBase.Display("Not a valid entry");
        }
    } while (fail);
    return input;
}
#pragma endregion
#pragma endregion