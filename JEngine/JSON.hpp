#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <errno.h>
#include "garbage.hpp"

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

class JSONObject {
private:
    map<string, string> Items;
    map<string, vector<string>> Arrays;
    map<string, string>::iterator it_internal;
    map<string, string>::iterator it_internal_next;
    map<string, vector<string>>::iterator it_arr_internal;
    map<string, vector<string>>::iterator it_arr_internal_next;
    string file;
    bool _iswhitespace(char*);

public:
    map<string, string>::iterator iterator;
    map<string, string>::iterator begin();
    map<string, string>::iterator end();
    JSONObject();
    JSONObject(string);
    JSONObject(JSONObject& obj);
    string JSONString(bool);
    void additem(string, string);
    //void additem(string, bool);
    //void additem(string, int);
    //void additem(string, unsigned int);
    //void additem(string, long);
    //void additem(string, unsigned long);
    //void additem(string, long long);
    //void additem(string, unsigned long long);
    //void additem(string, double);
    //void additem(string, float);
    //void additem(string, long double);
    //void additem(string, char);
    //void additem(string, char*);
    void addelement(string, string);
    bool parse(string);
    string getItem(string);
    bool exists(string);
    vector<string> getElements(string);
    string getElement(string, int);
    void printElements(string, bool);
    string ElementsJSONstring(string, bool);
    bool Item_equals(string, string);
    bool Element_equals(string, int, string);
    void print(bool);
    bool open(string);
    void write(bool);
    void write(string, bool);
};

JSONObject::JSONObject() {
    Items.clear();
    Arrays.clear();
    it_internal = Items.begin();
    it_internal_next = Items.begin();
    it_arr_internal = Arrays.begin();
    it_arr_internal_next = Arrays.begin();

}

JSONObject::JSONObject(string Input) {
    Items.clear();
    Arrays.clear();
    this->parse(Input);
    it_internal = Items.begin();
    it_internal_next = Items.begin();
    if (it_internal_next != Items.end()) {
        ++it_internal_next;
    }
    it_arr_internal = Arrays.begin();
    it_arr_internal_next = Arrays.begin();
    if (it_arr_internal_next != Arrays.end()) {
        ++it_arr_internal_next;
    }
}

JSONObject::JSONObject(JSONObject& Obj) {
    this->parse(Obj.JSONString(false));
}
bool JSONObject::_iswhitespace(char* input) {
    bool result = true;
    int i = 0;
    while ((i < strlen(input)) && result) {
        if (not (isblank(input[i]))) {
            result = false;
        }
        i++;
    }
    return result;
}

string JSONObject::JSONString(bool indent) {
    string JSONString = "";
    //Item iterators
    this->it_internal = Items.begin();
    this->it_internal_next = Items.begin();
    //Array iterators
    this->it_arr_internal = Arrays.begin();
    this->it_arr_internal_next = Arrays.begin();

    //track one ahead in Items
    if (it_internal_next != Items.end()) {
        ++it_internal_next;
    }

    // Iterate through the map and append the elements
    JSONString.append("{");
    while (it_internal != Items.end()) {

        if (indent) {
            JSONString.append("\n   ");
        }

        JSONString.append("\"");
        JSONString.append(it_internal->first);
        JSONString.append("\"");
        JSONString.append(":");
        JSONString.append("\"");
        JSONString.append(it_internal->second);
        JSONString.append("\"");

        if ((it_internal_next != Items.end()) || it_arr_internal != Arrays.end()) {
            JSONString.append(",");
        }

        //increment Items iterator
        ++it_internal;
        //if the Items next iterator is not at the end it too can increment forward
        if (it_internal_next != Items.end()) {
            ++it_internal_next;
        }
    }

    //track one ahead in Items
    if (it_arr_internal_next != Arrays.end()) {
        ++it_arr_internal_next;
    }
    //append array JSON strings
    while (it_arr_internal != Arrays.end()) {
        JSONString.append(ElementsJSONstring(it_arr_internal->first, indent));
        //if there is a next array, append a comma after this one
        if ((it_arr_internal_next != Arrays.end())) {
            JSONString.append(",");
        }

        //increment Arrays iterator forward
        ++it_arr_internal;
        //if the Arrays next iterator is not at the end it too can increment forward
        if (it_arr_internal_next != Arrays.end()) {
            ++it_arr_internal_next;
        }
    }

    //Back to outer JSON object
    if (indent) {
        JSONString.append("\n");
    }

    JSONString.append("}");
    return JSONString;
}

string JSONObject::ElementsJSONstring(string Item, bool indent) {
    string JSONString = "";
    if (indent) {
        JSONString.append("\n   ");
    }
    JSONString.append("\"");
    JSONString.append(Item);
    JSONString.append("\"");
    JSONString.append(":");
    if (indent) {
        JSONString.append("\n   ");
    }
    JSONString.append("[");
    if (Arrays[Item].size() > 0) {
        for (int i = 0; i < Arrays[Item].size(); i++) {
            if (indent) {
                JSONString.append("\n       ");
            }
            JSONString.append("\"");
            JSONString.append(Arrays[Item][i]);
            JSONString.append("\"");
            if (i < (Arrays[Item].size() - 1)) {
                JSONString.append(",");
            }
        }
    }
    if (indent) {
        JSONString.append("\n   ");
    }
    JSONString.append("]");
    return JSONString;
}

