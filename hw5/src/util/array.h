/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0), _isSorted(0) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return (*_node); }
      T& operator * () { return (*_node); }
      iterator& operator ++ () { _node += 1; return (*this); }
      iterator operator ++ (int) { iterator tmp = (*this);  _node += 1; return tmp; }
      iterator& operator -- () { _node -= 1; return (*this); }
      iterator operator -- (int) { iterator tmp = (*this);  _node -= 1; return tmp; }

      iterator operator + (int i) const { return iterator(_node + i); }
      iterator& operator += (int i) { _node += i; return (*this); }

      iterator& operator = (const iterator& i) { return i; }

      bool operator != (const iterator& i) const {
         return (_node != i._node);
      }
      bool operator == (const iterator& i) const {
         return (_node == i._node);
      }
      // Need to handle error from i?
   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_data); }
   iterator end() const { return iterator(&_data[_size-1]+1); }
   bool empty() const {
      if (_size == 0) { return true; }
      else return false;
   }
   size_t size() const { return _size; }

   T& operator [] (size_t i) {
      if (i >= 0 && i < _size)   return *(_data+i);
      else  return 0;
   }
   const T& operator [] (size_t i) const {
      if (i >= 0 && i < _size)   return *(_data+i);
      else  return 0;
   }

   void push_back(const T& x) {
      if (_capacity == 0) {
         _capacity = 1;
         T* newArr = new T[_capacity];
         _data = newArr;
      }  else if (_size == _capacity) {
         _capacity *= 2;
         T* newArr = new T[_capacity];
         for (size_t i=0; i<_size; i++) {
            *(newArr+i) = *(_data+i);
         }
         T* tmp = _data;
         _data = newArr;
         delete []tmp;
      }
      _data[_size++] = x;
   }

   void pop_front() {
      if (_size == 0) { return; }
      *(_data) = *(_data+_size-1);
      *(_data+_size-1) = T();
      --_size;
      // What if _size == 1?
   }
   void pop_back() {
      if (_size == 0) { return; }
      *(_data+_size-1) = T();
      --_size;
   }

   bool erase(iterator pos) {
      if (_size == 0) { return false; }
      *pos = *(_data+_size-1);
      *(_data+_size-1) = T();
      --_size;
      return true;
   }
   bool erase(const T& x) {
      if (_size == 0) { return false; }
      for (size_t i=0; i<_size; i++) {
         if (x == *(_data+i)) {
            *(_data+i) = *(_data+_size-1);
            *(_data+_size-1) = T();
            --_size;
            return true;
         }
      }
      return false;
   }

   iterator find(const T& x) {
      for (size_t i=0; i<_size; i++) {
         if (x == *(_data+i)) {
            return iterator(_data+i);
         }
      }
      return end();
   }

   void clear() {
      if (_capacity > 0) {
         delete []_data;
         _capacity = 0;
      }
      _size = 0;
      _isSorted = 0;
      _data = 0;
   }

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const { if (!empty()) ::sort(_data, _data+_size); }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H
