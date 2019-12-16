/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const { return _data[0]; }
   void insert(const Data& d) {
      _data.push_back(d);
      int idx = _data.size() - 1;
      int parentIdx = (idx-1) / 2;
      while (true) {
         if (idx == 0) {
            return;
         }
         if (_data[idx] < _data[parentIdx]) {
            // Swap data with parent
            Data tmp = _data[idx];
            _data[idx] = _data[parentIdx];
            _data[parentIdx] = tmp;
            // Continue comparing
            idx = parentIdx;
            parentIdx = (parentIdx-1) / 2;
         } else {
            return;
         }
      }
   }
   void delMin() {
      // Revise from the pseudo code
      // Not quite understand this ><
      size_t p = 0, t = 2*(p+1);
      size_t n = _data.size();
      while (t <= n) {
         if (t < n) {
            if (_data[t-1] < _data[t]) {
               --t;
            }
            if (_data[n-1] < _data[t]) { break; }
            _data[p] = _data[t];
            p = t;
            t = 2*(p+1);
         } else { break; }         
      }
      _data[p] = _data[n-1];
      _data.pop_back();
   }
   void delData(size_t i) {
      // Revise from the pseudo code
      // Not quite understand this ><
      size_t p = i, t = 2*(p+1);
      size_t n = _data.size();
      while (t <= n) {
         if (t < n) {
            if (_data[t-1] < _data[t]) {
               --t;
            }
         }
         if (_data[n-1] < _data[t]) { break; }
         _data[p] = _data[t];
         p = t;
         t = 2*p;
      }
      _data[p] = _data[t];
      _data.pop_back();
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
