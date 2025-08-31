///*
//File: Elements.cpp
//Author: Glenn Upthagrove
//Last Updated: 02/18/2025
//Description: This file is the testing environment for the Elements application framework and the initial applications built off of it.
//*/

#include "Xeon.hpp"

int main() {
   /// Xeon::XeonSetupWizard();
    string testjason = "{\"name\":\"john\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"adam\" \"ob2\":{\"n3\":\"bill\"}},\"sex\":m}";
    string testjason2 = "{\"name\":\"john\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"adam\" \"ob2\":{\"n3\":\"bill\"}}}";
    string testjason3 = "{\"name\":\"john\", \"age\":30, \"car\":null, \"petnamearray\":[\"jax\",\"dakota\",\"nova\"],\"sex\":m}";
    string testjason4 = "{\"name\":\"john\", \"age\":30, \"car\":null, \"petnamearray\":[\"jax\",\"dakota\",\"nova\"]}";
    O2::JSONObject myobj;

    myobj.Parse(testjason);
    MyBase.Display(testjason);
    MyBase.Display(myobj.Stringify());
    MyBase.Display("\n");

    myobj.Parse(testjason2);
    MyBase.Display(testjason2);
    MyBase.Display(myobj.Stringify());
    MyBase.Display("\n");


    myobj.Parse(testjason3);
    MyBase.Display(testjason3);
    MyBase.Display(myobj.Stringify());
    MyBase.Display("\n");


    myobj.Parse(testjason4);
    MyBase.Display(testjason4);
    MyBase.Display(myobj.Stringify());
    MyBase.Display("\n");



    HFile file1("C:\\ServerFiles\\TEST.txt");
    MyBase.Display("File 1 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file1.DataString());
    MyBase.Display("\n");


    HFile file2 = file1;
    MyBase.Display("File 2 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file2.DataString());
    return 0;
}

