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
#include <fstream>
#include <string>
#include <map>
#include <errno.h>
#include "garbage.hpp"

using std::string;
using std::map;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
using std::isblank;

class JSONObject {
private:
    map<string, string> Items;
    map<string, string>::iterator it_internal;
    map<string, string>::iterator it_internal_next;
    string file;
    bool _iswhitespace(char*);

public:
    map<string, string>::iterator iterator;
    JSONObject();
    JSONObject(string);
    JSONObject(JSONObject &obj);
    string JSONString(bool);
    void additem(string, string);
    void additem(string, bool);
    void additem(string, int);
    void additem(string, unsigned int);
    void additem(string, long);
    void additem(string, unsigned long);
    void additem(string, long long);
    void additem(string, unsigned long long);
    void additem(string, double);
    void additem(string, float);
    void additem(string, long double);
    void additem(string, char);
    void additem(string, char*);
    bool parse(string);
    string get(string);
    bool exists(string);
    void print(bool);
    bool open(string);
    void write(bool);
    void write(string, bool);
    map<string, string>::iterator begin();
    map<string, string>::iterator end();
};

JSONObject::JSONObject() {
    Items.clear();
    it_internal = Items.begin();
    it_internal_next = Items.begin();
}

JSONObject::JSONObject(string Input) {
    this->parse(Input);
    it_internal = Items.begin();
    it_internal_next = Items.begin();
    if (it_internal_next != Items.end()) {
        ++it_internal_next;
    }
}

JSONObject::JSONObject(JSONObject &Obj) {
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
    this->it_internal = Items.begin();
    this->it_internal_next = Items.begin();

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

        if (it_internal_next != Items.end()) {
            JSONString.append(",");
        }

        ++it_internal;

        if (it_internal_next != Items.end()) {
            ++it_internal_next;
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

void JSONObject::additem(string Item, bool Value) {
    if (Value) {
        Items[Item] = "1";
    }
    else {
        Items[Item] = "0";
    }
}

void JSONObject::additem(string Item, int Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, unsigned int Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, long Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, unsigned long Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, long long Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, unsigned long long Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, double Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, float Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, long  double Value) {
    Items[Item] = to_string(Value);
}

void JSONObject::additem(string Item, char Value) {
    string tmp = "";
    tmp += Value;
    Items[Item] = Value;
}

void JSONObject::additem(string Item, char* Value) {
    string tmp;
    tmp.assign(Value);
    Items[Item] = Value;
}

bool JSONObject::parse(string input) {
    const string delim= ",{}:\"\n";
    bool sucess = true;
    char *token;
    string stoken;
    char *next_token;
    string item = "";
    string value = "";
    char *buffer = new char[input.length() + 1];
    errno_t ret = strcpy_s(buffer, input.length() + 1, input.c_str());

    Items.clear();
    token = strtok_s(buffer, delim.c_str(), &next_token);
    while (token != NULL)
    {
        if (not (this->_iswhitespace(token))) {
            if (item.empty()) {
                item.assign(token);
            }
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

map<string, string>::iterator JSONObject::begin() {
    return Items.begin();
}

map<string, string>::iterator JSONObject::end() {
    return Items.end();
}

string JSONObject::get(string Item) {
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

int main()
{
    string test_string = "{\"Narrator Text\":\"Hello World\", \"Player Text\":\"Hello Narrator\"}";
    JSONObject test(test_string);
    test.print(true);


    //This is for testing
    JSONObject game;
    string directory;
    string input = "";
    string player_text = "";
    string narrator_text = "";
    string file = "";
    bool opened = false;

    cout << "Would you like to build a new game, or add to one?" << endl;
    cout << "1. Build new game." << endl;
    cout << "2. Update an existing game." << endl;
    cin >> input;

    if (input == "2") {
        cout << "Update functionality not yet supported by JEngine." << endl;
        //while (not opened) {
        //    cout << "File to open?" << endl;
        //    cin >> input;
        //    opened = game.open(input);
        //}
    }

    input = "";
    while (input != "1") {
        narrator_text = "";
        if (input == "2") {
            game.print(true);
            system("pause");
            system("cls");
        }
        else if (input == "3") {
            player_text = "";
            cout << "Enter player text" << endl;
            cin >> player_text;
            cout << "Enter first narrator response" << endl;
            cin >> narrator_text;
            game.additem(player_text, narrator_text);
            system("cls");
        }
        else if (input == "4") {
            cout << "Enter response" << endl;
            cin >> narrator_text;
            game.additem(player_text, narrator_text);
            system("cls");
        }
        else if (input == "5") {
            if (file.empty()) {
                cout << "Enter file location/name" << endl;
                cin >> file;
            }
            game.write(file, true);
            system("cls");
        }
        else if (input == "6") {
            cout << "Enter file location/name" << endl;
            cin >> file;
            game.write(file, true);
            system("cls");
        }

        cout << "What would you like to do?" << endl;
        cout << "1 - Exit" << endl;
        cout << "2 - Print current game" << endl;
        cout << "3 - Add player text" << endl;
        if (not player_text.empty()) {
            cout << "4 - Add another narrator response" << endl;
            cout << "5 - Save" << endl;
            cout << "6 - Save As" << endl;
        }
        cin >> input;
        system("cls");
    }
    game.print(true);
    return 0;
}