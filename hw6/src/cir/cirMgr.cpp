/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

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
static size_t _global_ref;
static int dfsCount;

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
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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
   
   string line;
   bool isComment = false;

   // readHeader
   if (!readHeader(ifs)) {
      return false;
   }
   ++lineNo;

   // readInputs
   for (int i=0; i < _inputNum; ++i) {
      getline(ifs, line);
      colNo = 0;
      if(!readInputs(line)) {
         return false;
      }
      ++lineNo;
   }

   // readLatches
   // readOutputs
   for (int i=0; i < _outputNum; ++i) {
      getline(ifs, line);
      colNo = 0;
      if(!readOutputs(line, _maxIdx+1+i)) {
         return false;
      }
      ++lineNo;
   }

   // readAiGates
   for (int i=0; i < _andGateNum; ++i) {
      getline(ifs, line);
      colNo = 0;
      if(!readAiGates(line)) {
         return false;
      }
      ++lineNo;
   }
   
   // readSymbols and read comments
   getline(ifs, line);
   colNo = 0;
   while (line != "") {
      // Not sure what to do with comments
      if (isComment || line == "c") {
         isComment = true;
      } else {
         if(!readSymbols(line)) {
            return false;
         }
      }
      ++lineNo;
      getline(ifs, line);
   }
   
   // Connect the gates
   // Check for undefined gates
   connectGates();
   return true;
}

// Handle parsing error messages
// Use enum CirParseError (bool)
bool
CirMgr::readHeader(istream& ifs)
{
   // TODO: EXTRA_SPACE, MISSING_NUM errMsg and colNo
   
   /*******************************/
   /*    Checking identifier      */
   /*******************************/
   bool isSpace = false;
   buf[colNo] = ifs.get();
   if (buf[0] == ' ') {
      return parseError(EXTRA_SPACE);
   } else if (buf[0] == '\t') {
      errInt = '\t';
      return parseError(ILLEGAL_WSPACE);
   } else if (buf[0] == '\n') {
      errInt = '\n';
      return parseError(ILLEGAL_WSPACE);
   }
   while (!isSpace) {
      ++colNo;
      buf[colNo] = ifs.get();
      if (buf[colNo] == '\t') {
         errInt = '\t';
         return parseError(ILLEGAL_WSPACE);
      } else if (buf[colNo] == '\n') {
         errInt = '\n';
         return parseError(ILLEGAL_WSPACE);
      } else if (isdigit(buf[colNo])) {
         return parseError(MISSING_SPACE);
      } else if (buf[colNo] == ' ') {
         isSpace = true;
      }
   }
   string id(buf, colNo);
   if (id != "aag") {
      errMsg = id;
      cout << id << endl;
      return parseError(ILLEGAL_IDENTIFIER);
   }

   /*******************************/
   /*       Getting MILOA         */
   /*******************************/
   string line;
   getline(ifs, line);
   string m, i, l, o, a;
   if (line != "") {
      line = line.substr(myStrGetTok(line, m));
      if (!myStr2Int(m, _maxIdx)) {
         errMsg = m;
         return parseError(ILLEGAL_NUM);
      }
      line = line.substr(myStrGetTok(line, i));
      if (!myStr2Int(i, _inputNum)) {
         errMsg = i;
         return parseError(ILLEGAL_NUM);
      }
      line = line.substr(myStrGetTok(line, l));
      if (!myStr2Int(l, _latchNum)) {
         errMsg = l;
         return parseError(ILLEGAL_NUM);
      }
      line = line.substr(myStrGetTok(line, o));
      if (!myStr2Int(o, _outputNum)) {
         errMsg = o;
         return parseError(ILLEGAL_NUM);
      }
      myStrGetTok(line, a);
      if (!myStr2Int(a, _andGateNum)) {
         errMsg = a;
         return parseError(ILLEGAL_NUM);
      }
      for (int i=0; i<_maxIdx+_outputNum+1; ++i) {
         _allGates.push_back(0);
      }
      CirGate* cg = new ConstGate(0);
      _unordered.push_back(cg);
      _allGates[0] = cg;
      return true;
   } else {
      // TODO
      colNo = 0;
      errMsg = "";
      return parseError(MISSING_NUM);
   }   
}

