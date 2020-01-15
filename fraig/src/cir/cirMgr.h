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
   ~CirMgr() { // TODO: reset();
   } 

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
   void connectPo(const int&);
   void connectAig(const int&);
   void genDFSList();
   CirGate* dfsVisit(CirGate*, bool);
   CirGate* dfsVisit(CirGate*, vector<CirGate*>) const;

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void getSimLog(const size_t);

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
   vector<size_t> findFECGroup(const CirGate*) const;

private:
   // Maximal variable index, # of PIs, # of latches, # of POs, # of AIGs
   int                _miloa[5];
   // GateLists
   GateList   _gateList;
   vector<size_t>     _piList;
   vector<size_t>     _poList;
   vector<size_t>     _aigList;
   GateList           _dfsList;
   // FloatingLists
   // (b) A gate that cannot be reached from PI
   // Gates with floating fanins
   // referenced but not defined (UNDEF)
   // (c) A gate that cannot be reached from any PI and PO
   // (d) A gate with a floating fanin
   // Contains AIGs and POs
   IdList             _floatFanIns;
   
   // (a) A gate that cannot reach any PO
   // Gates defined but not used
   // Contains PIs and AIGs
   IdList             _unused;
   //
   ofstream           *_simLog;
   vector<vector<size_t>>   _fecGroups;

   void deleteGate(CirGate*);
   size_t patGen(string);
   bool packPat(ifstream& patternFile, size_t& patNum);
   size_t revPat(size_t, int);
   void checkFEC();
   void reset() {
      for (int i=0; i<_miloa[0]+_miloa[3]+1; ++i) {
         if (_gateList[i]) {
            delete (_gateList[i]);
            _gateList[i] = 0;
         }
      }
   }
};

#endif // CIR_MGR_H
