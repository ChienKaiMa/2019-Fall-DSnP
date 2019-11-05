/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>

using namespace std;

// Turn this on for debugging
// #define MEM_DEBUG

//--------------------------------------------------------------------------
// Define MACROs
//--------------------------------------------------------------------------
#define MEM_MGR_INIT(T) \
MemMgr<T>* const T::_memMgr = new MemMgr<T>

#define USE_MEM_MGR(T)                                                      \
public:                                                                     \
   void* operator new(size_t t) { return (void*)(_memMgr->alloc(t)); }      \
   void* operator new[](size_t t) { return (void*)(_memMgr->allocArr(t)); } \
   void  operator delete(void* p) { _memMgr->free((T*)p); }                 \
   void  operator delete[](void* p) { _memMgr->freeArr((T*)p); }            \
   static void memReset(size_t b = 0) { _memMgr->reset(b); }                \
   static void memPrint() { _memMgr->print(); }                             \
private:                                                                    \
   static MemMgr<T>* const _memMgr

// You should use the following two MACROs whenever possible to 
// make your code 64/32-bit platform independent.
// DO NOT use 4 or 8 for sizeof(size_t) in your code
//
#define SIZE_T      sizeof(size_t)
#define SIZE_T_1    (sizeof(size_t) - 1)

// TODO: Define them by SIZE_T and/or SIZE_T_1 MACROs.
//
// To promote 't' to the nearest multiple of SIZE_T; 
// e.g. Let SIZE_T = 8;  toSizeT(7) = 8, toSizeT(12) = 16
#define toSizeT(t)      (ceil(double(t) / SIZE_T)*SIZE_T)  // TODO // Not sure
//
// To demote 't' to the nearest multiple of SIZE_T
// e.g. Let SIZE_T = 8;  downtoSizeT(9) = 8, downtoSizeT(100) = 96
#define downtoSizeT(t)  (floor(double(t) / SIZE_T)*SIZE_T)  // TODO // Not sure

// R_SIZE is the size of the recycle list
#define R_SIZE 256

//--------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------
template <class T> class MemMgr;


//--------------------------------------------------------------------------
// Class Definitions
//--------------------------------------------------------------------------
// T is the class that use this memory manager
//
// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemBlock
{
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemBlock(MemBlock<T>* n, size_t b) : _nextBlock(n) {
      _begin = _ptr = new char[b]; _end = _begin + b; }
   ~MemBlock() { delete [] _begin; }

   // Member functions
   void reset() { _ptr = _begin; }
   // 1. Get (at least) 't' bytes memory from current block
   //    Promote 't' to a multiple of SIZE_T
   // 2. Update "_ptr" accordingly
   // 3. The return memory address is stored in "ret"
   // 4. Return false if not enough memory
   bool getMem(size_t t, T*& ret) {
      // TODO
      //
      t = toSizeT(t);
      ret = (T*) _ptr;
      if (getRemainSize() < t)   return false;
      else { _ptr += t; return true; }
      //
   }
   size_t getRemainSize() const { return size_t(_end - _ptr); }
      
   MemBlock<T>* getNextBlock() const { return _nextBlock; }

   // Data members
   char*             _begin;
   char*             _ptr;
   char*             _end;
   MemBlock<T>*      _nextBlock;
};

// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemRecycleList
{
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemRecycleList(size_t a = 0) : _arrSize(a), _first(0), _nextList(0) {}
   ~MemRecycleList() { reset(); }

   // Member functions
   // ----------------
   size_t getArrSize() const { return _arrSize; }
   MemRecycleList<T>* getNextList() const { return _nextList; }
   void setNextList(MemRecycleList<T>* l) { _nextList = l; }
   // pop out the first element in the recycle list
   T* popFront() {
      // TODO
      // Not sure
      T* tmp = _first;
      _first = *((T**) _first);
      return tmp;
      //
   }
   // push the element 'p' to the beginning of the recycle list
   void  pushFront(T* p) {
      // TODO
      // Maybe
      if (_first != 0)  { *((T**) p) = _first; }
      else { *((T**) p) = 0; }
      _first = p;
      //
   }
   // Release the memory occupied by the recycle list(s)
   // DO NOT release the memory occupied by MemMgr/MemBlock
   void reset() {
      // TODO
      //
      while (_first != 0) {
         _first = *((T**) _first);
      }
      //
   }

   // Helper functions
   // ----------------
   // count the number of elements in the recycle list
   size_t numElm() const {
      // TODO
      // Maybe
      size_t countElm = 0;
      T* _elm = _first;
      while (_elm != 0) {
         _elm = *((T**) _elm);
         ++countElm;
      }
      return countElm;
      //
   }

   // Data members
   size_t              _arrSize;   // the array size of the recycled data
   T*                  _first;     // the first recycled data
   MemRecycleList<T>*  _nextList;  // next MemRecycleList
                                   //      with _arrSize + x*R_SIZE
};

