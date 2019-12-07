/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include <algorithm>

using namespace std;

extern CirMgr *cirMgr;
static vector<int> walkList;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   // Need Modification
   int width = 39;
   width -= getTypeStr().length();
   width -= _symbol.length();
   width -= (static_cast<int>( log10(_gateId) )+1);
   width -= (static_cast<int>( log10(_lineNo) )+1);
   cout << "==================================================" << endl;
   cout << "= " << getTypeStr() << "(" << _gateId << ")";
   if (_symbol.empty()) {
      cout << ", line " << _lineNo << setw(width) << "=" << endl;
   } else {
      width -= 2;
      cout << "\"" << _symbol << "\", line " << _lineNo
      << setw(width) << "=" << endl;
   }
   cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   string myBlank = "";
   walkList.clear();
   printFanin(level, myBlank);
}

void
CirGate::printFanin(int level, string myBlank) const
{
   cout << getTypeStr() << " " << _gateId;
   if (getTypeStr() == "AIG") {
      if (find(walkList.begin(), walkList.end(), _gateId) != walkList.end()) {
         cout << " (*)" << endl;
         return;
      } else {
         walkList.push_back(_gateId);
      }
   }
   cout << endl;
   if (level != 0) {
      myBlank += "  ";
      if (getTypeStr() == "PO") {
         cout << myBlank;
         if (((OutputGate*)this)->_faninNum[0] % 2 != 0) {
            cout << "!";
         }
         _fanin[0]->printFanin(level-1, myBlank);
      } else if (getTypeStr() == "AIG") {
         for (size_t i=0; i<_fanin.size(); ++i) {
            cout << myBlank;
            if (((AndGate*)this)->_faninNum[i] % 2 != 0) {
               cout << "!";
            }
            _fanin[i]->printFanin(level-1, myBlank);
         }
      }
   }
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   string myBlank = "";
   walkList.clear();
   printFanout(level, myBlank);
}

void
CirGate::printFanout(int level, string myBlank) const
{
   assert(level >= 0);
   cout << getTypeStr() << " " << _gateId;
   if (getTypeStr() == "AIG") {
      if (find(walkList.begin(), walkList.end(), _gateId) != walkList.end()) {
         cout << " (*)" << endl;
         return;
      } else {
         if (level != 0) {
            walkList.push_back(_gateId);
         }
      }
   }
   cout << endl;
   if (level != 0) {
      myBlank += "  ";
      if (getTypeStr() == "AIG" || getTypeStr() == "PI" || getTypeStr() == "CONST") {
         for (int i=_fanoutNum.size()-1; i>=0; --i) {
            cout << myBlank;
            if (_fanoutNum[i] % 2 != 0) {
               cout << "!";
            }
            _fanout[i]->printFanout(level-1, myBlank);
         }
      }
   }
}

