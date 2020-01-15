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
#include <bitset>

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
static vector<int> walkList;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   cout << "================================================================================" << endl;
   cout << "= " << getTypeStr() << "(" << _gateId << ")";
   if (_mySymbol.empty()) {
      cout << ", line " << _lineNo << endl;
   } else {
      cout << "\"" << _mySymbol << "\", line " << _lineNo
       << endl;
   }
   cout << "= FECs:";
   printFECGroups();
   cout << endl;
   cout << "= Value: ";
   printSimValue();
   cout << endl;
   cout << "================================================================================" << endl;
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
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   string myBlank = "";
   walkList.clear();
   printFanout(level, myBlank);
}


void
CirGate::printFanin(int level, string myBlank) const
{
   // print this gate
   cout << getTypeStr() << " " << _gateId;
   // check if go before
   if (isAig()) {
      if (find(walkList.begin(), walkList.end(), _gateId) != walkList.end()) {
         cout << " (*)" << endl;
         return;
      } else {
         walkList.push_back(_gateId);
      }
   }
   cout << endl;
   // finish printing
   // print its fanins
   if (level != 0) {
      myBlank += "  ";
      if (getTypeStr() == "PO") {
         cout << myBlank;
         if (((PoGate*)this)->_fanin.isInv()) {
            cout << "!";
         }
         ((PoGate*)this)->_fanin.gate()->printFanin(level-1, myBlank);
      } else if (getTypeStr() == "AIG") {
         cout << myBlank;
         if (((AigGate*)this)->_fanin1.isInv()) {
            cout << "!";
         }
         ((AigGate*)this)->_fanin1.gate()->printFanin(level-1, myBlank);
         cout << myBlank;
         if (((AigGate*)this)->_fanin2.isInv()) {
            cout << "!";
         }
         ((AigGate*)this)->_fanin2.gate()->printFanin(level-1, myBlank);
      }
   }
}

void
CirGate::printFanout(int level, string myBlank) const
{
   assert(level >= 0);
   cout << getTypeStr() << " " << _gateId;
   if (isAig()) {
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
      if (isAig()) {
         vector<AigGateV>& myFanout = ((AigGate*)this)->_fanout;
         for (size_t i=0; i<myFanout.size(); ++i) {
            cout << myBlank;
            if (myFanout[i].isInv()) {
               cout << "!";
            }
            (myFanout[i].gate())->printFanout(level-1, myBlank);
         }
      } else if (getTypeStr() == "PI") {
         vector<AigGateV>& myFanout = ((PiGate*)this)->_fanout;
         for (size_t i=0; i<myFanout.size(); ++i) {
            cout << myBlank;
            if (myFanout[i].isInv()) {
               cout << "!";
            }
            (myFanout[i].gate())->printFanout(level-1, myBlank);
         }
      } else if (getTypeStr() == "CONST") {
         vector<AigGateV>& myFanout = ((ConstGate*)this)->_fanout;
         for (size_t i=0; i<myFanout.size(); ++i) {
            cout << myBlank;
            if (myFanout[i].isInv()) {
               cout << "!";
            }
            (myFanout[i].gate())->printFanout(level-1, myBlank);
         }
      }
   }
}
