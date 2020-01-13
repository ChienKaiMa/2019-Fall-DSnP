/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern CirMgr* cirMgr;

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  // Determine which gates are to sweep
  IdList toSweep;
  for (size_t i=0; i<_unused.size(); ++i) {
    CirGate* myGate = getGate(_unused[i]);
    myGate->sweep(toSweep);
    if (myGate->isAig()) {
      _unused.erase(_unused.begin()+i);
      --i;
    }
  }
  ::sort(toSweep.begin(), toSweep.end());

  // Start sweeping
  for (size_t i=0; i<toSweep.size(); ++i) {
    CirGate* toDel = getGate(toSweep[i]);
    if (toDel->getTypeStr() == "PI") {
      _unused.push_back(toSweep[i]);
    } else {
      cout << "Sweeping: " << toDel->getTypeStr() << "(" << toDel->_gateId << ") removed..." << endl;
      deleteGate(toDel);
    }
  }
  ::sort(_unused.begin(), _unused.end());
  ::sort(_floatFanIns.begin(), _floatFanIns.end());
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  IdList toMerge;
  for (size_t i=0; i<_dfsList.size(); ++i) {
    if (_dfsList[i]->isAig()) {
      ((AigGate*)_dfsList[i])->optimize(toMerge);
    }
  }
  for (size_t i=0; i<toMerge.size(); ++i) {
    deleteGate(getGate(toMerge[i]));
  }
  _dfsList.clear();
  genDFSList();

  // Check for unused gates
   for (int i=0; i<_miloa[1]; ++i) {
      CirGate* myPi = getGate(_piList[i]);
      if (((PiGate*)myPi)->_fanout.empty()) {
        bool rep = false;
        for (size_t j=0; j<_unused.size(); ++j) {
          if (_unused[j] == _piList[i]) {
            rep = true;
            break;
          }
        }
        if (!rep) {
          _unused.push_back(_piList[i]);
        }
      }
   }
   for (size_t i=0; i<_aigList.size(); ++i) {
      CirGate* myAig = getGate(_aigList[i]);
      if (((AigGate*)myAig)->_fanout.empty()) {
        bool rep = false;
        for (size_t j=0; j<_unused.size(); ++j) {
          if (_unused[j] == _aigList[i]) {
            rep = true;
            break;
          }
        }
        if (!rep) {
          _unused.push_back(_aigList[i]);
        }
      }
   }
   ::sort(_floatFanIns.begin(), _floatFanIns.end());
   ::sort(_unused.begin(), _unused.end());
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::deleteGate(CirGate* g)
{
  // Update _floatingFanins, _aigList
  if (g->isAig()) {
    for (size_t j=0; j<_aigList.size(); ++j) {
      if (_aigList[j] == g->_gateId) {
        _aigList.erase(_aigList.begin()+j);
        break;
      }
    }
    // 4. Check if its fanins' fanouts become empty
    // 5. Check if it is in flt list
    for (size_t i=0; i<_floatFanIns.size(); ++i) {
      if (g->_gateId == _floatFanIns[i]) {
        _floatFanIns.erase(_floatFanIns.begin()+i);
        break;
      }
    }
  }
  // Remove this gate from _gateList
  _gateList[g->_gateId] = 0;
  delete g;
}

