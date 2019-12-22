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

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      if (gid < _gateList.capacity()) {
         return _gateList[gid];
      } else {
         return 0;
      }
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void connect();
   void connectPo(int idx);
   void connectAig(int idx);
   bool genDFSList();
   CirGate* dfsVisit(CirGate* start);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   // Maximal variable index, # of PIs, # of latches, # of POs, # of AIGs
   int                _miloa[5];
   // GateLists
   GateList   _gateList;
   vector<size_t>     _piList; // or CirGate* ?
   vector<size_t>     _poList; // or CirGate* ?
   vector<size_t>     _aigList; // or CirGate* ?
   GateList           _dfsList;
   // FloatingLists
   // (b) A gate that cannot be reached from PI
   // Gates with floating fanins
   // referenced but not defined (UNDEF)
   // (c) A gate that cannot be reached from any PI and PO
   // (d) A gate with a floating fanin
   // Contains AIGs and POs
   IdList             _floatFanIns; // Or size_t?
   
   // (a) A gate that cannot reach any PO
   // Gates defined but not used
   // Contains PIs and AIGs
   IdList             _unused; // Or size_t?
   //
   ofstream           *_simLog;
};

#endif // CIR_MGR_H