bool
CirMgr::readInputs(string& line)
{
   // Should save the inputs in CirMgr
   string inputGate;
   int gateId;
   myStrGetTok(line, inputGate);
   if (!myStr2Int(inputGate, gateId)) {
      errMsg = inputGate;
      return parseError(ILLEGAL_NUM);
   } else if (gateId%2 != 0) {
      errMsg = inputGate;
      return parseError(ILLEGAL_NUM);
   } else if (gateId / 2 > _maxIdx) {
      // May be wrong
      // TODO
      errInt = gateId;
      // colNo;
      return parseError(MAX_LIT_ID);
   } else {
      // TODO
      // Save the input gate
      gateId = gateId/2;
      CirGate* ig = new InputGate(gateId, lineNo+1);
      _inputGates.push_back(ig);
      _unordered.push_back(ig);
      _allGates[gateId] = ig;
      return true;
   }
}

bool
CirMgr::readOutputs(string& line, const int& gateId)
{
   string faninGate;
   int fanin;
   myStrGetTok(line, faninGate);
   if (!myStr2Int(faninGate, fanin)) {
      errMsg = faninGate;
      return parseError(ILLEGAL_NUM);
   } else if (fanin / 2 > _maxIdx) {
      // May be wrong
      // TODO
      errInt = fanin;
      // colNo;
      return parseError(MAX_LIT_ID);
   } else {
      CirGate* og = new OutputGate(gateId, lineNo+1, fanin);
      // TODO
      // Remember to connect save the faninGate to _fanin
      _outputGates.push_back(og);
      _unordered.push_back(og);
      _allGates[gateId] = og;
      return true;
   }
}

bool
CirMgr::readAiGates(string& line)
{
   string faninGate;
   int gateId; // first literal
   int fanin1; // second literal
   int fanin2; // third literal
   line = line.substr(myStrGetTok(line, faninGate));
   if (!myStr2Int(faninGate, gateId)) {
      errMsg = faninGate;
      return parseError(ILLEGAL_NUM);
   } else if (gateId / 2 > _maxIdx) {
      // May be wrong
      // TODO
      errInt = gateId;
      // colNo;
      return parseError(MAX_LIT_ID);
   }
   line = line.substr(myStrGetTok(line, faninGate));
   if (!myStr2Int(faninGate, fanin1)) {
      errMsg = faninGate;
      return parseError(ILLEGAL_NUM);
   } else if (fanin1 / 2 > _maxIdx) {
      // May be wrong
      // TODO
      errInt = fanin1;
      // colNo;
      return parseError(MAX_LIT_ID);
   }
   myStrGetTok(line, faninGate);
   if (!myStr2Int(faninGate, fanin2)) {
      errMsg = faninGate;
      return parseError(ILLEGAL_NUM);
   } else if (fanin2 / 2 > _maxIdx) {
      // May be wrong
      // TODO
      errInt = fanin2;
      // colNo;
      return parseError(MAX_LIT_ID);
   }
   gateId = gateId/2;
   CirGate* aig = new AndGate(gateId, lineNo+1, fanin1, fanin2);
   // TODO
   // Remember to connect save the faninGate to _fanin
   _aigGates.push_back(gateId);
   _unordered.push_back(aig);
   _allGates[gateId] = aig;
   return true;
}

