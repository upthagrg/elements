//#pragma once
///*
//File: WebDoc.hpp
//Author: Glenn Upthagrove
//Last Updated: 01/20/2024
//Description: This header file contains the WebDOc engine. This will eventually get integrated into the Elements framework. 
//This will allow the creation of dynamic HTML content. The content is built and updated in memory, not held as a file on disk.
//THis can be used in conjuntion with the Xeon web or local harness to create a GUI for an Elements application, that can be deliverd via any web browser.
//*/
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <algorithm>
//#include <map>
//#include <vector>
//#include <errno.h>
//#include <windows.h>
//#include "garbage.hpp"
//#include "JSON.hpp"
//#include "Hydrogen.hpp"
//#include "Oxygen.hpp"
//
//using std::string;
//using std::map;
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
//using std::getline;
//
//namespace WebDoc {
//    const string HTML5_DOC_START = "<!DOCTYPE html>\n<html lang = \"en\">\n<head>\n<meta charset = \"utf-8\">\n<meta name = \"viewport\" content = \"width=device-width, initial-scale=1\">\n";
//    const string Title_Tag_Open = "<title>";
//    const string Title_Tag_Close = "</title>";
//    const string Style_Tag_Open = "<style>";    
//    const string Style_Tag_Close = "</style>";
//    const string HTML5_START_BODY = "</head>\n<body>\n";
//    string HTML5_DOC_END = "\n</body>\n</html>";
//
//    struct field {
//        string fieldname;
//        string value;
//    };
//
//    class WebDoc {
//    private:
//        string page;
//        string title;
//        vector<string> style;
//        vector<field> fields;
//    public:
//        WebDoc();
//        WebDoc(string);
//        string Get_Page();
//        void Add_Title(string);
//        void Add_Style(string);
//        void Add_Button(string, string, string, string);
//        void Add_Content_Raw(string, string);
//    };
//    WebDoc::WebDoc() {
//        title = "<title>New Page</title>";
//    }
//    WebDoc::WebDoc(string title_par) {
//        title += Title_Tag_Open;
//        title += title_par;
//        title += Title_Tag_Close;
//    }
//    string WebDoc::Get_Page() {
//        string page = "";
//        page += HTML5_DOC_START;
//        page += title;
//        for (int i = 0; i < style.size(); i++) {
//            page += style[i];
//        }
//        page += HTML5_START_BODY;
//        for (int i = 0; i < fields.size(); i++) {
//            page += fields[i].value;
//            page += "\n";
//        }
//        page += HTML5_DOC_END;
//        return page;
//    }
//    void WebDoc::Add_Title(string title_par) {
//        title = "";
//        title += Title_Tag_Open;
//        title += title_par;
//        title += Title_Tag_Close;
//    }
//    void WebDoc::Add_Style(string style_par) {
//        string newstyle = "";
//        newstyle += Style_Tag_Open;
//        newstyle += style_par;
//        newstyle += Style_Tag_Close;
//        style.push_back(newstyle);
//    }
//    void WebDoc::Add_Button(string Field_par, string Class_par, string Action_par, string Label_par) {
//        field button;
//        if (Field_par.length() > 0 && Class_par.length() && Label_par.length()) {
//            button.fieldname = Field_par;
//            button.value += "<Button ";
//            button.value += Action_par;
//            button.value += " ";
//            button.value += "class=\"";
//            button.value += Class_par;
//            button.value += "\" ";
//            button.value += "id=\"";
//            button.value += Field_par;
//            button.value += "\">";
//            button.value += Label_par;
//            button.value += "</button>";
//            fields.push_back(button);
//        }
//    }
//    void WebDoc::Add_Content_Raw(string content_identifier, string raw_content) {
//        field raw;
//        if (content_identifier.length() > 0 && raw_content.length() > 0) {
//            raw.fieldname = content_identifier;
//            raw.value = raw_content;
//            fields.push_back(raw);
//        }
//        else {
//            cout << "failed to add raw content" << endl;
//        }
//    }
//
//    string home() {
//        WebDoc Home("HomePage");
//        Home.Add_Style(".button{border: none;\ncolor: white;\npadding: 15px 32px;\ntext - align: center;\ntext - decoration: none;\ndisplay: inline - block;\nfont - size: 16px;\nmargin: 4px 2px;\ncursor: pointer;}\n.button1 {background-color: #04AA6D;}");
//        Home.Add_Content_Raw("Raw1", "<h1>Home Page</h1>");
//        Home.Add_Button("MyButton", "button1 button1", "onClick=\"window.location.reload();\"", "Refresh");
//        //action=\"\" 
//        string Form = "<form action=\"/Hello.html\" method=\"post\"><label for = \"fname\" > First name : </label><br><input type = \"text\" id = \"fname\" name = \"fname\" value = \"John\"><br><label for = \"lname\">Last name : </label><br><input type = \"text\" id = \"lname\" name = \"lname\" value = \"Doe\"><br><br><input type = \"submit\" value = \"Submit\"></form>";
//        Home.Add_Content_Raw("NameForm", Form);
//        return Home.Get_Page();
//    }
//}