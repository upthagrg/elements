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
#include "JSON.hpp"


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

    cout << "string" << endl;

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