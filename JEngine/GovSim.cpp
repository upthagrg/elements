///*
//File: GovSim.cpp
//Author: Glenn Upthagrove
//Last Updated: 07/01/2024
//Description: GovSim game file
//*/
//
//#include "Xeon.hpp" //Uses the Hydrogen architecture as a base
//string GetRandomName(vector<string>*);
//enum Sex {
//    Male = 1,
//    Female = 2
//};
//enum EducationLevel {
//    None = 0,
//    Elementary = 1,
//    MiddleSchool = 2,
//    HighSchool = 3,
//    College = 4,
//    Masters = 5,
//    PHD = 6
//};
//
//
//class Citizen {
//private:
//    int ID;
//    int FatherID;
//    int MotherID;
//    int SpouseID;
//    vector<int>ChildIDs;
//    bool Alive;
//    string FirstName;
//    string MiddleName;
//    string FamilyName;
//    int Sex;
//    int Age;
//    int Education;
//    int Income;
//    int Wealth;
//public:
//    Citizen();
//    Citizen(int, int);
//    ~Citizen();
//    void SetFirstName(string);
//    void SetMiddleName(string);
//    void SetFamilyName(string);
//    int GetSex();
//    void Print();
//};
//Citizen::Citizen() {
//    int rnum;
//    ID = MyBase.GetNextID("CITIZEN");
//    FatherID = -1;
//    MotherID = -1;
//    SpouseID = -1;
//    Alive = true;
//    FirstName = "";
//    MiddleName = "";
//    FamilyName = "";
//    rnum = MyBase.GenRandomInt(1,2);
//    if (rnum == 1) {
//        Sex = Female;
//    }
//    else {
//        Sex = Male;
//    }
//    Age = MyBase.GenRandomInt(0, 65);
//    if (Age < 12) {
//        Education = None;
//    }
//    else if (Age < 15) {
//        Education = Elementary;
//    }
//    else if (Age < 18) {
//        Education = MiddleSchool;
//    }
//    else if (Age < 22) {
//        Education = HighSchool;
//    }
//    else { //Age over 22 33% chance of being college educated
//        rnum = MyBase.GenRandomInt(1, 3);
//        if (rnum == 1) {
//            Education = College;
//        }
//        else{
//            Education = HighSchool;
//        }
//        //If College educated and over 23, 10% chance of Master's
//        if (Education == College && Age > 23) {
//            rnum = MyBase.GenRandomInt(1, 10);
//            if (rnum == 1) {
//                Education = Masters;
//            }
//        }
//        //If Has a masters and over 27, 10% of PHD
//        if (Education == Masters && Age > 27) {
//            rnum = MyBase.GenRandomInt(1, 10);
//            if (rnum == 1) {
//                Education = PHD;
//            }
//        }
//    }
//    if (Age > 17) {
//        if (Education == PHD) {
//            Income = MyBase.GenRandomInt(120000, 180000);
//        }
//        else if (Education == Masters) {
//            Income = MyBase.GenRandomInt(100000, 250000);
//        }
//        else if (Education == College) {
//            Income = MyBase.GenRandomInt(60000, 200000);
//        }
//        else {
//            Income = MyBase.GenRandomInt(30000, 80000);
//        }
//        Wealth = (Income * 0.2) * (Age - 18);
//    }
//}
//Citizen::Citizen(int FatherID, int MotherID) {
//
//}
//Citizen::~Citizen() {
//
//}
//void Citizen::SetFirstName(string Name) {
//    FirstName = Name;
//}
//void Citizen::SetMiddleName(string Name) {
//    MiddleName = Name;
//}
//void Citizen::SetFamilyName(string Name) {
//    FamilyName = Name;
//}
//int Citizen::GetSex() {
//    return Sex;
//}
//void Citizen::Print() {
//    string Message;
//
//    MyBase.Display("----------");
//    MyBase.Display(FirstName + " " + MiddleName + " " + FamilyName);
//    Message.append("ID: ");
//    Message.append(to_string(ID));
//    MyBase.Display(Message);
//    Message = "";
//    if (Alive) {
//        MyBase.Display("Alive: True");
//    }
//    else {
//        MyBase.Display("Alive: False");
//    }
//    if (Sex == Female) {
//        MyBase.Display("Sex: Female");
//    }
//    else {
//        MyBase.Display("Sex: Male");
//    }
//    Message.append("Age: ");
//    Message.append(to_string(Age));
//    MyBase.Display(Message);
//    Message = "";
// 
//    if (Education == None) {
//        MyBase.Display("Education: None");
//    }
//    else if (Education == Elementary) {
//        MyBase.Display("Education: Elementary");
//    }
//    else if (Education == MiddleSchool) {
//        MyBase.Display("Education: Middle School");
//    }
//    else if (Education == HighSchool) {
//        MyBase.Display("Education: High School");
//    }
//    else if (Education == College) {
//        MyBase.Display("Education: College");
//    }
//    else if (Education == Masters) {
//        MyBase.Display("Education: Master's");
//    }
//    else {
//        MyBase.Display("Education: PHD");
//    }
//
//    Message.append("Income: ");
//    Message.append(to_string(Income));
//    MyBase.Display(Message);
//    Message = "";
//
//    Message.append("Wealth: ");
//    Message.append(to_string(Wealth));
//    MyBase.Display(Message);
//    Message = "";
//}
//
//
//string GamePath = "C:\\GovSim\\";
////std::unordered_map<int, Citizen*> Citizens;
//int main() {
//    HFile MaleFirstNamesFile(GamePath + "MaleFirstNames.txt");
//    HFile MaleMiddleNamesFile(GamePath + "MaleMiddleNames.txt");
//    HFile FemaleFirstNamesFile(GamePath + "FemaleFirstNames.txt");
//    HFile FemaleMiddleNamesFile(GamePath + "FemaleMiddleNames.txt");
//    HFile FamilyNamesFile(GamePath + "LastNames.txt");
//    vector<string> Filter;
//    vector<string> MaleFirstNames = TokenizeString(MaleFirstNamesFile.DataString(), "|", Filter);
//    vector<string> MaleMiddleNames = TokenizeString(MaleMiddleNamesFile.DataString(), "|", Filter);
//    vector<string> FemaleFirstNames = TokenizeString(FemaleFirstNamesFile.DataString(), "|", Filter);
//    vector<string> FemaleMiddleNames = TokenizeString(FemaleMiddleNamesFile.DataString(), "|", Filter);
//    vector<string> FamilyNames = TokenizeString(FamilyNamesFile.DataString(), "|", Filter);
//
//    Citizen* C1;
//
//    for (int i = 0; i < 100; i++) {
//        C1 = new Citizen();
//        string name;
//        if (C1->GetSex() == Male) {
//            name = GetRandomName(&MaleFirstNames);
//            C1->SetFirstName(name);
//            name = GetRandomName(&MaleMiddleNames);
//            C1->SetMiddleName(name);
//        }
//        else {
//            name = GetRandomName(&FemaleFirstNames);
//            C1->SetFirstName(name);
//            name = GetRandomName(&FemaleMiddleNames);
//            C1->SetMiddleName(name);
//        }
//        C1->SetFamilyName(GetRandomName(&FamilyNames));
//        C1->Print();
//        Sleep(5000);
//    }
//    cout << "Done" << endl;
//
//    return 0;
//}
//
//string GetRandomName(vector<string>* Names) {
//    int rnum;
//    rnum = MyBase.GenRandomInt(0, Names->size()-1);
//    return (*Names)[rnum];
//}