template <class T>
class MemMgr
{
   #define S sizeof(T)

public:
   MemMgr(size_t b = 65536) : _blockSize(b) {
      assert(b % SIZE_T == 0);
      _activeBlock = new MemBlock<T>(0, _blockSize);
      for (int i = 0; i < R_SIZE; ++i)
         _recycleList[i]._arrSize = i;
   }
   ~MemMgr() { reset(); delete _activeBlock; }

   // 1. Remove the memory of all but the firstly allocated MemBlocks
   //    That is, the last MemBlock searchd from _activeBlock.
   //    reset its _ptr = _begin (by calling MemBlock::reset())
   // 2. reset _recycleList[]
   // 3. 'b' is the new _blockSize; "b = 0" means _blockSize does not change
   //    if (b != _blockSize) reallocate the memory for the first MemBlock
   // 4. Update the _activeBlock pointer
   void reset(size_t b = 0) {
      assert(b % SIZE_T == 0);
      #ifdef MEM_DEBUG
      cout << "Resetting memMgr...(" << b << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      // Reset _recycleList[]
      int i = 0;
      while (i < R_SIZE) {
         MemRecycleList<T>* ll = getMemRecycleList(i);
         ++i;
         #ifdef MEM_DEBUG
         cerr << "RecycleList: " << ll << endl;
         #endif // MEM_DEBUG
         while (ll != 0) {
            ll->reset();
            #ifdef MEM_DEBUG
            cerr << "RecycleList: " << ll << endl;
            cerr << "ArrSize " << ll->_arrSize << endl;
            #endif // MEM_DEBUG
            if (ll->_arrSize >= R_SIZE) {
               MemRecycleList<T>* toDelete = ll;
               ll = ll->_nextList;
               delete toDelete;
            }  else  {
               MemRecycleList<T>* toReset = ll;
               ll = ll->_nextList;
               toReset->_nextList = 0;               
            }
         }
      }
      #ifdef MEM_DEBUG
      cerr << "Reset _recycleList[] done " << endl;
      #endif // MEM_DEBUG
      // Remove the memory of all but the firstly allocated MemBlocks
      while (getNumBlocks() > 1) {
         _activeBlock->reset();
         MemBlock<T>* tmp = _activeBlock;
         _activeBlock = (*_activeBlock)._nextBlock;
         delete tmp;
         #ifdef MEM_DEBUG
         if (getNumBlocks() == 1) {
            cerr << "_activeBlock " << _activeBlock << endl;
            cerr << "Number of Blocks: " << getNumBlocks() << endl;
         }         
         #endif // MEM_DEBUG
      }
      if (b == 0 || b == _blockSize) {
         _activeBlock->reset();
      }  else {
         _activeBlock->reset();
         #ifdef MEM_DEBUG
         cerr << "_activeBlock " << _activeBlock << endl;
         #endif // MEM_DEBUG
         delete _activeBlock;
         #ifdef MEM_DEBUG
         cerr << "_activeBlock " << _activeBlock << endl;
         #endif // MEM_DEBUG
         _blockSize = b;
         _activeBlock = new MemBlock<T>(0, _blockSize);
         #ifdef MEM_DEBUG
         cerr << "New _activeBlock " << _activeBlock << endl;
         #endif // MEM_DEBUG
      }
   }
   // Called by new
   T* alloc(size_t t) {
      assert(t == S);
      #ifdef MEM_DEBUG
      cout << "Calling alloc...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      return getMem(t);
   }
   // Called by new[]
   T* allocArr(size_t t) {
      #ifdef MEM_DEBUG
      cout << "Calling allocArr...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // Note: no need to record the size of the array == > system will do
      return getMem(t);
   }
   // Called by delete
   void  free(T* p) {
      #ifdef MEM_DEBUG
      cout << "Calling free...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      getMemRecycleList(0)->pushFront(p);
   }
   // Called by delete[]
   void  freeArr(T* p) {
      #ifdef MEM_DEBUG
      cout << "Calling freeArr...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      // Get the array size 'n' stored by system,
      // which is also the _recycleList index
      //
      size_t n = *((size_t*) p);
      //
      #ifdef MEM_DEBUG
      cout << ">> Array size = " << n << endl;
      cout << "Recycling " << p << " to _recycleList[" << n << "]" << endl;
      #endif // MEM_DEBUG
      // add to recycle list...
      getMemRecycleList(n)->pushFront(p);
   }
   void print() const {
      cout << "=========================================" << endl
           << "=              Memory Manager           =" << endl
           << "=========================================" << endl
           << "* Block size            : " << _blockSize << " Bytes" << endl
           << "* Number of blocks      : " << getNumBlocks() << endl
           << "* Free mem in last block: " << _activeBlock->getRemainSize()
           << endl
           << "* Recycle list          : " << endl;
      int i = 0, count = 0;
      while (i < R_SIZE) {
         const MemRecycleList<T>* ll = &(_recycleList[i]);
         while (ll != 0) {
            size_t s = ll->numElm();
            if (s) {
               cout << "[" << setw(3) << right << ll->_arrSize << "] = "
                    << setw(10) << left << s;
               if (++count % 4 == 0) cout << endl;
            }
            ll = ll->_nextList;
         }
         ++i;
      }
      cout << endl;
   }

private:
   size_t                     _blockSize;
   MemBlock<T>*               _activeBlock;
   MemRecycleList<T>          _recycleList[R_SIZE];

