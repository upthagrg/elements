///*
//File: Elements.cpp
//Author: Glenn Upthagrove
//Last Updated: 02/18/2025
//Description: This file is the testing environment for the Elements application framework and the initial applications built off of it.
//*/

#include "Xeon.hpp"

int main() {
   /// Xeon::XeonSetupWizard();
    string testJSON = "{\"name\":\"John\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"Adam\" \"Ob2\":{\"n3\":\"bill\"}},\"Sex\":M}";
    //string testJSON2 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"obj\":{\"name2\":\"Adam\" \"Ob2\":{\"n3\":\"bill\"}}}";
    //string testJSON3 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"PetNameArray\":[\"Jax\",\"Dakota\",\"Nova\"],\"Sex\":M}";
    //string testJSON4 = "{\"name\":\"John\", \"age\":30, \"car\":null, \"PetNameArray\":[\"Jax\",\"Dakota\",\"Nova\"]}";
    //O2::JSONObject myobj;

    //myobj.Parse(testJASON);
    //MyBase.Display(testJASON);
    //MyBase.Display(myobj.Stringify());
 /*   HFile file1("C:\\ServerFiles\\TEST.txt");
    MyBase.Display("File 1 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file1.DataString());

    HFile file2 = file1;
    MyBase.Display("File 2 contents");
    MyBase.Display(file1.Data());
    MyBase.Display(file2.DataString());*/

    O2::JSONFile File("C:\\ServerFiles\\TEST.txt");
    MyBase.Display("JSON FILE:");
    MyBase.Display(File.GetJSON().Stringify());
    File.SetJSON(testJSON);
    MyBase.Display("JSON FILE:");
    MyBase.Display(File.GetJSON().Stringify());
    File.Write();
    return 0;
}

