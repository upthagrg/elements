#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
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

void engine_test();
bool str_equals(string, string);
bool str_equals(char*, char*);
bool str_equals(string, char*);
bool str_equals(char*, string);


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