   // Private member functions
   //
   // t: #Bytes; MUST be a multiple of SIZE_T
   // return the size of the array with respect to memory size t
   // [Note] t must >= S
   // [NOTE] Use this function in (at least) getMem() to get the size of array
   //        and call getMemRecycleList() later to get the index for
   //        the _recycleList[]
   size_t getArraySize(size_t t) const {
      assert(t % SIZE_T == 0);
      assert(t >= S);
      // TODO
      return (t-SIZE_T)/S; // Done?
   }
   // Go through _recycleList[m], its _nextList, and _nexList->_nextList, etc,
   //    to find a recycle list whose "_arrSize" == "n"
   // If not found, create a new MemRecycleList with _arrSize = n
   //    and add to the last MemRecycleList
   // So, should never return NULL
   // [Note]: This function will be called by MemMgr->getMem() to get the
   //         recycle list. Therefore, the recycle list is first created
   //         by the MTNew command, not MTDelete.
   MemRecycleList<T>* getMemRecycleList(size_t n) {
      size_t m = n % R_SIZE;
      // TODO
      //
      MemRecycleList<T>* currentList = &_recycleList[m];
      while (n != currentList->_arrSize) {
         if (currentList->_nextList == 0) {
            MemRecycleList<T>* myNextList = new MemRecycleList<T>(n);
            currentList->_nextList = myNextList;
            currentList = currentList->_nextList;
         }  else  currentList = currentList->_nextList;
      }
      return currentList;
      // Maybe it's done
   }
   // t is the #Bytes requested from new or new[]
   // Note: Make sure the returned memory is a multiple of SIZE_T
   T* getMem(size_t t) {
      T* ret = 0;
      #ifdef MEM_DEBUG
      cout << "Calling MemMgr::getMem...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // 1. Make sure to promote t to a multiple of SIZE_T
      t = toSizeT(t);
      // 2. Check if the requested memory is greater than the block size.
      //    If so, throw a "bad_alloc()" exception.
      //    Print this message for exception
      //    cerr << "Requested memory (" << t << ") is greater than block size"
      //         << "(" << _blockSize << "). " << "Exception raised...\n";
      // TODO
      // Newly add
      if (t > _blockSize) {
         cerr << "Requested memory (" << t << ") is greater than block size"
         << "(" << _blockSize << "). " << "Exception raised...\n";
         throw bad_alloc();
      }
      //
      // 3. Check the _recycleList first...
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
      //    #endif // MEM_DEBUG
      //    => 'n' is the size of array
      //    => "ret" is the return address
      size_t n = getArraySize(t);
      // TODO
      // Newly add
      MemRecycleList<T>* myList = getMemRecycleList(n);
      if (myList->numElm() != 0) {
         ret = myList->popFront(); // Maybe
         #ifdef MEM_DEBUG
         cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
         #endif // MEM_DEBUG
         return ret;
      }
      //
      // If no match from recycle list...
      // 4. Get the memory from _activeBlock
      // 5. If not enough, recycle the remained memory and print out ---
      //    Note: recycle to the as biggest array index as possible
      //    Note: rn is the array size
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
      //    #endif // MEM_DEBUG
      //    ==> allocate a new memory block, and print out ---
      //    #ifdef MEM_DEBUG
      //    cout << "New MemBlock... " << _activeBlock << endl;
      //    #endif // MEM_DEBUG
      // TODO
      //
      if(!_activeBlock->getMem(t, ret)) {
         size_t rn = _activeBlock->getRemainSize();
         if (rn >= S) {
            rn = getArraySize(downtoSizeT(rn));
            MemRecycleList<T>* remainList = getMemRecycleList(rn);
            remainList->pushFront(ret);
            #ifdef MEM_DEBUG
            cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
            #endif // MEM_DEBUG
         }         
         MemBlock<T>* myNewBlock = new MemBlock<T>(_activeBlock, _blockSize);
         _activeBlock = myNewBlock;
         #ifdef MEM_DEBUG
         cout << "New MemBlock... " << _activeBlock << endl;
         #endif // MEM_DEBUG
         _activeBlock->getMem(t, ret);
         return ret;
      }
      //
      // 6. At the end, print out the acquired memory address
      #ifdef MEM_DEBUG
      cout << "Memory acquired... " << ret << endl;
      #endif // MEM_DEBUG
      return ret;
   }
   // Get the currently allocated number of MemBlock's
   size_t getNumBlocks() const {
      // TODO
      //
      size_t numOfBlocks = 1;
      MemBlock<T>* currentBlock = _activeBlock;
      while (currentBlock->_nextBlock != 0) {
         numOfBlocks += 1;
         currentBlock = currentBlock->_nextBlock;
      }
      return numOfBlocks;
      //
   }

};

#endif // MEM_MGR_H
