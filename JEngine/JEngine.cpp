// JEngine.cpp : This file contains the 'main' function. Program execution begins and ends here.
// This program will be command line based and allows the creation and play of text adventure games. 
// The engine will heavily use JSON to store data.
//

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


#include <iostream>
#include <string>
#include <map>
#include <errno.h>
#include "garbage.hpp"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::map;

class JSONObject {
private:
    map<string, string> Items;
public:
    JSONObject();
    JSONObject(string);
    string JSONString(bool);
    void additem(string, string);
    bool parse(string);
    void print(bool);
};

JSONObject::JSONObject() {
    Items.clear();
}

JSONObject::JSONObject(string Input) {
    this->parse(Input);
}

string JSONObject::JSONString(bool indent) {
    map<string, string>::iterator it = Items.begin();
    map<string, string>::iterator it_next = Items.begin();
    if (it_next != Items.end()) {
        ++it_next;
    }

    string JSONString = "";
    // Iterate through the map and append the elements
    JSONString.append("{");
    while (it != Items.end()) {
        if (indent) {
            JSONString.append("\n   ");
        }
        JSONString.append("\"");
        JSONString.append(it->first);
        JSONString.append("\"");
        JSONString.append(":");
        JSONString.append("\"");
        JSONString.append(it->second);
        JSONString.append("\"");
        if (it_next != Items.end()) {
            JSONString.append(",");
        }
        ++it;
        if (it_next != Items.end()) {
            ++it_next;
        }
    }
    if (indent) {
        JSONString.append("\n");
    }
    JSONString.append("}");
    return JSONString;
}

void JSONObject::additem(string Item, string Value) {
    Items[Item] = Value;
}

bool JSONObject::parse(string input) {
    bool sucess = true;
    const string delim = "  {}:\"\n";
    char *token;
    char *next_token;
    string item = "";
    string value = "";
    char *buffer = new char[input.length() + 1];
    errno_t ret = strcpy_s(buffer, input.length() + 1, input.c_str());

    Items.clear();
    token = strtok_s(buffer, delim.c_str(), &next_token);
    if (token == NULL) {
        sucess = false;
    }

    while (token != NULL)
    {
        if (item.empty()) {
            item.assign(token);
        }
        else{
            value.assign(token);
            this->additem(item, value);
            item = "";
            value = "";
        }
        token = strtok_s(NULL, delim.c_str(), &next_token);
    }

    if (!item.empty() || !value.empty()) {
        sucess = false;
    }

    return sucess;
}

void JSONObject::print(bool indent) {
    cout << this->JSONString(indent) << endl;
}

int main()
{
    // This is for testing
    int size;
    int* array;
    string parse_test = "{\n    \"Name\":\"Glenn\"\n    \"Age\":\"28\"\n}";
    string parse_test2 = "{\n    \"Make\":\"Ford\"\n    \"Model\":\"F150\"\n}";

    JSONObject jo;
    jo.additem("Name", "Glenn");
    jo.additem("Age", "27");
    jo.additem("Gender", "Male");


    cout << "manually added items to jo: " << endl;
    jo.print(true);

    jo.parse(parse_test);

    cout << "jo after parse " << endl;
    jo.print(true);

    JSONObject jo2(parse_test2);
    cout << "jo2 built with constructor from string: " << endl;
    jo2.print(true);

    cout << "jo2 JSON string: " << endl;
    cout << jo2.JSONString(false) << endl;

    cout << "Enter array size:" << endl;
    cin >> size;
    array = new int[size];
    for (int i = 0; i < size; i++) {
        cout << "enter a number " << endl;
        cin >> array[i];
    }
    cout << "You enetered:" << endl;
    for (int i = 0; i < size; i++) {
        cout << "   " << array[i] << endl;
    }
    free(array);
    return 0;
}