void JSONObject::additem(string Item, string Value) {
    Items[Item] = Value;
}

//void JSONObject::additem(string Item, bool Value) {
//    if (Value) {
//        Items[Item] = "1";
//    }
//    else {
//        Items[Item] = "0";
//    }
//}
//
//void JSONObject::additem(string Item, int Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, unsigned int Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, long Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, unsigned long Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, long long Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, unsigned long long Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, double Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, float Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, long  double Value) {
//    Items[Item] = to_string(Value);
//}
//
//void JSONObject::additem(string Item, char Value) {
//    string tmp = "";
//    tmp += Value;
//    Items[Item] = Value;
//}
//
//void JSONObject::additem(string Item, char* Value) {
//    string tmp;
//    tmp.assign(Value);
//    Items[Item] = Value;
//}

//Parse a JASON string into this JSON object
bool JSONObject::parse(string input) {
    const string delim = ",{}:\"\n";
    bool sucess = true;
    char* token;
    char* next_token;
    char* arr_token;
    char* next_arr_token;
    string item = "";
    string value = "";
    string element = "";
    char* buffer = new char[input.length() + 1];
    char* arr_buffer = new char[input.length() + 1];
    errno_t ret = strcpy_s(buffer, input.length() + 1, input.c_str());

    Items.clear();
    token = strtok_s(buffer, delim.c_str(), &next_token);
    while (token != NULL)
    {
        if (not (this->_iswhitespace(token))) {
            //This Value is an array
            if (token[0] == '[') {
                if (item.empty()) {
                    //array found as an Item
                    sucess = false;
                    break;
                }
                //seek the closing ]
                for (int i = 0; i < strlen(next_token); i++) {
                    if (next_token[i] == ']') {
                        string temp;
                        temp.assign(next_token);
                        //pull the whole array as a string
                        temp = temp.substr(0, i);
                        //move next_token to the address of the next character after the closing ]
                        next_token = &next_token[i + 1];
                        //now you loop across the elements and call the addelement(item, <element>)
                        ret = strcpy_s(arr_buffer, temp.length() + 1, temp.c_str());
                        arr_token = strtok_s(arr_buffer, delim.c_str(), &next_arr_token);
                        while (arr_token != NULL) {
                            if (not (this->_iswhitespace(arr_token))) {
                                element.assign(arr_token);
                                this->addelement(item, element);
                            }
                            arr_token = strtok_s(NULL, delim.c_str(), &next_arr_token);
                        }
                        //clear item
                        item = "";
                    }
                }
            }
            //This is an Item
            else if (item.empty()) {
                item.assign(token);
            }
            //This is a simple Value
            else {
                value.assign(token);
                this->additem(item, value);
                item = "";
                value = "";
            }
        }
        token = strtok_s(NULL, delim.c_str(), &next_token);
    }

    if (!item.empty() || !value.empty()) {
        sucess = false;
    }

    return sucess;
}

string JSONObject::getItem(string Item) {
    if (this->exists(Item)) {
        return Items[Item];
    }
    else {
        return "";
    }
}

bool JSONObject::exists(string Item) {
    it_internal = Items.find(Item);
    if (it_internal == Items.end()) {
        return false;
    }
    else {
        return true;
    }
}

void JSONObject::addelement(string Item, string Element) {
    Arrays[Item].push_back(Element);
}

vector<string> JSONObject::getElements(string Item) {
    return Arrays[Item];
}
string JSONObject::getElement(string Array, int Element) {
    return Arrays[Array][Element];
}

void JSONObject::printElements(string Item, bool indent) {
    cout << this->ElementsJSONstring(Item, indent) << endl;
}

map<string, string>::iterator JSONObject::begin() {
    return Items.begin();
}

map<string, string>::iterator JSONObject::end() {
    return Items.end();
}


bool JSONObject::Item_equals(string Item, string compare) {
    string str1;
    string str2;
    if (this->exists(Item)) {
        str1 = Items[Item];
        str2 = compare;
        transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
        transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
        return str1 == str2;
    }
    else {
        return false;
    }
}

bool JSONObject::Element_equals(string Array, int Element, string compare) {
    string str1;
    string str2;
    if (Element < Arrays[Array].size()) {
        str1 = Arrays[Array][Element];
        str2 = compare;
        transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
        transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
        return str1 == str2;
    }
    else {
        return false;
    }
}

void JSONObject::print(bool indent) {
    cout << this->JSONString(indent) << endl;
}

bool JSONObject::open(string directory) {
    bool succeed = true;
    string JSONstring;
    string line;
    ifstream readfile(directory);
    succeed = readfile.is_open();
    while (getline(readfile, line)) {
        JSONstring.append(line);
    }
    readfile.close();
    this->parse(JSONstring);
    file = directory;
    return succeed;
}

void JSONObject::write(bool indent) {
    ofstream writefile(file);
    if (writefile.is_open()) {
        writefile << this->JSONString(indent);
        writefile.close();
    }
}

void JSONObject::write(string diretory, bool indent) {
    ofstream writefile(diretory);
    if (writefile.is_open()) {
        writefile << this->JSONString(indent);
        writefile.close();
    }
}
