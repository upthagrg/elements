#pragma once
/*
File: JEngine.hpp
Author: Glenn Upthagrove
Last Updated: 01/20/2024
Description: This header file contains the JEngine text adventure game engine.
Everything you need is currently in the .hpp but this will be broken out into a .hpp, .cpp, and .o files later.
JEngine will allow you to build a a text based adventure game through its text based interface, 
or can be integrated into your own program for additional custom functionality. 
It will be able store and load the game definition, and individual game saves.
It will be able to build, edit, and play the games. 
It will be able to be used as developer and as player without the acess of the developer. 
This will be one of the first programs to use the Elements application framework.
This will heavily utilize JSON to hold its games. 
*/


#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <errno.h>
#include "garbage.hpp"
#include "JSON.hpp"
#include "Hydrogen.hpp"

//list of game state
//ability to raise a state flag
//state flag to state

using std::getline;

class JEngine {
private:
    JSONObject game_internals;
    JSONObject global_response;
    JSONObject responses;
    JSONObject state_flag_to_state;
    JSONObject player_text_to_flag;

public:
    JEngine();

    string get_response(string);

    void add_global_response(string, string);
    string _get_global_response(string);

    void map_state_flag_to_state(string, string);
    void raise_state_flag(string);
    string get_game_state();

    void map_player_text_to_flags(string, vector<string>);
    vector<string> get_flags_from_player_text(string);

    void raise_flag(string);

    void set_directory(string);
    string get_directory();
};

JEngine::JEngine() {
    game_internals.additem("DIRECTORY", "C:\JEngine"); //default game directory
    game_internals.additem("GAME_STATE", "NEW"); //default game state
    responses.additem("DEFAULT", "I'm sorry, I don't have an answer to that."); //Default answer to no found mapping
    string r = game_internals.getItem("GAME_STATE");
}

void JEngine::add_global_response(string player_text, string response) {
    global_response.additem(to_upper(player_text), response);
}

string JEngine::_get_global_response(string player_text) {
    string player_text_upper = to_upper(player_text);
    return global_response.getItem(player_text_upper);
}

void JEngine::map_state_flag_to_state(string state_flag, string state) {
    state_flag_to_state.additem(to_upper(state_flag), to_upper(state));
}

void JEngine::raise_state_flag(string state_flag) {
    game_internals.additem("GAME_STATE", state_flag_to_state.getItem(to_upper(state_flag)));
}

string JEngine::get_game_state() {
    return game_internals.getItem("GAME_STATE");
}

void JEngine::map_player_text_to_flags(string player_text, vector<string> flags) {
    for (int i = 0; i < flags.size(); i++) {
        //flag is a state flag
        if (flags[i][0] == 'S' || flags[i][0] == 's') {
            //hold the flags for this text as an array item
            player_text_to_flag.addelement(to_upper(player_text), to_upper(flags[i]));
        }
    }
}

vector<string> JEngine::get_flags_from_player_text(string player_text) {
    return player_text_to_flag.getElements(to_upper(player_text));
}


void JEngine::raise_flag(string flag) {
    //flag is a state flag
    if (flag[0] == 'S' || flag[0] == 's') {
        raise_state_flag(flag);
    }
}

string JEngine::get_response(string player_text) {
    string default_response = responses.getItem("DEFAULT"); //default response to unknown player input
    string global_response;
    vector<string> flags = this->get_flags_from_player_text(player_text);

    //raise flags
    for (int i = 0; i < flags.size(); i++) {
        this->raise_flag(flags[i]);
    }

    //global responses is highest priority
    global_response = this->_get_global_response(player_text);
    if (not global_response.empty()) {
        return global_response;
    }
    //No match found return default;
    return default_response;
}

void JEngine::set_directory(string directory_in) {
    game_internals.additem("DIRECTORY", directory_in);
}

string JEngine::get_directory() {
    return game_internals.getItem("DIRECTORY");
}

void test_engine1() {
    //string test_string = "{\"Narrator Text\":\"Hello World\", \"Player Text\":\"Hello Narrator\"}";
    //string test_string = "{\"Player Text\":\"I like cake\",\"Narrator Text\":[\"Hello\", \"Hi\"]}";
    //transform(test_string.begin(), test_string.end(), test_string.begin(), ::toupper);
    //JSONObject test(test_string);
    //test.print(true);
    //engine_test();

    string input;
    JEngine test;
    //test.add_global_response("who are you?", "The Narrator, of course.");
    //test.map_state_flag_to_state("S1", "Broken_Narrator");
    //vector<string> state_flags;
    //state_flags.push_back("S1");
    //test.map_player_text_to_flags("who are you?", state_flags);

    //cout << "Game state: " << test.get_game_state() << endl;
    //cout << "testing get response for Who are you?" << endl;
    //cout << test.get_response("Who are you?") << endl;
    //cout << "Game state: " << test.get_game_state() << endl;

    input = "";
    vector<string> flags;
    string player_text;
    string response;
    string flag;
    string state;
    while (input != "Exit") {
        if (input == "1") {
            cout << "Enter player text" << endl;
            cin >> player_text;
            cout << "Enter response" << endl;
            cin >> response;
            test.add_global_response(player_text, response);
            cout << "Would you like this player input to trigger a flag? Y/N" << endl;
            cin >> input;
            while (input != "Y" && input != "N") {
                cout << "Unrecognized command, please tyoe Y or N" << endl;
            }
            if (input == "Y") {
                cout << "Which flag type would you like to add?" << endl;
                cout << "S - state" << endl;
                cin >> input;
                while (input != "S") {
                    cout << "Unrecognized command, which flag would you like to add?" << endl;
                    cout << "S - state" << endl;
                    cin >> input;
                }
                if (input == "S") {
                    cout << "Enter flag" << endl;
                    cin >> flag;
                    flag = "S" + flag;
                    cout << "Enter new game state" << endl;
                    cin >> input;
                    test.map_state_flag_to_state(flag, input);
                    flags.push_back(flag);
                    test.map_player_text_to_flags(player_text, flags);
                    flags.clear();
                }
            }
        }
        else if (input == "2") {
            cout << "Game state " << test.get_game_state() << endl;
            cout << "Enter player text" << endl;
            cin >> player_text;
            cout << test.get_response(player_text) << endl;
            cout << "Game state " << test.get_game_state() << endl;
        }
        else {
            cout << "Unrecognized command" << endl;
        }

        cout << "What would you like to do?" << endl;
        cout << "Exit" << endl;
        cout << "1 - Add global response?" << endl;
        cout << "2 - Test" << endl;
        cin >> input;
    }
}





void test_engine2() {
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