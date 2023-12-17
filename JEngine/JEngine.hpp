#pragma once
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
#include "MyLib.hpp"

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

void test_engine() {
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