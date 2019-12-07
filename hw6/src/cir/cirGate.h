/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include <cmath>

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
  CirGate() {}
  virtual ~CirGate() {}

  // Basic access methods
  string getTypeStr() const {
    switch (myType)
    {
    case UNDEF_GATE:
      return "UNDEF";
      break;
    case PI_GATE:
      return "PI";
      break;
    case PO_GATE:
      return "PO";
      break;
    case AIG_GATE:
      return "AIG";
      break;
    case CONST_GATE:
      return "CONST";
      break;
    default:
      return "";
      break;
    }
  }
  unsigned getLineNo() const { return _lineNo; }

  // Printing functions
  virtual void printGate() const = 0;
  void reportGate() const;
  void reportFanin(int level) const;
  void reportFanout(int level) const;
  void printFanin(int level, string myBlank) const;
  void printFanout(int level, string myBlank) const;

  uint _gateId;
  uint _lineNo = 0;
  string _symbol = "";
  size_t _ref;
  GateList _fanin;
  GateList _fanout;
  IdList _fanoutNum;
  GateType myType;
private:
protected:
};

class InputGate : public CirGate
{
public:
  InputGate() {}
  InputGate(int g, int l) {
    _gateId = g;
    _lineNo = l;
    _symbol = "";
    _ref = 0;
    myType = PI_GATE;
  }
  ~InputGate() {}

  void printGate() const override {
    cout << "PI  " << _gateId;
    if (_symbol.empty()) { cout << endl; }
    else { cout << " (" << _symbol << ")" << endl; }
  }

private:
};

class OutputGate : public CirGate
{
public:
  OutputGate() {}
  OutputGate(const int g, int l, int f) {
    _gateId = g;
    _lineNo = l;
    _faninNum[0] = f;
    _symbol = "";
    _ref = 0;
    myType = PO_GATE;
    _fanin.reserve(1);
    isUndef[0] = false;
  }
  ~OutputGate() {}

  void printGate() const override
  {
    cout << "PO  " << _gateId;
    cout << " ";
    if (isUndef[0]) { cout << "*"; }
    if (_faninNum[0] % 2 != 0) {
      cout << "!";
    }
    cout << _faninNum[0] / 2;
    if (_symbol.empty()) {
      cout << endl;
    } else {
      cout << " (" << _symbol << ")" << endl;
    }
  }
  uint _faninNum[1];
  bool isUndef[1];
  
private:
};

class AndGate : public CirGate
{
public:
  AndGate() {}
  AndGate(const uint g, uint l, uint fanin1, uint fanin2)
  {
    _gateId = g;
    _lineNo = l;
    // TODO
    // Save fanins
    _faninNum[0] = fanin1;
    _faninNum[1] = fanin2;
    _symbol = "";
    _ref = 0;
    myType = AIG_GATE;
    _fanin.reserve(2);
    isUndef[0] = isUndef[1] = false;
  }
  ~AndGate() {}

  void printGate() const override
  {
    cout << "AIG " << _gateId;
    cout << " ";
    if (isUndef[0]) { cout << "*"; }
    if (isFaninInv(0)) { cout << "!"; }
    cout << _faninNum[0] / 2;
    cout << " ";
    if (isUndef[1]) { cout << "*"; }
    if (isFaninInv(1)) { cout << "!"; }
    cout << _faninNum[1] / 2;
    if (_symbol.empty()) {
      cout << endl;
    } else {
      cout << " (" << _symbol << ")" << endl;
    }
  }

  bool isFaninInv(int num) const
  {
    return (_faninNum[num] % 2 != 0);
  }

  uint _faninNum[2];
  bool isUndef[2];
};

class ConstGate : public CirGate
{
public:
  ConstGate(int i) {
    _gateId = 0;
    _lineNo = 0;
    _symbol = "";
    _ref = 0;
    myType = CONST_GATE;
  }
  ~ConstGate() {}

  void printGate() const override
  {
    cout << getTypeStr() << "0" << endl;
  }
};

class UndefGate : public CirGate
{
public:
  UndefGate() {}
  UndefGate(const uint g) {
    _gateId = g;
    _lineNo = 0;
    _symbol = "";
    _ref = 0;
    myType = UNDEF_GATE;
  }
  ~UndefGate() {}

  void printGate() const override
  {
  }
  // fanin and fanout?

};

#endif // CIR_GATE_H

/*
class AigGate {
  Array<AigGateV> _faninList;
  size_t _ref;
  static size_t _globalRef_s;
};

class AigGateV {
  #define NEG 0x1
  AigGateV(AigGate* g, size_t phase):
      _gateV(size_t(g) + phase) { }
  AigGate* gate() const {
    return (AigGate*)(_gateV & ~size_t(NEG)); }
  bool isInv() const { return (_gateV & NEG); }
};
*/