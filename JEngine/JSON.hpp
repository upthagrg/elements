//#pragma once
//
///*
//File: JSON.hpp
//Author: Glenn Upthagrove
//Last Updated: 01/20/2024
//Description: This header file contains the C++ JSON library. 
//This will probably get absorbed into a more fundamental library of the Elements framework, probably Hydrogen.hpp.
//This library allows a C++ application to create, edit, store, and load JSON content. 
//*/
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <algorithm>
//#include <map>
//#include <vector>
//#include <errno.h>
//#include "garbage.hpp"
//#include "Hydrogen.hpp"
//
//
//using std::string;
//using std::map;
//using std::map;
//using std::unordered_map;
//using std::vector;
//using std::to_string;
//using std::ifstream;
//using std::ofstream;
//using std::fstream;
//using std::getline;
//using std::cout;
//using std::cin;
//using std::endl;
//using std::isblank;
//using std::transform;
//using namespace Hydrogen;
//
//class JSONObject {
//private:
//    const char ObjectStart = '{';
//    const char ObjectEnd = '}';
//    const char PropertyValueSeperator = ':';
//    const char Seperator = ',';
//    const char ArrayStart = '[';
//    const char ArrayEnd = ']';
//    const string NullValue = "null";
//    const string TokenDelims = ":,\"   \n";
//    const int ItemEnm = 1;
//    const int ArrayEnm = 2;
//    const int ObjectEnm = 3;
//
//
//    unordered_map<string, int> ItemType;
//
//    unordered_map<string, string> Items;
//    unordered_map<string, vector<string>> Arrays;
//    unordered_map<string, vector<JSONObject>> Objects;
//    //unordered_map<string, vector<JSONObject>> ArraysOfObjects;
//
//    //unordered_map<string, string>::iterator it_internal;
//    //unordered_map<string, string>::iterator it_internal_next;
//    //unordered_map<string, vector<string>>::iterator it_arr_internal;
//    //unordered_map<string, vector<string>>::iterator it_arr_internal_next;
//public:
//    JSONObject();
//    JSONObject(string);
//    JSONObject(JSONObject& obj);
//    JSONObject parse(string);
//};
//JSONObject::JSONObject() {
//    //default constructor
//}
//JSONObject::JSONObject(string input) {
//    //non-default constructor
//    vector<string> Tokens;
//    vector<string> Filter;
//    unordered_map<string, string>::iterator Item_Iterator;
//    unordered_map<string, string>::iterator Item_Compare_Iterator;
//    int Instance = 0;
//    bool LastInstanceFound = false;
//    string CompareToken = "";
//    if (input.length() > 0) {
//        if (input[0] == ObjectStart) {
//            //root is object
//            Tokens = TokenizeString(input, TokenDelims.c_str(), Filter);
//            for (int i = 0; i < Tokens.size() - 1; i += 2) {
//                //Look ahead one token to determine this element's type
//                if (Tokens[i + 1][0] == ObjectStart) {
//                    //This item is an object
//                    ItemType[Tokens[i]] = ObjectEnm;
//                    
//                }
//                else if (Tokens[i + 1][0] == ArrayStart) {
//                    //This item is an array
//                    ItemType[Tokens[i]] = ArrayEnm;
//
//
//                }
//                else {
//                    //This item is an item
//                    ItemType[Tokens[i]] = ItemEnm;
//                    Item_Iterator = Items.find(Tokens[i]);
//                    if (Item_Iterator == Items.end()) {
//                        //Item already in object. This is not technically valid but often happens
//                        //To handle we will append a number to the end
//                        //First we need to find the highest numbered instance of this key
//                        do {
//                            Instance++;
//                            CompareToken = Tokens[i];
//                            CompareToken.append("_");
//                            CompareToken.append(to_string(Instance));
//                            Item_Compare_Iterator = Items.find(CompareToken);
//                            if (Item_Compare_Iterator == Items.end()) {
//                                //This instace is not yet in items
//                                LastInstanceFound = true;
//                                Tokens[i] = CompareToken;
//                            }
//
//                        } while (!LastInstanceFound);
//                        //Reset
//                        LastInstanceFound = false;
//                        Instance = 0;
//                    }
//                    Items[Tokens[i]] = Tokens[i + 1];
//                }
//            }
//        }
//        else if (input[0] == ArrayStart) {
//            //root is array of elements or objects
//        }
//    }
//}
//
////Copy Constructor
//JSONObject::JSONObject(JSONObject& obj) {
//
//}
//JSONObject JSONObject::parse(string input) {
//    JSONObject data;
//    return data;
//}