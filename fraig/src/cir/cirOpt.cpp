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
  for (size_t i=0; i<_unused.size(); ++i) {
    CirGate* myGate = getGate(_unused[i]);
    if (myGate->isAig()) {
      cout << "Sweeping: AIG(" << myGate->_gateId << ") removed..." << endl;
      sweepGate(myGate);
      _unused.erase(_unused.begin()+i);
      for (size_t i=0; i<_aigList.size(); ++i) {
        if (_aigList[i] == myGate->_gateId) {
          _aigList.erase(_aigList.begin()+i);
          break;
        }
      }
      --i;
    }
  }
  ::sort(_unused.begin(), _unused.end());
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  for (size_t i=0; i<_dfsList.size(); ++i) {
    if (_dfsList[i]->isAig()) {
      AigGateV& f1 = ((AigGate*)_dfsList[i])->_fanin1;
      AigGateV& f2 = ((AigGate*)_dfsList[i])->_fanin2;
      // What about UNDEF gates?
      if (f1.gate()->getTypeStr() == "CONST") {
        if (f1.isInv()) {
          // Fanin has constant 1 -> replaced by the other fanin
          // TODO not done yet!
          if (f2.gate()->isAig()) {
            AigGate* g2 = f2.gate();
            for (size_t i=0; i<g2->_fanout.size(); ++i) {
              if (_dfsList[i] == g2->_fanout[i]) {
                g2->_fanout[i] = g2->_fanout[g2->_fanout.size()-1];
                g2->_fanout.pop_back();
              }
            }
            for (size_t i=0; i<f2.gate()->_fanout.size(); ++i) {
              for (size_t j=0; j<g2->_fanout.size(); ++j) {
                if (g2->_fanout[j] == f2.gate()->_fanout[i]) {
                  break;
                }
              }
              g2->_fanout.push_back(f2.gate()->_fanout[i]);
            }
          } else if (f2.gate()->isUndef()) {
            UndefGate* g2 = (UndefGate*)f2.gate();
            for (size_t i=0; i<g2->_fanout.size(); ++i) {
              if (_dfsList[i] == g2->_fanout[i]) {
                g2->_fanout[i] = g2->_fanout[g2->_fanout.size()-1];
                g2->_fanout.pop_back();
              }
            }
            for (size_t i=0; i<f2.gate()->_fanout.size(); ++i) {
              g2->_fanout.push_back(f2.gate()->_fanout[i]);
            }
            if (g2->_fanout.empty()) {
              // TODO
              // delete g2
            }
          } else {
            
          }
        } else {
          // Fanin has constant 0 -> replaced with 0
        }
      } else if (f2.gate()->getTypeStr() == "CONST") {
        if (f2.isInv()) {
          // Fanin has constant 1 -> replaced by the other fanin
        } else {
          // Fanin has constant 0 -> replaced with 0
        }
      } else if (f1.gate() == f2.gate()) {
        if (f1.isInv() != f2.isInv()) {
          // Inverted fanins -> replaced with 0
        } else {
          // Identical fanins -> replaced with the (fanin + phase)
        }
      }
    } else if (_dfsList[i]->getTypeStr() == "PO") {
      // Has anything to do?
    }
  }
  genDFSList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::sweepGate(CirGate* g)
{
  if (g->getTypeStr() == "AIG") {
    // 1. store its fanins
    CirGate* f1 = ((AigGate*)g)->_fanin1.gate();
    CirGate* f2 = ((AigGate*)g)->_fanin2.gate();
    // 2. Remove this gate from its fanins' fanouts
    if (f1->getTypeStr() == "AIG") {
      for (size_t i=0; i<((AigGate*)f1)->_fanout.size(); ++i) {
         if (g == ((AigGate*)f1)->_fanout[i]) {
            ((AigGate*)f1)->_fanout[i] = ((AigGate*)f1)->_fanout[((AigGate*)f1)->_fanout.size()-1];
            ((AigGate*)f1)->_fanout.pop_back();
         }
      }
      if (((AigGate*)f1)->_fanout.empty()) {
        cout << "Sweeping: AIG(" << f1->_gateId << ") removed..." << endl;
        sweepGate(f1);
        for (size_t i=0; i<_aigList.size(); ++i) {
          if (_aigList[i] == f1->_gateId) {
            _aigList.erase(_aigList.begin()+i);
            break;
          }
        }
      }
    } else if (f1->getTypeStr() == "PI") {
      for (size_t i=0; i<((PiGate*)f1)->_fanout.size(); ++i) {
         if (g == ((PiGate*)f1)->_fanout[i]) {
            ((PiGate*)f1)->_fanout[i] = ((PiGate*)f1)->_fanout[((PiGate*)f1)->_fanout.size()-1];
            ((PiGate*)f1)->_fanout.pop_back();
         }
      }
      if (((PiGate*)f1)->_fanout.empty()) {
        _unused.push_back(f1->_gateId);
      }
    } else if (f1->getTypeStr() == "CONST") {
      for (size_t i=0; i<((ConstGate*)f1)->_fanout.size(); ++i) {
         if (g == ((ConstGate*)f1)->_fanout[i]) {
            ((ConstGate*)f1)->_fanout[i] = ((ConstGate*)f1)->_fanout[((ConstGate*)f1)->_fanout.size()-1];
            ((ConstGate*)f1)->_fanout.pop_back();
         }
      }
    } else if (f1->getTypeStr() == "UNDEF") {
      for (size_t i=0; i<((UndefGate*)f1)->_fanout.size(); ++i) {
         if (g == ((UndefGate*)f1)->_fanout[i]) {
            ((UndefGate*)f1)->_fanout[i] = ((UndefGate*)f1)->_fanout[((UndefGate*)f1)->_fanout.size()-1];
            ((UndefGate*)f1)->_fanout.pop_back();
         }
      }
      if (((UndefGate*)f1)->_fanout.empty()) {
        cout << "Sweeping: UNDEF(" << f1->_gateId << ") removed..." << endl;
        sweepGate(f1);
      }
    }
    if (f2->getTypeStr() == "AIG") {
      for (size_t i=0; i<((AigGate*)f2)->_fanout.size(); ++i) {
         if (g == ((AigGate*)f2)->_fanout[i]) {
            ((AigGate*)f2)->_fanout[i] = ((AigGate*)f2)->_fanout[((AigGate*)f2)->_fanout.size()-1];
            ((AigGate*)f2)->_fanout.pop_back();
         }
      }
      if (((AigGate*)f2)->_fanout.empty()) {
        cout << "Sweeping: AIG(" << f2->_gateId << ") removed..." << endl;
        sweepGate(f2);
        for (size_t i=0; i<_aigList.size(); ++i) {
          if (_aigList[i] == f2->_gateId) {
            _aigList.erase(_aigList.begin()+i);
            break;
          }
        }
      }
    } else if (f2->getTypeStr() == "PI") {
      for (size_t i=0; i<((PiGate*)f2)->_fanout.size(); ++i) {
         if (g == ((PiGate*)f2)->_fanout[i]) {
            ((PiGate*)f2)->_fanout[i] = ((PiGate*)f2)->_fanout[((PiGate*)f2)->_fanout.size()-1];
            ((PiGate*)f2)->_fanout.pop_back();
         }
      }
      if (((PiGate*)f2)->_fanout.empty()) {
        _unused.push_back(f2->_gateId);
      }
    } else if (f2->getTypeStr() == "CONST") {
      for (size_t i=0; i<((ConstGate*)f2)->_fanout.size(); ++i) {
         if (g == ((ConstGate*)f2)->_fanout[i]) {
            ((ConstGate*)f2)->_fanout[i] = ((ConstGate*)f2)->_fanout[((ConstGate*)f2)->_fanout.size()-1];
            ((ConstGate*)f2)->_fanout.pop_back();
         }
      }
    } else if (f2->getTypeStr() == "UNDEF") {
      for (size_t i=0; i<((UndefGate*)f2)->_fanout.size(); ++i) {
         if (g == ((UndefGate*)f2)->_fanout[i]) {
            ((UndefGate*)f2)->_fanout[i] = ((UndefGate*)f2)->_fanout[((UndefGate*)f2)->_fanout.size()-1];
            ((UndefGate*)f2)->_fanout.pop_back();
         }
      }
      if (((UndefGate*)f2)->_fanout.empty()) {
        cout << "Sweeping: UNDEF(" << f2->_gateId << ") removed..." << endl;
        sweepGate(f2);
      }
    }
    // 3. Remove this gate from _gateList
    _gateList[g->_gateId] = 0;
    // 4. Check if its fanins' fanouts become empty
    // 5. Check if it is in flt list
    for (size_t i=0; i<_floatFanIns.size(); ++i) {
      if (g->_gateId == _floatFanIns[i]) {
        _floatFanIns.erase(_floatFanIns.begin()+i);
        break;
      }
    }

  } else if (g->getTypeStr() == "UNDEF") {
    // Remove this gate from _gateList
    _gateList[g->_gateId] = 0;
  }
}