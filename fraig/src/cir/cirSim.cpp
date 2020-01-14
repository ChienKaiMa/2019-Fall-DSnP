/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <bitset>
#include <unordered_set>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
struct FEC {
  size_t operator () (size_t gId) const {
    CirGate* myGate = cirMgr->getGate(gId);
    return myGate->getSimValue();
  }
};

struct FKey {
  bool operator () (size_t gId1, size_t gId2) const {
    FEC f;
    return f(gId1) == f(gId2);
  }
};

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  // Packing the patterns into size_t
  size_t myBits[_miloa[1]];
  string line;
  size_t totPatNum = 0;
  size_t patNum = 0;
  size_t simCount = 0;
  bool isEOF = false;
  while (!isEOF)
  {
    if (packPat(patternFile, patNum)) {
      isEOF = true;
      if (patNum == 0) {
        break;
      }
    }
    ++simCount;
    for (size_t i=0; i<_dfsList.size(); ++i) {
      _dfsList[i]->simulate();
    }
    checkFEC();
  }
  // TODO: Find FEC Groups
  
  checkFEC();
  if (!_fecGroups.empty()) {
    cout << "Total #FEC Group = " << _fecGroups.size() << endl;
  }

  if (patNum != 0) {
    totPatNum = (simCount-1) * 64 + patNum;
  } else {
    totPatNum = simCount * 64;
  }  
  cout << totPatNum << " patterns simulated." << endl;
  // Save the simValues
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

size_t
CirMgr::patGen(string)
{
  return 0; 
}

bool
CirMgr::packPat(ifstream& patternFile, size_t& patNum)
{
  // Packing the patterns into size_t
  patNum = 0;
  bool isEOF = false;
  size_t myBits[_miloa[1]];
  string line;
  for (int i=0; i<_miloa[1]; ++i) {
    myBits[i] = 0;
  }
  while (!isEOF) {
    if (!getline(patternFile, line)) {
      isEOF = true;
      if (patNum == 0) {
        return isEOF;
      }
      break;
    }
    ++patNum;
    // TODO: Check if there's something fishy in the line
    for (int i=0; i<_miloa[1]; ++i) {
      myBits[i] = (myBits[i] << 1) + (line[i] - '0');
    }
    if (patNum >= 64) {
      break;
    }
  }
  for (int i=0; i<_miloa[1]; ++i) {
    getGate(_piList[i])->SimValue = revPat(myBits[i], patNum);
  }
  return isEOF;
}

size_t
CirMgr::revPat(size_t pat, int count)
{
  size_t newPat = 0;
  while (count > 0) {
    newPat = (newPat << 1) + (pat % 2);
    pat /= 2;
    --count;
  }
  return newPat;
}

void
CirMgr::checkFEC()
{
  if (_fecGroups.empty()) {
    _fecGroups.push_back(_aigList);
  }
  for (size_t i=0; i<_fecGroups.size(); ++i) {
    vector<size_t>& myGroup = _fecGroups[i];
    for (size_t j=0; j<myGroup.size(); ++j) {
      unordered_set<size_t, FEC, FKey> FHash;
      auto myPair = FHash.emplace(myGroup[j]);
      if (j==0) {
        // Insert it in a new group
        // It is strange
      }
      if (!myPair.second) {
        // The same hash as j==0
        // Delete it here and insert it in another group
      }
    }
  }
}

bool
CirGate::simulate()
{
  if (isAig()) {
    AigGateV& myFIn1 = ((AigGate*)this)->_fanin1;
    AigGateV& myFIn2 = ((AigGate*)this)->_fanin2;
    if (myFIn1.isInv() && myFIn2.isInv()) {
      SimValue = ~(myFIn1.gate()->SimValue) & ~(myFIn2.gate()->SimValue);
    } else if (myFIn1.isInv()) {
      SimValue = ~(myFIn1.gate()->SimValue) & myFIn2.gate()->SimValue;
    } else if (myFIn2.isInv()) {
      SimValue = myFIn1.gate()->SimValue & ~(myFIn2.gate()->SimValue);
    } else {
      SimValue = myFIn1.gate()->SimValue & myFIn2.gate()->SimValue;
    }
  } else if (_myType == PO_GATE) {
    AigGateV& myFIn = ((PoGate*)this)->_fanin;
    if (myFIn.isInv()) {
      SimValue = ~(myFIn.gate()->SimValue);
    } else {
      SimValue = myFIn.gate()->SimValue;
    }
  }
  return true;
}

/*
bool
CirGate::simulate()
{
  // PI, Undef, Const are Don't cares

  if (_myType == PI_GATE || isUndef() || _myType == CONST_GATE) {
    return true;
  }
  
  Recursively simulate each fanin.
  If (no fanin has value change) return false;
  Simulate this gate;
  if (value changed) return true;
  return false;
  
  size_t oldSim = SimValue;
  if (isAig()) {
    AigGateV& myFIn1 = ((AigGate*)this)->_fanin1;
    AigGateV& myFIn2 = ((AigGate*)this)->_fanin2;
    if (!(myFIn1.gate()->simulate()) && !(myFIn2.gate()->simulate())) {
      return false;
    }
    if (myFIn1.isInv() && myFIn2.isInv()) {
      SimValue = ~(myFIn1.gate()->SimValue) & ~(myFIn2.gate()->SimValue);
    } else if (myFIn1.isInv()) {
      SimValue = ~(myFIn1.gate()->SimValue) & myFIn2.gate()->SimValue;
    } else if (myFIn2.isInv()) {
      SimValue = myFIn1.gate()->SimValue & ~(myFIn2.gate()->SimValue);
    } else {
      SimValue = myFIn1.gate()->SimValue & myFIn2.gate()->SimValue;
    }
  } else if (_myType == PO_GATE) {
    AigGateV& myFIn = ((PoGate*)this)->_fanin;
    if (!myFIn.gate()->simulate()) {
      return false;
    }
    if (myFIn.isInv()) {
      SimValue = ~(myFIn.gate()->SimValue);
    } else {
      SimValue = myFIn.gate()->SimValue;
    }
  }
  if (oldSim == SimValue) {
    return false;
  } else {
    return true;
  }
}
*/