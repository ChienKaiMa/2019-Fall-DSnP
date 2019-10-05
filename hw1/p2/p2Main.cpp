/****************************************************************************
  FileName     [ p2Main.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define main() function ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include "p2Json.h"

using namespace std;

enum StringValue {DEFAULT, SUM, AVE, MAX, MIN, ADD, PRINT, EXIT};
static map<string, StringValue> mapStringValues;
static void executeCommand(string command, Json& json, string jsonFile);
static void mapCommand();

int main()
{
   Json json;

   // Read in the csv file. Do NOT change this part of code.
   string jsonFile;
   cout << "Please enter the file name: ";
   cin >> jsonFile;
   if (json.read(jsonFile))
      cout << "File \"" << jsonFile << "\" was read in successfully." << endl;
   else {
      cerr << "Failed to read in file \"" << jsonFile << "\"!" << endl;
      exit(-1); // jsonFile does not exist.
   }

   // TODO read and execute commands
   mapCommand();
   while (true) {
      string command;
      cout << "Enter command: ";
      cin >> command;
      executeCommand(command, json, jsonFile);
   }

}

void mapCommand() {
   mapStringValues["SUM"] = SUM;
   mapStringValues["AVE"] = AVE;
   mapStringValues["MAX"] = MAX;
   mapStringValues["MIN"] = MIN;
   mapStringValues["ADD"] = ADD;
   mapStringValues["PRINT"] = PRINT;
   mapStringValues["EXIT"] = EXIT;
}

void executeCommand(string command, Json& json, string jsonFile) {  
   switch (mapStringValues[command])
   {
   case 1:
      if (json.isEmpty()) {
         cout << "Error: No element found!!" << endl;
      }
      else {
         cout << "The summation of the values is: " << json.sum() << '.' << endl;
      }
      break;
   case 2:
      if (json.isEmpty()) {
         cout << "Error: No element found!!" << endl;
      }
      else {
         cout << fixed;
         cout << "The average of the values is: " << setprecision(1) << json.ave() << '.' << endl;
      }
      break;
   case 3:
      if (json.isEmpty()) {
         cout << "Error: No element found!!" << endl;
      }
      else {
         cout << "The maximum element is: ";
         json.maxElem().printElem();
         cout << '.' << endl;
      }
      break;
   case 4:
      if (json.isEmpty()) {
         cout << "Error: No element found!!" << endl;
      }
      else {
         cout << "The minimum element is: ";
         json.minElem().printElem();
         cout << '.' << endl;
      }      
      break;
   case 5:
      json.add();
      break;
   case 6:
      json.print();
      break;
   case 7:
      exit(1);
      break;
   default:
      cout << "Invalid command" << endl;
   }
}