///*
//File: Elements.cpp
//Author: Glenn Upthagrove
//Last Updated: 12/10/2024
//Description: This file is the testing environment for the Elements application framework and the initial applications built off of it.
//*/

#include "Xeon.hpp"

int main() {
   /// Xeon::XeonSetupWizard();
    string testJASON = "{\"name\":\"John\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"Adam\" \"Ob2\":{\"n3\":\"bill\"}}\"Sex\":M}";//TODO:filter out the ending } character(s)
    O2::JSONObject myobj;
    myobj.Parse(testJASON);
    return 0;
}

