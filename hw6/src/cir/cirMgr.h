/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
  CirMgr() {}
  ~CirMgr() { reset(); }

  // Access functions
  // return '0' if "gid" corresponds to an undefined gate.
  CirGate *getGate(unsigned gid) const
  {
    if (gid < _allGates.capacity()) {
      return _allGates[gid];
    } else {
      return 0;
    }
  }

  // Member functions about circuit construction
  bool readCircuit(const string &);
  bool readHeader(istream&);
  bool readInputs(string &);
  bool readOutputs(string &, const int &);
  bool readAiGates(string &);
  bool readSymbols(string &);
  bool readComment(string &);
  bool connectGates();
  void sortGates();
  void reset();
  
  // Member functions about circuit reporting
  void printSummary() const;
  void printNetlist() const;
  void printPIs() const;
  void printPOs() const;
  void printFloatGates() const;
  void writeAag(ostream &) const;

  // Depth-first search
  void dfs() const;
  CirGate* dfsVisit(CirGate*) const;

private:
  GateList _unordered;
  GateList _allGates;
  GateList _inputGates;
  GateList _outputGates;
  GateList _float1;
  GateList _float2;
  GateList _float3;
  GateList _float4;
  IdList _aigGates;
  int _maxIdx, _inputNum, _latchNum, _outputNum, _andGateNum;
};

#endif // CIR_MGR_H
