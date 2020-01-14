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
#include <bitset>

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
  size_t getSimValue() const { return SimValue; }
  virtual bool isAig() const = 0;
  virtual bool isUndef() const = 0;

  // Printing functions
  virtual void printGate() const = 0;
  void reportGate() const;
  void reportFanin(int level) const;
  void reportFanout(int level) const;
  void printFanin(int level, string myBlank) const;
  void printFanout(int level, string myBlank) const;
  void getFanin(vector<CirGate*>&, vector<CirGate*>&) const;

  // Opt
  virtual void sweep(IdList&) = 0;
  virtual void deleteFanout(AigGateV) = 0;
  virtual bool isFanoEmpty() = 0;
  virtual void replaceGate(AigGateV) = 0;
   
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
  size_t            SimValue = 0; // Not sure
  void printSimValue() const {
    string mySimValue = (bitset<sizeof(size_t)*8> (SimValue)).to_string();
    cout << mySimValue.substr(0, 8) << "_" << mySimValue.substr(8, 8) << "_"
    << mySimValue.substr(16, 8) << "_" << mySimValue.substr(24, 8) << "_"
    << mySimValue.substr(32, 8) << "_" << mySimValue.substr(40, 8) << "_"
    << mySimValue.substr(48, 8) << "_" << mySimValue.substr(56, 8);
  }
  bool simulate();

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

  AigGateV operator ! () {
    return AigGateV(((CirGate*)this->gate()), !(this->isInv()));
  }

  bool operator == (AigGateV a) {
    return (this->_gateV == a._gateV);
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
  void printGate() const override
  {
    cout << "AIG " << _gateId << " ";
    if (_fanin1.gate()->isUndef()) { cout << "*"; }
    if (_fanin1.isInv()) { cout << "!"; }
    cout << (_fanin1.gate())->getGateId() << " ";
    if (_fanin2.gate()->isUndef()) { cout << "*"; }
    if (_fanin2.isInv()) { cout << "!"; }
    cout << (_fanin2.gate())->getGateId();
    if (_mySymbol.empty()) {
      cout << endl;
    } else {
      cout << " (" << _mySymbol << ")" << endl;
    }
  }

  void sweep(IdList& toSweep) override {
    toSweep.push_back(_gateId);
    // 1. store its fanins
    CirGate* f1 = _fanin1.gate();
    CirGate* f2 = _fanin2.gate();
    // 2. Remove this gate from its fanins' fanouts
    f1->deleteFanout(AigGateV((CirGate*)this, _fanin1.isInv()));
    if (f1->isFanoEmpty()) {
      if (f1->isAig() || f1->isUndef()) {
        f1->sweep(toSweep);
      } else if (f1->getTypeStr() == "PI") {
        toSweep.push_back(f1->getGateId());
      }
    }
    f2->deleteFanout(AigGateV((CirGate*)this, _fanin2.isInv()));
    if (f2->isFanoEmpty()) {
      if (f2->isAig() || f2->isUndef()) {
        f2->sweep(toSweep);
      } else if (f2->getTypeStr() == "PI") {
        toSweep.push_back(f2->getGateId());
      }
    }
  }

  void deleteFanout(AigGateV g) override {
    for (size_t i=0; i<_fanout.size(); ++i) {
      if (g.gate() == _fanout[i].gate()) {
        _fanout[i] = _fanout[_fanout.size()-1];
        _fanout.pop_back();
      }
    }
  }

  bool isFanoEmpty() override {
    return _fanout.empty();
  }

  void optimize(IdList&);
  void replaceGate(AigGateV repl) override;
  //Sim

  AigGateV _fanin1;
  AigGateV _fanin2;
  vector<AigGateV> _fanout;
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

  void sweep(IdList& toSweep) override {
    toSweep.push_back(_gateId);
  }

  void deleteFanout(AigGateV g) override {
    for (size_t i=0; i<_fanout.size(); ++i) {
      if (g.gate() == _fanout[i].gate()) {
        _fanout[i] = _fanout[_fanout.size()-1];
        _fanout.pop_back();
      }
    }
  }

  bool isFanoEmpty() override {
    return _fanout.empty();
  }

  void replaceGate(AigGateV) override;

  vector<AigGateV> _fanout;
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

  void sweep(IdList& toSweep) override {
    // Do nothing
  }

  void deleteFanout(AigGateV g) override {
    for (size_t i=0; i<_fanout.size(); ++i) {
      if (g.gate() == _fanout[i].gate()) {
        _fanout[i] = _fanout[_fanout.size()-1];
        _fanout.pop_back();
      }
    }
  }

  void replaceGate(AigGateV) override;

  bool isFanoEmpty() override {
    return _fanout.empty();
  }

  vector<AigGateV> _fanout;
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

  void sweep(IdList& toSweep) override {
    // Do nothing
  }

  void deleteFanout(AigGateV g) override {
    // Do nothing
  }

  bool isFanoEmpty() override {
    return true;
  }

  void replaceGate(AigGateV) override {

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

  void sweep(IdList& toSweep) override {
    // Do nothing
  }

  void deleteFanout(AigGateV g) override {
    for (size_t i=0; i<_fanout.size(); ++i) {
      if (g.gate() == _fanout[i].gate()) {
        _fanout[i] = _fanout[_fanout.size()-1];
        _fanout.pop_back();
      }
    }
  }

  bool isFanoEmpty() override {
    return _fanout.empty();
  }

  void replaceGate(AigGateV) override;

  vector<AigGateV> _fanout;
};

#endif // CIR_GATE_H