bool
CirMgr::readSymbols(string& line)
{
   string gate;
   int gateIdx;
   string symbol;
   line = line.substr(myStrGetTok(line, gate));
   if (gate.substr(0, 1) == "i") {
      gate = gate.substr(1);
      myStr2Int(gate, gateIdx);
      CirGate* thisGate = cirMgr->_inputGates[gateIdx];
      if (thisGate->_symbol != "") {
         errMsg = "i";
         errInt = gateIdx;
         return parseError(REDEF_SYMBOLIC_NAME);
      }
      myStrGetTok(line, symbol);
      for (size_t i=0; i<symbol.length(); ++i) {
         if (!isprint(symbol[i])) {
            colNo = 3+i;
            errInt = symbol[i];
            return parseError(ILLEGAL_SYMBOL_NAME);
         }
      }
      thisGate->_symbol = symbol;
   } else if (gate.substr(0, 1) == "o") {
      gate = gate.substr(1);
      myStr2Int(gate, gateIdx);
      CirGate* thisGate = cirMgr->_outputGates[gateIdx];
      if (thisGate->_symbol != "") {
         errMsg = "o";
         errInt = gateIdx;
         return parseError(REDEF_SYMBOLIC_NAME);
      }
      myStrGetTok(line, symbol);
      for (size_t i=0; i<symbol.length(); ++i) {
         if (!isprint(symbol[i])) {
            colNo = 3+i;
            errInt = symbol[i];
            return parseError(ILLEGAL_SYMBOL_NAME);
         }
      }
      thisGate->_symbol = symbol;
   } else {
      errMsg = gate.substr(0, 1);
      colNo = 0;
      return parseError(ILLEGAL_SYMBOL_TYPE);
   }   
   return true;
}

bool
CirMgr::connectGates()
{
   // fanin and fanout
   for (int i=0; i<_outputNum; ++i) {
      size_t thisGateId = (((OutputGate*) _outputGates[i]) -> _faninNum[0]) / 2;
      CirGate* faninGate = _allGates[thisGateId];
      if (faninGate == 0) {
         CirGate* undef = new UndefGate(thisGateId);
         _unordered.push_back(undef);
         _float1.push_back(_outputGates[i]);
         _allGates[thisGateId] = undef;
         faninGate = _allGates[thisGateId];
         ((OutputGate*)_outputGates[i])->isUndef[0] = 1;
      }
      _outputGates[i] -> _fanin.push_back(faninGate);
      faninGate->_fanout.push_back(_outputGates[i]);
   }
   for (int i=0; i<_andGateNum; ++i) {
      CirGate* thisGate = _allGates[_aigGates[i]];
      size_t gateId1 = ((AndGate*) thisGate) -> _faninNum[0] / 2;
      size_t gateId2 = ((AndGate*) thisGate) -> _faninNum[1] / 2;
      CirGate* faninGate1 = _allGates[gateId1];
      if (faninGate1 == 0) {
         CirGate* undef = new UndefGate(gateId1);
         _unordered.push_back(undef);
         _float1.push_back(thisGate);
         _allGates[gateId1] = undef;
         faninGate1 = _allGates[gateId1];
         ((AndGate*)thisGate)->isUndef[0] = 1;
      }
      CirGate* faninGate2 = _allGates[gateId2];
      if (faninGate2 == 0) {
         CirGate* undef = new UndefGate(gateId2);
         _unordered.push_back(undef);
         _allGates[gateId2] = undef;
         faninGate2 = _allGates[gateId2];
         ((AndGate*)thisGate)->isUndef[1] = 1;
      }
      thisGate->_fanin.push_back(faninGate1);
      thisGate->_fanin.push_back(faninGate2);
      faninGate1->_fanout.push_back(thisGate);
      faninGate2->_fanout.push_back(thisGate);
      faninGate1->_fanoutNum.push_back(thisGate->_gateId * 2 + ((AndGate*) thisGate) -> _faninNum[0] % 2);
      faninGate2->_fanoutNum.push_back(thisGate->_gateId * 2 + ((AndGate*) thisGate) -> _faninNum[1] % 2);
   }
   for (int i=0; i<_inputNum; ++i) {
      if (_inputGates[i]->_fanout.empty()) {
         _float2.push_back(_inputGates[i]);
      }
   }
   for (int i=0; i<_andGateNum; ++i) {
      CirGate* thisGate = _allGates[_aigGates[i]];
      if (thisGate->_fanout.empty()) {
         _float2.push_back(thisGate);
      }
   }
   sortGates();
   return true;
}

void
CirMgr::sortGates()
{
   if (!_allGates.empty()) {
   }
}


