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
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class UndefGate;
class PiGate;
class PoGate;
class AigGate;
class AigGateV;
class ConstGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
  CirGate() {}
  virtual ~CirGate() {}

  friend CirMgr;

  // Basic access methods
  string getTypeStr() const {
    switch (_myType)
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
  unsigned getGateId() const { return _gateId; }
  virtual bool isAig() const = 0;
  virtual bool isUndef() const = 0;

  // Printing functions
  virtual void printGate() const = 0;
  void reportGate() const;
  void reportFanin(int level) const;
  void reportFanout(int level) const;
   
private:
protected:
  // Gate identity
  unsigned _gateId;
  GateType _myType;
  unsigned _lineNo;
  string _mySymbol;

  // Connected gates
  // vector<AigGateV> _fanin;
  // GateList _fanout;

  // DFS
  size_t        _ref;

  // Simulation
  static vector<GateList>*  FECgroups; // Not sure
  vector<size_t>            SIMValues; // Not sure

};

class AigGateV
{
public:
  #define NEG 0x1

  AigGateV() {}
  AigGateV(CirGate* g, size_t phase) :
    _gateV(size_t(g) + phase) { }
  ~AigGateV() {}
  
  AigGate* gate() const {
    return (AigGate*)(_gateV & ~size_t(NEG));
  }

  // Basic access methods
  bool isInv() const {
    return (_gateV & NEG);
  }

protected:
  size_t _gateV;
};

class AigGate : public CirGate
{
public:
  AigGate() {}
  AigGate(unsigned gId, unsigned lNo) {
    _gateId = gId;
    _myType = AIG_GATE;
    _lineNo = lNo;
    _mySymbol = "";
    _ref = 0;
  }
  ~AigGate() {}

  // Basic access methods
  bool isAig() const override {
    return true;
  }

  bool isUndef() const override {
    return false;
  }

  // Printing functions
  void printGate() const override;

  AigGateV _fanin1;
  AigGateV _fanin2;
  GateList _fanout;
};


class UndefGate : public CirGate
{
public:
  UndefGate() {}
  UndefGate(unsigned gId) {
    _gateId = gId;
    _myType = UNDEF_GATE;
    _lineNo = 0;
    _mySymbol = "";
    _ref = 0;
  }
  ~UndefGate() {}

  // Basic access methods
  bool isAig() const override {
    return false;
  }

  bool isUndef() const override {
    return true;
  }

  // Printing functions
  void printGate() const override {
    return;
  }

  GateList _fanout;
};

class PiGate : public CirGate
{
public:
  PiGate() {}
  PiGate(unsigned gId, unsigned lNo) {
    _gateId = gId;
    _myType = PI_GATE;
    _lineNo = lNo;
    _mySymbol = "";
    _ref = 0;
  }
  ~PiGate() {}

  // Basic access methods
  bool isAig() const override {
    return false;
  }

  bool isUndef() const override {
    return false;
  }

  // Printing functions
  void printGate() const override {
    cout << "PI  " << _gateId;
    if (_mySymbol.empty()) {
      cout << endl;
    } else {
      cout << " (" << _mySymbol << ")" << endl;
    }
  }

  GateList _fanout;
};

class PoGate : public CirGate
{
public:
  PoGate() {}
  PoGate(unsigned gId, unsigned lNo) {
    _gateId = gId;
    _myType = PO_GATE;
    _lineNo = lNo;
    _mySymbol = "";
    _ref = 0;
  }
  ~PoGate() {}

  // Basic access methods
  bool isAig() const override {
    return false;
  }

  bool isUndef() const override {
    return false;
  }

  // Printing functions
  void printGate() const override {
    cout << "PO  " << _gateId << " ";
    if (_fanin.gate()->isUndef()) { cout << "*"; }
    if (_fanin.isInv()) {
      cout << "!";
    }
    cout << (_fanin.gate())->getGateId();
    if (_mySymbol.empty()) {
      cout << endl;
    } else {
      cout << " (" << _mySymbol << ")" << endl;
    }
  }

  AigGateV _fanin;
};

class ConstGate : public CirGate
{
public:
  ConstGate() {
    _gateId = 0;
    _myType = CONST_GATE;
    _lineNo = 0;
    _mySymbol = "";
    _ref = 0;
  }
  ~ConstGate() {}

  // Basic access methods
  bool isAig() const override {
    return false;
  }

  bool isUndef() const override {
    return false;
  }

  // Printing functions
  void printGate() const override {
    cout << "CONST0" << endl;
  }

  GateList _fanout;
};

#endif // CIR_GATE_H
