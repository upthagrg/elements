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

int main()
{
    //string test_string = "{\"Narrator Text\":\"Hello World\", \"Player Text\":\"Hello Narrator\"}";
    //string test_string = "{\"Player Text\":\"I like cake\",\"Narrator Text\":[\"Hello\", \"Hi\"]}";
    //transform(test_string.begin(), test_string.end(), test_string.begin(), ::toupper);

    //JSONObject test(test_string);
    //test.print(true);

    engine_test();
    return 0;
}