void
AigGate::optimize(IdList& toMerge) {
  // What about UNDEF gates?
  if (_fanin1.gate()->getTypeStr() == "CONST") {
    if (_fanin1.isInv()) {
      // Fanin has constant 1 -> replaced by the other fanin
      cerr << "Fanin has constant 1" << endl;
      cout << "Simplifying: " << _fanin2.gate()->_gateId << " merging ";
      if (_fanin2.isInv()) {
        cout << "!";
      }
      cout << _gateId << "..." << endl;
      // TODO not done yet!
      _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
      _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
      _fanin2.gate()->replaceGate(AigGateV((CirGate*)this, _fanin2.isInv()));
      toMerge.push_back(_gateId);
    } else {
      // Fanin has constant 0 -> replaced with 0
      // Replace with fanin1
      cerr << "Fanin has constant 0" << endl;
      cout << "Simplifying: " << _fanin1.gate()->_gateId << " merging " << _gateId << "..." << endl;
      _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
      _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
      // TODO: Check if _fanin2 has no fanout
      _fanin1.gate()->replaceGate(AigGateV((CirGate*)this, _fanin1.isInv()));
      toMerge.push_back(_gateId);
    }
  } else if (_fanin2.gate()->getTypeStr() == "CONST") {
    if (_fanin2.isInv()) {
      // Fanin has constant 1 -> replaced by the other fanin
      cerr << "Fanin has constant 1" << endl;
      cout << "Simplifying: " << _fanin1.gate()->_gateId << " merging ";
      if (_fanin1.isInv()) {
        cout << "!";
      }
      cout << _gateId << "..." << endl;

      _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
      _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
      _fanin1.gate()->replaceGate(AigGateV((CirGate*)this, _fanin1.isInv()));
      toMerge.push_back(_gateId);
    } else {
      // Fanin has constant 0 -> replaced with 0
      cerr << "Fanin has constant 0" << endl;
      cout << "Simplifying: " << _fanin2.gate()->_gateId << " merging " << _gateId << "..." << endl;
      _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
      _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
      // TODO: Check if _fanin1 has no fanout
      _fanin2.gate()->replaceGate(AigGateV((CirGate*)this, _fanin2.isInv()));
      toMerge.push_back(_gateId);
    }
  } else if (_fanin1 == !_fanin2) {
    // Inverted fanins -> replaced with 0
    cerr << "Inverted fanins" << endl;
    cout << "Simplifying: 0 merging " << _gateId << "..." << endl;
    _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
    _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
    cirMgr->getGate(0)->replaceGate(AigGateV((CirGate*)this, 0));
    // TODO: Check if _fanin1 and _fanin2 have no fanout
    toMerge.push_back(_gateId);
  } else if (_fanin1 == _fanin2){
    // Identical fanins -> replaced with the (fanin + phase)
    cerr << "Identical fanins" << endl;
    cout << "Simplifying: " << _fanin1.gate()->_gateId << " merging " << _gateId << "..." << endl;
    _fanin1.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
    _fanin2.gate()->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
    // TODO: Check if _fanin2 has no fanout
    _fanin1.gate()->replaceGate(AigGateV((CirGate*)this, _fanin1.isInv()));
    toMerge.push_back(_gateId);
  }
}

void
AigGate::replaceGate(AigGateV repl) {
  // connect g's fanouts to this
  AigGate* g = (AigGate*)(repl.gate());
  for (size_t j=0; j<g->_fanout.size(); ++j) {
    _fanout.push_back(g->_fanout[j]);
    CirGate* myGate = (g->_fanout[j]).gate();
    bool isInv = (g->_fanout[j]).isInv();
    if (repl.isInv()) {
      isInv = !isInv;
    }
    if (myGate->isAig()) {
      if (((AigGate*) myGate)->_fanin1.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin1 = AigGateV((CirGate*)this, isInv);
      }
      if (((AigGate*) myGate)->_fanin2.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin2 = AigGateV((CirGate*)this, isInv);
      }
    } else if (myGate->getTypeStr() == "PO") {
      if (((PoGate*) myGate)->_fanin.gate() == repl.gate()) {
        ((PoGate*) myGate)->_fanin = AigGateV((CirGate*)this, isInv);
      }
    }
  }
}

void
ConstGate::replaceGate(AigGateV repl) {
  // connect g's fanouts to this
  AigGate* g = (AigGate*)(repl.gate());
  for (size_t j=0; j<g->_fanout.size(); ++j) {
    _fanout.push_back(g->_fanout[j]);
    CirGate* myGate = (g->_fanout[j]).gate();
    bool isInv = (g->_fanout[j]).isInv();
    if (repl.isInv()) {
      isInv = !isInv;
    }
    if (myGate->isAig()) {
      if (((AigGate*) myGate)->_fanin1.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin1 = AigGateV((CirGate*)this, isInv);
      }
      if (((AigGate*) myGate)->_fanin2.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin2 = AigGateV((CirGate*)this, isInv);
      }
    } else if (myGate->getTypeStr() == "PO") {
      if (((PoGate*) myGate)->_fanin.gate() == repl.gate()) {
        ((PoGate*) myGate)->_fanin = AigGateV((CirGate*)this, isInv);
      }
    }
  }
}

