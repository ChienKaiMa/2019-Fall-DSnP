/****************************************************************************
  FileName     [ test.cpp ]
  PackageName  [ test ]
  Synopsis     [ Test program for simple database db ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "dbJson.h"

using namespace std;

extern DBJson dbjson;

class CmdParser;
CmdParser* cmdMgr = 0; // for linking purpose

int
main(int argc, char** argv)
{
   if (argc != 2) {  // testdb <jsonfile>
      cerr << "Error: using testdb <jsonfile>!!" << endl;
      exit(-1);
   }

   ifstream inf(argv[1]);

   if (!inf) {
      cerr << "Error: cannot open file \"" << argv[1] << "\"!!\n";
      exit(-1);
   }

   if (dbjson) {
      cout << "Table is resetting..." << endl;
      dbjson.reset();
   }
   if (!(inf >> dbjson)) {
      cerr << "Error in reading JSON file!!" << endl;
      exit(-1);
   }

   cout << "========================" << endl;
   cout << " Print JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson << endl;

   // TODO
   // Insert what you want to test here by calling DBJson's member functions

   cout << "========================" << endl;
   cout << " Sum of JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson.sum() << endl;

   cout << "========================" << endl;
   cout << " Ave of JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson.ave() << endl;

   cout << "========================" << endl;
   cout << " Max of JSON object" << endl;
   cout << "========================" << endl;
   size_t maxI;
   cout << dbjson.max(maxI) << endl;

   cout << "========================" << endl;
   cout << " Min of JSON object" << endl;
   cout << "========================" << endl;
   size_t minI;
   cout << dbjson.min(minI) << endl;

   cout << "========================" << endl;
   cout << " Append JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson.add(DBJsonElem("Ric", 100)) << endl;

   cout << "========================" << endl;
   cout << " SortKey JSON object" << endl;
   cout << "========================" << endl;
   const DBSortKey kea;
   dbjson.sort(kea);
   cout << dbjson << endl;

   cout << "========================" << endl;
   cout << " SortValue JSON object" << endl;
   cout << "========================" << endl;
   const DBSortValue vala;
   dbjson.sort(vala);
   cout << dbjson << endl;

   cout << "========================" << endl;
   cout << " Reset JSON object" << endl;
   cout << "========================" << endl;
   dbjson.reset();
   cout << dbjson << endl; 

   return 0;
}