void CirMgr::reset()
{
   if (!_unordered.empty()) {
      for (size_t i=0; i<_unordered.size(); ++i) {
         CirGate* toDel = _unordered[i];
         delete toDel;
      }
   }
   _unordered.clear();
   _allGates.clear();
   _inputGates.clear();
   _outputGates.clear();
   _aigGates.clear();
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
   cout << "  PI" << setw(12) << _inputNum << endl;
   cout << "  PO" << setw(12) << _outputNum << endl;
   cout << "  AIG" << setw(11) << _andGateNum << endl;
   cout << "------------------" << endl;
   cout << "  Total" << setw(9) << _inputNum + _outputNum + _andGateNum << endl;
}

void
CirMgr::printNetlist() const
{
   dfsCount = 0;
   cout << endl;
   dfs();
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (int i=0; i<_inputNum; ++i) {
      cout << " " << _inputGates[i]->_gateId;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (int i=0; i<_outputNum; ++i) {
      cout << " " << _outputGates[i]->_gateId;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   // TODO
   /*
   A gate that cannot reach any PO
(c) A gate that cannot be reached from PI
(b) A gate that cannot be reached from any PI and PO
(d) A gate with a floating fanin
   */
   if (!_float1.empty()) {
      cout << "Gates with floating fanin(s):";
      for (size_t i=0; i<_float1.size(); ++i) {
         cout << " " << _float1[i]->_gateId;
      }
   }
   if (!_float2.empty()) {
      cout << "Gates defined but not used  :";
      for (size_t i=0; i<_float2.size(); ++i) {
         cout << " " << _float2[i]->_gateId;
      }
   }
   if (!_float3.empty()) {
      cout << "";
      for (size_t i=0; i<_float3.size(); ++i) {
         cout << " " << _float3[i]->_gateId;
      }
   }
   if (!_float4.empty()) {
      cout << "";
      for (size_t i=0; i<_float4.size(); ++i) {
         cout << " " << _float4[i]->_gateId;
      }
   }
   if (!(_float1.empty() && _float2.empty() && _float3.empty() && _float4.empty())) {
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   // writeHeader
   outfile << "aag " << _maxIdx << " " << _inputNum << " "
   << _latchNum << " " << _outputNum << " " << _andGateNum << endl;
   // writeIO
   for (int i=0; i<_inputNum; ++i) {
      outfile << _inputGates[i]->_gateId * 2 << endl;
   }
   for (int i=0; i<_outputNum; ++i) {
      outfile << ((OutputGate*)_outputGates[i])->_faninNum[0] << endl;
   }
   // writeAig
   for (int i=0; i<_andGateNum; ++i) {
      AndGate* thisGate = (AndGate*)getGate(_aigGates[i]);
      outfile << thisGate->_gateId * 2 << " " << thisGate->_faninNum[0]
      << " " << thisGate->_faninNum[1] << endl;
   }
   // writeSymbol
   for (int i=0; i<_inputNum; ++i) {
      if (_inputGates[i]->_symbol != "") {
         outfile << "i" << i << " " << _inputGates[i]->_symbol << endl;
      }
   }
   for (int i=0; i<_outputNum; ++i) {
      if (_outputGates[i]->_symbol != "") {
         outfile << "o" << i << " " << _outputGates[i]->_symbol << endl;
      }
   }
   // writeComment
   outfile << "c" << endl;
   outfile << "Koova Kevy" << endl;
}

// Depth-first Search
void CirMgr::dfs() const
{
   ++_global_ref;
   for (int i=0; i<_outputNum; ++i)
   {
      dfsVisit(_outputGates[i]);
   }
}

CirGate*
CirMgr::dfsVisit(CirGate* start) const
{
   if (start == 0) {
      return 0;
   }
   if (_global_ref == start->_ref) {
      return 0;
   }
   if (start->getTypeStr() == "UNDEF") {
      return 0;
   } else if (start->getTypeStr() == "PO") {
      dfsVisit(((OutputGate*)start)->_fanin[0]);
   } else if (start->getTypeStr() == "AIG") {
      dfsVisit(((AndGate*)start)->_fanin[0]);
      dfsVisit(((AndGate*)start)->_fanin[1]);
   }
   ++(start->_ref);
   cout << "[" << dfsCount << "] ";
   start->printGate();
   ++dfsCount;
   return start;
}