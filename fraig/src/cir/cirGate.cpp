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

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

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
   cout << "= FECs: " << "TODO: printFECGroups" << endl;
   cout << "= Value: " << "TODO: printSIMValues" << endl;
   cout << "================================================================================" << endl;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
}

void
AigGate::printGate() const
{
   cout << "AIG " << _gateId << " ";
   if (_fanin[0].gate()->isUndef()) { cout << "*"; }
   if (_fanin[0].isInv()) { cout << "!"; }
   cout << (_fanin[0].gate())->getGateId() << " ";
   if (_fanin[1].gate()->isUndef()) { cout << "*"; }
   if (_fanin[1].isInv()) { cout << "!"; }
   cout << (_fanin[1].gate())->getGateId();
   if (_mySymbol.empty()) {
     cout << endl;
   } else {
     cout << " (" << _mySymbol << ")" << endl;
   }
}

