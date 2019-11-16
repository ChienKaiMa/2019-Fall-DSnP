/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
      _isSorted = 0;
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () { _node = _node->_next; return (*this); }
      iterator operator ++ (int) { iterator tmp = (*this);  _node = _node->_next; return tmp; }
      iterator& operator -- () { _node = _node->_prev; return (*this); }
      iterator operator -- (int) { iterator tmp = (*this);  _node = _node->_prev; return tmp; }

      iterator& operator = (const iterator& i) { return i; }

      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_head); }
   iterator end() const { return iterator(_head->_prev); }
   bool empty() const {
      if (_head == _head->_next && _head == _head->_prev) {
         return true;
      }  else  return false;      
   }
   size_t size() const {
      size_t a = 0;
      if (empty()) { return 0; }
      for (iterator i = begin(); i != end(); i++) {
         ++a;
      }
      return a;
   }
   void push_back(const T& x) {
      DListNode<T>* d = new DListNode<T>(x);
      if (empty()) {
         DListNode<T>* dummy = _head;
         dummy->_next = d;
         d->_next = dummy;
         dummy->_prev = d;
         d->_prev = dummy;
         _head = d;
      } else {
         DListNode<T>* dummy = _head->_prev;
         DListNode<T>* tmpEnd = dummy->_prev;
         tmpEnd->_next = d;
         d->_prev = tmpEnd;
         dummy->_prev = d;
         d->_next = dummy;
      }
   }
   void pop_front() {
      if (empty()) { return; }
      DListNode<T>* dummy = _head->_prev;
      DListNode<T>* tmpHead = _head;
      DListNode<T>* newHead = _head->_next;
      dummy->_next = newHead;
      newHead->_prev = dummy;
      _head = newHead;
      delete tmpHead;
   }
   void pop_back() {
      if (empty()) { return; }
      if (_head == _head->_prev->_prev) {
         pop_front();
         return;
      }
      DListNode<T>* dummy = _head->_prev;
      DListNode<T>* tmpEnd = dummy->_prev;
      DListNode<T>* newEnd = tmpEnd->_prev;
      dummy->_prev = newEnd;
      newEnd->_next = dummy;
      delete tmpEnd;
   }

   // return false if nothing to erase
   bool erase(iterator pos) {
      if (empty() || pos == end()) {
         return false;
      }  else if (pos == iterator(_head)) {
         pop_front();
         return true;
      }  else  {
         DListNode<T>* toDelete = (DListNode<T>*)(&(*pos));
         DListNode<T>* prevNode = toDelete->_prev;
         DListNode<T>* nextNode = toDelete->_next;
         nextNode->_prev = prevNode;
         prevNode->_next = nextNode;
         delete toDelete;
         return true;
      }
   }
   bool erase(const T& x) {
      if (empty()) { return false; }
      for (iterator i=begin(); i!=end(); i++) {
         if (*i == x) { return erase(i); }
      }
      return false;
   }

   iterator find(const T& x) {
      if (empty()) { return end(); }
      for (iterator i=begin(); i!=end(); i++) {
         if (*i == x) { return i; }
      }
      return end();
   }

   void clear() {
      //for (iterator i=begin(); i!=end(); i++) {
      while (!empty()) {
         pop_back();
      }
   }  // delete all nodes except for the dummy node

   void sort() const {
      QuickSort(_head, _head->_prev);
   }

   void swapData(DListNode<T>*& a, DListNode<T>*& b) const {
      T tmp = a->_data;
      a->_data = b->_data;
      b->_data = tmp;
   }

   DListNode<T>* myPartition(DListNode<T>* myHead, DListNode<T>* myEnd) const {
      T pivotData = myHead->_data;
      DListNode<T>* pivot = myHead;
      for (iterator it=iterator(myHead->_next); it!=iterator(myEnd); it++) {
         if (*it < pivotData) {
            // cerr << *it << " swap with " << pivotData << endl;
            for (iterator swapIt=it; swapIt!=pivot; swapIt--) {
               swapData(swapIt._node->_prev, swapIt._node);
            }
            pivot = pivot->_next;
         }
      }
      return pivot;
   }

   void QuickSort(DListNode<T>* myHead, DListNode<T>* myEnd) const {
      /*
      cerr << "Never touch this " << _head->_prev << endl;
      cerr << "myHead is " << myHead << " " << myHead->_data << endl;
      */
      if (myHead->_next == _head->_prev) { return; }
      if (myHead != myEnd) {
         DListNode<T>* myPivot = myPartition(myHead, myEnd);
         if (myPivot == myHead) { return; }

         /*
         cerr << "myHead(in) is " << myHead << " " << myHead->_data << endl;
         cerr << "myPivot is " << myPivot << " " << myPivot->_data << endl;         
         cerr << "myEnd is " << myEnd << " " << myEnd->_data << endl;
         cerr << "" << endl;
         cerr << 1 << endl;
         */

         QuickSort(myHead, myPivot);

         // cerr << 2 << endl;
         if (myPivot->_next != _head->_prev) {
            QuickSort(myPivot->_next, myEnd);
         }
      }      
   }



private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
