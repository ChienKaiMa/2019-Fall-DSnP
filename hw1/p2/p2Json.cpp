/****************************************************************************
  FileName     [ p2Json.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Json and JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include "p2Json.h"

using namespace std;
 
// Implement member functions of class Row and Table here


bool Json::read(const string& jsonFile)
{
   ifstream file(jsonFile);
   string line, key, value;
   while(!getline(file, line).eof()){
      getline(file, key, ':');
      for (size_t i=0;i<key.size();i++) {
         if (key[i] == '\"') {
            key = key.substr(i+1, key.size()-i-3);
            break;
         }
      }
      if (getline(file, value, ',')) {
         (this ->_obj).push_back(JsonElem(key, stoi(value)));
      }      
   }
   return true;
}

bool Json::isEmpty() {
   if (_obj.size() == 0) {
      return true;
   }
   else {
      return false;
   }
}

int Json::sum() {
   int num = 0;
   for (size_t i=0;i<_obj.size();i++) {
      num += *_obj[i].getValue();
   }
   return num;
}

double Json::ave() {
   return double(this -> sum()) / _obj.size();
}

JsonElem& Json::maxElem() {
   int* max = _obj[0].getValue();
   int maxIndex = 0;
   for (size_t i=0;i<_obj.size();i++) {
      int* value = _obj[i].getValue();
      if (*value > *max) {
         max = value;
         maxIndex = i;
      }
   }
   return _obj[maxIndex];
}

JsonElem& Json::minElem() {
   int* min = _obj[0].getValue();
   int minIndex = 0;
   for (size_t i=0;i<_obj.size();i++) {
      int* value = _obj[i].getValue();
      if (*value < *min) {
         min = value;
         minIndex = i;
      }
   }
   return _obj[minIndex];
}

void Json::add() {
   string key;
   int value;
   if (cin >> key >> value) {
      (this -> _obj).push_back(JsonElem(key, value));
   }
   else {
      cout << "Invalid key-value pair." << endl;
   }
}

void Json::print() {
   cout << '{' << endl;
   for (size_t i = 0; i < (this ->_obj).size(); i++)
   {
      if (i == (this ->_obj).size() - 1) {
         cout << "  " << (this ->_obj)[i] << endl;
      }
      else {
         cout << "  " << (this ->_obj)[i] << ',' << endl;
      }
   }
   cout << "}" << endl;
}

ostream&
operator << (ostream& os, const JsonElem& j)
{
   return (os << "\"" << j._key << "\" : " << j._value);
}