void
PiGate::replaceGate(AigGateV repl) {
  // connect g's fanouts to this
  AigGate* g = (AigGate*)(repl.gate());
  for (size_t j=0; j<g->_fanout.size(); ++j) {
    _fanout.push_back(g->_fanout[j]);
    CirGate* myGate = (g->_fanout[j]).gate();
    bool isInv = (g->_fanout[j]).isInv();
    if (repl.isInv()) {
      isInv = !isInv;
    }
    if (myGate->isAig()) {
      if (((AigGate*) myGate)->_fanin1.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin1 = AigGateV((CirGate*)this, isInv);
      }
      if (((AigGate*) myGate)->_fanin2.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin2 = AigGateV((CirGate*)this, isInv);
      }
    } else if (myGate->getTypeStr() == "PO") {
      if (((PoGate*) myGate)->_fanin.gate() == repl.gate()) {
        ((PoGate*) myGate)->_fanin = AigGateV((CirGate*)this, isInv);
      }
    }
  }
}

void
UndefGate::replaceGate(AigGateV repl) {
  // connect g's fanouts to this
  AigGate* g = (AigGate*)(repl.gate());
  for (size_t j=0; j<g->_fanout.size(); ++j) {
    _fanout.push_back(g->_fanout[j]);
    CirGate* myGate = (g->_fanout[j]).gate();
    bool isInv = (g->_fanout[j]).isInv();
    if (repl.isInv()) {
      isInv = !isInv;
    }
    if (myGate->isAig()) {
      if (((AigGate*) myGate)->_fanin1.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin1 = AigGateV((CirGate*)this, isInv);
      }
      if (((AigGate*) myGate)->_fanin2.gate() == repl.gate()) {
        ((AigGate*) myGate)->_fanin2 = AigGateV((CirGate*)this, isInv);
      }
    } else if (myGate->getTypeStr() == "PO") {
      if (((PoGate*) myGate)->_fanin.gate() == repl.gate()) {
        ((PoGate*) myGate)->_fanin = AigGateV((CirGate*)this, isInv);
      }
    }
  }
}
/*
void
CirMgr::connectAig(const int& idx)
{
   CirGate* myAig = getGate(_aigList[idx]);
   size_t aigFaninId;
   CirGate* faninGate;
   bool isFloat = false;
   for (int j=0; j<2; ++j) {
      AigGateV aigFIn;
      if (j == 0) {
         aigFIn = ((AigGate*)myAig)->_fanin1;
      } else {
         aigFIn = ((AigGate*)myAig)->_fanin2;
      }
      aigFaninId = (size_t)(aigFIn.gate());
      aigFaninId /= 2;
      faninGate = getGate(aigFaninId);
      if (faninGate == 0) {
         CirGate* undef = new UndefGate(aigFaninId);
         faninGate = undef;
         _gateList[aigFaninId] = faninGate;
         // You don't want to push_back the floating gate twice
         if (!isFloat) {
            _floatFanIns.push_back(_aigList[idx]);
            isFloat = true;
         }
         if (j == 0) {
            ((AigGate*)myAig)->_fanin1 = AigGateV(faninGate, 0);
         } else {
            ((AigGate*)myAig)->_fanin2 = AigGateV(faninGate, 0);
         }
      } else {
         bool isInv = aigFIn.isInv();
         if (j == 0) {
            ((AigGate*)myAig)->_fanin1 = AigGateV(faninGate, isInv);
         } else {
            ((AigGate*)myAig)->_fanin2 = AigGateV(faninGate, isInv);
         }
      }
      string fInType = faninGate->getTypeStr();
      if (fInType == "UNDEF") {
         ((UndefGate*)faninGate)->_fanout.push_back(myAig);
      } else if (fInType == "PI") {
         ((PiGate*)faninGate)->_fanout.push_back(myAig);      
      } else if (fInType == "AIG") {
         ((AigGate*)faninGate)->_fanout.push_back(myAig);
      } else if (fInType == "CONST") {
         ((ConstGate*)faninGate)->_fanout.push_back(myAig);
      }
   }
}
*/