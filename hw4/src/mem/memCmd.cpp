/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   // Newly add
   vector<string> tokens;
   if (!lexOptions(option, tokens)) return CMD_EXEC_ERROR;
   else if (tokens.empty()) { return CmdExec::errorOption(CMD_OPT_MISSING, ""); } 
   else if (tokens.size() == 2) { return CmdExec::errorOption(CMD_OPT_ILLEGAL, ""); }
   else if (tokens.size() == 1) {
      int numObjects;
      if (myStr2Int(tokens[0], numObjects)) {
         if (numObjects > 0) {
            try { mtest.newObjs(size_t(numObjects)); }
            catch (bad_alloc& e) { }
            return CMD_EXEC_DONE;
         }
         else  return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
      } else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);         
   } else if (tokens.size() == 3) {
      if (myStrNCmp("-Array", tokens[0], 2)) {
         if (myStrNCmp("-Array", tokens[1], 2)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
         }
         // the rest (0, 2) are numbers
         int arraySize, numObjects; // 2, 0
         if (myStr2Int(tokens[2], arraySize) && myStr2Int(tokens[0], numObjects)) {
            if (arraySize <= 0)  return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
            if (numObjects <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
            try { mtest.newArrs(numObjects, arraySize); }
            catch (bad_alloc& e) { }
         } else return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
      } else {
         int arraySize, numObjects; // 1, 2
         if (myStr2Int(tokens[1], arraySize) && myStr2Int(tokens[2], numObjects)) {
            if (arraySize <= 0)  return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
            if (numObjects <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
            try { mtest.newArrs(numObjects, arraySize); }
            catch (bad_alloc& e) { }
         } else return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
      }
   } else   return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
   //
   // Use try-catch to catch the bad_alloc exception
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   // Newly add: Parse the options
   vector<string> tokens;
   bool randNotIdx, objNotArr;
   size_t remainIdx;

   if (!lexOptions(option, tokens)) return CMD_EXEC_ERROR;
   else if (tokens.empty() || tokens.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   else if (tokens.size() == 2) {
      remainIdx = 1;
      objNotArr = 1;
      // Check if -Index or -Random
      if (myStrNCmp("-Index", tokens[0], 2)) {
         if (myStrNCmp("-Random", tokens[0], 2)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
         } else randNotIdx = 1;
      } else randNotIdx = 0;
   } else if (tokens.size() == 3) {
      objNotArr = 0;
      // Check if -Array, -Index, and -Random
      if (myStrNCmp("-Array", tokens[0], 2)) {
         if (myStrNCmp("-Array", tokens[2], 2)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
         } else if (myStrNCmp("-Index", tokens[0], 2)) {
            if (myStrNCmp("-Random", tokens[0], 2)) {
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
            } else randNotIdx = 1;  remainIdx = 1;
         } else randNotIdx = 0;  remainIdx = 1;
      } else {
         if (myStrNCmp("-Index", tokens[1], 2)) {
            if (myStrNCmp("-Random", tokens[1], 2)) {
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
            } else randNotIdx = 1;  remainIdx = 2;
         } else randNotIdx = 0;  remainIdx = 2;
      }
   } else   return CmdExec::errorOption(CMD_OPT_EXTRA, "");   
         
   // Get size_t numRandId / size_t objId
   int whateverId;
   if(!myStr2Int(tokens[remainIdx], whateverId)) {
      cout << "yeah" << remainIdx<< endl;
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx]);
   } else if (whateverId < 0) {
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx]);
   } else {
      if (objNotArr) {
         if (randNotIdx) {
            int i = 0, myRand;
            if (mtest.getObjListSize() == 0) {
               cerr << "Size of object list is 0!!" << endl;
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx-1]);
            }
            while (i<whateverId) {
               myRand = rnGen(mtest.getObjListSize());
               if (size_t(myRand) < mtest.getObjListSize() && myRand >= 0) {
                  mtest.deleteObj(myRand);
               }
               ++i;
            }
         } else {
            if (size_t(whateverId) < mtest.getObjListSize()) {
               mtest.deleteObj(whateverId);
            } else {
               cerr << "Size of object list (" << mtest.getObjListSize()
               << ") is <= " << whateverId << "!!" << endl;
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx]);
            }
         }
      } else {
         if (randNotIdx) {
            // Random delete
            int i = 0, myRand;
            if (mtest.getArrListSize() == 0) {
               cerr << "Size of array list is 0!!" << endl;
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx-1]);
            }
            while (i<whateverId) {
               myRand = rnGen(mtest.getArrListSize());
               if (size_t(myRand) < mtest.getArrListSize() && myRand >= 0) {
                  mtest.deleteArr(myRand);
               }
               ++i;
            }
         } else {
            if (size_t(whateverId) < mtest.getArrListSize() && whateverId >= 0) {
               mtest.deleteArr(whateverId);
            } else {
               cerr << "Size of array list (" << mtest.getArrListSize()
               << ") is <= " << whateverId << "!!" << endl;
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[remainIdx]);
            }
         }
      }
   }   
   //
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


