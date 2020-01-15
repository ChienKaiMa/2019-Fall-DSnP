/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_set>

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
struct Strash {
  size_t operator () (const AigGate* g) const {
    size_t num1 = (size_t)(g->_fanin1.gate()) + g->_fanin1.isInv();
    size_t num2 = (size_t)(g->_fanin2.gate()) + g->_fanin2.isInv();
    if (num1 > num2) {
      return (num1 << 32) + num2;
    } else {
      return (num2 << 32) + num1;
    }
  }
};

struct KeyEqual {
  bool operator () (const AigGate* g1, const AigGate* g2) const {
    Strash s;
    return s(g1) == s(g2);
  }
};

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  unordered_set<AigGate*, Strash, KeyEqual> myHash;
  vector<CirGate*> toDel;
  for (size_t i=0; i<_dfsList.size(); ++i) {
    if (_dfsList[i]->isAig()) {
      // average cost of emplace: constant
      auto myPair = myHash.emplace((AigGate*)(_dfsList[i]));
      if (!myPair.second) {
        ((AigGate*)_dfsList[i])->_fanin1.gate()->deleteFanout(AigGateV(_dfsList[i], ((AigGate*)_dfsList[i])->_fanin1.isInv()));
        ((AigGate*)_dfsList[i])->_fanin2.gate()->deleteFanout(AigGateV(_dfsList[i], ((AigGate*)_dfsList[i])->_fanin2.isInv()));
        // TODO: Check if _fanin2 has no fanout
        cout << "Strashing: " << (*(myPair.first))->_gateId << " merging " << _dfsList[i]->_gateId << "..." << endl;
        (*(myPair.first))->replaceGate(AigGateV(_dfsList[i], 0));
        toDel.push_back(_dfsList[i]);
      }
    }
  }
  if (!toDel.empty()) {
    _dfsList.clear();
    genDFSList();
  }
  for (size_t i=0; i<toDel.size(); ++i) {
    deleteGate(toDel[i]);
  }
}

void
CirMgr::fraig()
{

}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
