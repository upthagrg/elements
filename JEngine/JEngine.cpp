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
    void addelement(string, string);
    bool parse(string);
    string get(string);
    bool exists(string);
    vector<string> getelements(string);
    void printelements(string, bool);
    string elementsJSONstring(string, bool);
    void print(bool);
    bool open(string);
    void write(bool);
    void write(string, bool);
    map<string, string>::iterator begin();
    map<string, string>::iterator end();
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
        JSONString.append(elementsJSONstring(it_arr_internal->first, indent));
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

string JSONObject::elementsJSONstring(string Item, bool indent) {
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
            if (i < (Arrays[Item].size()-1)) {
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

//Parse a JASON string into this JSON object
bool JSONObject::parse(string input) {
    const string delim= ",{}:\"\n";
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
                        next_token = &next_token[i+1];
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

void JSONObject::addelement(string Item, string Element) {
    Arrays[Item].push_back(Element);
}

vector<string> JSONObject::getelements(string Item) {
    return Arrays[Item];
}

void JSONObject::printelements(string Item, bool indent) {
    cout << this->elementsJSONstring(Item, indent) << endl;
}

map<string, string>::iterator JSONObject::begin() {
    return Items.begin();
}

map<string, string>::iterator JSONObject::end() {
    return Items.end();
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


void engine_test();
bool str_equals(string, string);
bool str_equals(char*, char*);
bool str_equals(string, char*);
bool str_equals(char*, string);


int main()
{
    //string test_string = "{\"Narrator Text\":\"Hello World\", \"Player Text\":\"Hello Narrator\"}";
    string test_string = "{\"Player Text\":\"I like cake\",\"Narrator Text\":[\"Hello\", \"Hi\"]}";
    //transform(test_string.begin(), test_string.end(), test_string.begin(), ::toupper);

    JSONObject test(test_string);
    test.print(true);

    //test.addelement("Fruit", "apple");
    //test.addelement("Fruit", "bannana");
    //test.addelement("People", "Aijaeh");
    //test.addelement("People", "Glenn");

    //test.print(false);
    //cout << endl;
    //cout << endl;
    //cout << endl;
    //test.print(true);


    //engine_test();
    return 0;
}

void engine_test() {
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
}

bool str_equals(string str1, string str2) {
    transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
    return str1 == str2;
}

bool str_equals(char* str1, char* str2) {
    string s1, s2;
    s1.assign(str1);
    s2.assign(str2);
    return str_equals(s1, s2);
}
bool str_equals(string str1, char* str2) {
    string s2;
    s2.assign(str2);
    return str_equals(str1, s2);
}
bool str_equals(char* str1, string str2) {
    string s1;
    s1.assign(str1);
    return str_equals(s1, str2);
}