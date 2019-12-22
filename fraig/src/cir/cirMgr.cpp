/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static size_t _globalRef;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   lineNo = 0;
   colNo = 0;
   ifstream ifs(fileName.c_str());
   if (!ifs) {
      cerr << "Error: \"" << fileName << "\" does not exist!!" << endl;
      return false;
   }

   // Omit error handling in reading circuit files
   string token;
   int literal[3];
   bool isComment = false;

   // Read header
   ifs >> token >> _miloa[0] >> _miloa[1] >> _miloa[2] >> _miloa[3] >> _miloa[4];
   ++lineNo;

   // Initialize _gateList
   for (int i=0; i < (_miloa[0] + _miloa[3] + 1); ++i) {
      _gateList.push_back(0);
   }
   
   // push_back const0
   CirGate* cg = new ConstGate();
   _gateList[0] = cg;

   // Read PIs
   for (int i=0; i < _miloa[1]; ++i) {
      // Get gateId
      ifs >> literal[0];
      assert(literal[0]%2 == 0);
      // Create new PiGate
      literal[0] /= 2;
      CirGate* pi = new PiGate(literal[0], lineNo+1);
      _gateList[literal[0]] = pi;
      _piList.push_back(literal[0]);
      ++lineNo;
   }

   // Read POs
   for (int i=0; i < _miloa[3]; ++i) {
      ifs >> literal[0];
      // TODO
      CirGate* po = new PoGate(_miloa[0]+1 + i, lineNo+1);
      po->_fanin.push_back(AigGateV((CirGate*)(literal[0]), (size_t)0));
      _gateList[_miloa[0]+1 + i] = po;
      _poList.push_back(_miloa[0]+1 + i);
      ++lineNo;
   }

   // Read AIGs
   for (int i=0; i < _miloa[4]; ++i) {
      ifs >> literal[0] >> literal[1] >> literal[2];
      // TODO
      literal[0] /= 2;
      CirGate* aig = new AigGate(literal[0], lineNo+1);
      aig->_fanin.push_back(AigGateV((CirGate*)(literal[1]), (size_t)0));
      aig->_fanin.push_back(AigGateV((CirGate*)(literal[2]), (size_t)0));
      _gateList[literal[0]] = aig;
      _aigList.push_back(literal[0]);
      ++lineNo;
   }

   // Read symbols and comments
   string line;
   getline(ifs, line);
   getline(ifs, line);
   colNo = 0;
   while (line != "") {
      if (isComment || line == "c") {
         isComment = true;
      } else {
         // Read symbols
         // TODO
         string gate;
         int gateIdx;
         string symbol;
         line = line.substr(myStrGetTok(line, gate));
         if (gate.substr(0, 1) == "i") {
            gate = gate.substr(1);
            myStr2Int(gate, gateIdx);
            CirGate* thisGate = getGate(_piList[gateIdx]);
            myStrGetTok(line, symbol);
            thisGate->_mySymbol = symbol;
         } else if (gate.substr(0, 1) == "o") {
            gate = gate.substr(1);
            myStr2Int(gate, gateIdx);
            CirGate* thisGate = getGate(_poList[gateIdx]);
            myStrGetTok(line, symbol);
            thisGate->_mySymbol = symbol;
         }
      }
      ++lineNo;
      getline(ifs, line);
   }
  
   // Connect gates
   connect();

   // Generate _dfsList
   genDFSList();
   return true;
}

void
CirMgr::connect()
{
   // Connect Po to its fanin
   for (int i=0; i<_miloa[3]; ++i) {
      connectPo(i);
   }
   // Connect Aig to its fanin
   for (int i=0; i<_miloa[4]; ++i) {
      connectAig(i);
   }

   // Check for unused gates
   for (int i=0; i<_miloa[1]; ++i) {
      CirGate* myPi = getGate(_piList[i]);
      if (myPi->_fanout.empty()) {
         _unused.push_back(_piList[i]);
      }
   }
   for (int i=0; i<_miloa[4]; ++i) {
      CirGate* myAig = getGate(_aigList[i]);
      if (myAig->_fanout.empty()) {
         _unused.push_back(_aigList[i]);
      }
   }
}

void
CirMgr::connectPo(int idx)
{
   CirGate* myPo = getGate(_poList[idx]);
   size_t poFaninId = (size_t)(myPo->_fanin[0].gate());
   poFaninId /= 2;
   CirGate* faninGate = getGate(poFaninId);
   if (faninGate == 0) {
      CirGate* undef = new UndefGate(poFaninId);
      faninGate = undef;
      _gateList[poFaninId] = faninGate;
      myPo->_fanin[0] = AigGateV(faninGate, 0);
      _floatFanIns.push_back(_poList[idx]);
   } else {
      bool isInv = myPo->_fanin[0].isInv();
      myPo->_fanin[0] = AigGateV(faninGate, isInv);
   }
   faninGate->_fanout.push_back(myPo);
}

void
CirMgr::connectAig(int idx)
{
   CirGate* myAig = getGate(_aigList[idx]);
   size_t aigFaninId;
   CirGate* faninGate;
   bool isFloat = false;
   for (int j=0; j<2; ++j) {
      aigFaninId = (size_t)(myAig->_fanin[j].gate());
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
      } else {
         bool isInv = myAig->_fanin[j].isInv();
         myAig->_fanin[j] = AigGateV(faninGate, isInv);
      }
      faninGate->_fanout.push_back(myAig);
   }
}

bool
CirMgr::genDFSList()
{
   ++(_globalRef);
   for (int i=0; i<_miloa[3]; ++i)
   {
      dfsVisit(getGate(_poList[i]));
   }
   return true;
}

CirGate*
CirMgr::dfsVisit(CirGate* start)
{
   if (start == 0) {
      return 0;
   }
   if (_globalRef == start->_ref) {
      return 0;
   }
   if (start->getTypeStr() == "UNDEF") {
      return 0;
   } else if (start->getTypeStr() == "PO") {
      dfsVisit(start->_fanin[0].gate());
   } else if (start->getTypeStr() == "AIG") {
      dfsVisit(start->_fanin[0].gate());
      dfsVisit(start->_fanin[1].gate());
   }
   ++(start->_ref);
   _dfsList.push_back(start);
   return start;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << "  PI" << setw(12) << _miloa[1] << endl;
   cout << "  PO" << setw(12) << _miloa[3] << endl;
   cout << "  AIG" << setw(11) << _miloa[4] << endl;
   cout << "------------------" << endl;
   cout << "  Total" << setw(9) << _miloa[1] + _miloa[3] + _miloa[4] << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
}

void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

