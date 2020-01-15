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
#include <cmath>

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
    size_t myVal = myGate->getSimValue();
    if (myVal > ~myVal) {
      myVal = ~myVal;
    }
    return myVal;
  }
};

struct FKey {
  bool operator () (size_t gId1, size_t gId2) const {
    FEC f;
    //cout << f(gId1) << " " << f(gId2) << endl;
    return f(gId1) == f(gId2) || f(gId1) == ~f(gId2);
  }
};

struct SortKey {
  bool operator () (vector<size_t>& L1, vector<size_t>& L2) const {
    return (L1[0] < L2[0]);
  }
};

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  int simCount = sqrt(_miloa[1] + _miloa[3] + _miloa[4]) * 4;
  for (int i=0; i<simCount; ++i) {
    // Random
    for (int i=0; i<_miloa[1]; ++i) {
      getGate(_piList[i])->SimValue = rnGen(INT_MAX);
    }
    for (size_t i=0; i<_dfsList.size(); ++i) {
      _dfsList[i]->simulate();
    }
    if (_simLog)  {
      getSimLog(32);
    }
    checkFEC();
  }
  if (!_fecGroups.empty()) {
    cout << "Total #FEC Group = " << _fecGroups.size() << endl;
  }
  cout << simCount * 32 << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  // Packing the patterns into size_t
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
    if (_simLog)  {
      getSimLog(patNum);
    }
    checkFEC();
  }
  if (patNum != 0) {
    totPatNum = (simCount-1) * 64 + patNum;
  } else {
    totPatNum = simCount * 64;
  }
  if (!_fecGroups.empty()) {
    cout << "Total #FEC Group = " << _fecGroups.size() << endl;
  }
  cout << totPatNum << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::getSimLog(const size_t patNum)
{
  /*
  cout << patNum << endl;
  for (int i=63; i>=64-int(patNum); --i) {
    for (size_t j=0; j<_piList.size(); ++j) {
      bitset<64> myBit (getGate(_piList[j])->SimValue);
      *_simLog << myBit[i];
    }
    *_simLog << " ";
    for (size_t j=0; j<_poList.size(); ++j) {
      bitset<64> myBit (getGate(_poList[j])->SimValue);
      *_simLog << myBit[i];
    }
    *_simLog << endl;
  }
  */
  if (patNum == 64) {
    for (int i=63; i>=0; --i) {
      for (size_t j=0; j<_piList.size(); ++j) {
        bitset<64> myBit (getGate(_piList[j])->SimValue);
        *_simLog << (myBit.to_string())[i];
      }
      *_simLog << " ";
      for (size_t j=0; j<_poList.size(); ++j) {
        bitset<64> myBit (getGate(_poList[j])->SimValue);
        *_simLog << (myBit.to_string())[i];
      }
      *_simLog << endl;
    }
  } else {
    for (int i=63; i>=64-patNum; --i) {
      for (size_t j=0; j<_piList.size(); ++j) {
        bitset<64> myBit (getGate(_piList[j])->SimValue);
        *_simLog << (myBit.to_string())[i];
      }
      *_simLog << " ";
      for (size_t j=0; j<_poList.size(); ++j) {
        bitset<64> myBit (getGate(_poList[j])->SimValue);
        *_simLog << (myBit.to_string())[i];
      }
      *_simLog << endl;
    }
  }
}

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
      if (line.length() != _miloa[1]) {
        cerr << "Pattern(" << line << ") length(" << line.length()
        << ") does not match the number of inputs(" << _miloa[1]
        << ") in a circuit!!" << endl;
        patNum = 0;
        return true;
      }
      if (line[i] != '0' && line[i] != '1') {
        cerr << "Error: Pattern(" << line << ") contains a non-0/1"
        << " character('" << line[i] << "')." << endl;
        patNum = 0;
        return true;
      }
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
    vector<size_t> myList;
    myList.push_back(0);
    myList.insert(myList.end(), _aigList.begin(), _aigList.end());
    _fecGroups.push_back(myList);
  }
  vector<vector<size_t>> temp;
  for (size_t i=0; i<_fecGroups.size(); ++i) {
    vector<size_t>& myGroup = _fecGroups[i];
    unordered_set<size_t, FEC, FKey> FHash;
    for (size_t j=0; j<myGroup.size(); ++j) {
      auto myPair = FHash.emplace(myGroup[j]);
      if (!myPair.second) {
        // The same hash as someone
        // Delete it here and insert it in another group
        bool isInsert = false;
        for (size_t k=0; k<temp.size(); ++k) {
          if (temp[k][0] == *(myPair.first)) {
            temp[k].push_back(myGroup[j]);
            isInsert = true;
            break;
          }
        }
        if (!isInsert) {
          vector<size_t> smallGroup;
          smallGroup.push_back(*(myPair.first));
          smallGroup.push_back(myGroup[j]);
          temp.push_back(smallGroup);
        }
      }
    }
  }
  for (size_t i=0; i < temp.size(); ++i) {
    ::sort((temp[i]).begin(), temp[i].end());
  }
  SortKey s;
  ::sort(temp.begin(), temp.end(), s);

  _fecGroups = temp;
}

vector<size_t>
CirMgr::findFECGroup(const CirGate* g) const
{
  for (size_t i=0; i<_fecGroups.size(); ++i) {
    FEC f;
    if (f(_fecGroups[i][0]) == f(g->_gateId)) {
      return _fecGroups[i];
    }
  }
  return vector<size_t>();
}

void
CirGate::printFECGroups() const
{
  if (isAig() || _myType == CONST_GATE) {
    vector<size_t> myGroup = cirMgr->findFECGroup(this);
    for (size_t i=0; i<myGroup.size(); ++i) {
      if (myGroup[i] != _gateId) {
        CirGate* myGate = cirMgr->getGate(myGroup[i]);
        size_t myVal = myGate->getSimValue();
        cout << " ";
        if (myVal == ~SimValue) {
          cout << "!";
        }
        cout << myGroup[i];
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