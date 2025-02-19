///*
//File: Elements.cpp
//Author: Glenn Upthagrove
//Last Updated: 02/18/2025
//Description: This file is the testing environment for the Elements application framework and the initial applications built off of it.
//*/

#include "Xeon.hpp"

int main() {
   /// Xeon::XeonSetupWizard();
    //string testJASON = "{\"name\":\"John\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"Adam\" \"Ob2\":{\"n3\":\"bill\"}},\"Sex\":M}";
    //string testJASON2 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"Adam\" \"Ob2\":{\"n3\":\"bill\"}}}";
    //string testJASON3 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"PetNameArray\":[\"Jax\",\"Dakota\",\"Nova\"],\"Sex\":M}";
    //string testJASON4 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"PetNameArray\":[\"Jax\",\"Dakota\",\"Nova\"]}";
    //O2::JSONObject myobj;

    //myobj.Parse(testJASON);
    //MyBase.Display(testJASON);
    //MyBase.Display(myobj.Stringify());
    HFile file1("C:\\ServerFiles\\TEST.txt");
    MyBase.Display("File 1 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file1.DataString());

    HFile file2 = file1;
    MyBase.Display("File 2 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file2.DataString());
    return 0;
}

