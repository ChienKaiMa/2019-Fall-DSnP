/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
   public:
      iterator(Data* n = 0, vector<Data>* b = 0, size_t nb = 0, size_t mbn = 0, size_t idx = 0) : _node(n), _buckets(b), _numBuckets(nb), _myBucketNum(mbn), _idxInBucket(idx) {}
      iterator(const iterator& i, vector<Data>* b = 0, size_t nb = 0, size_t mbn = 0, size_t idx = 0) : _node(i._node), _buckets(b), _numBuckets(nb), _myBucketNum(mbn),_idxInBucket(idx) {}
      ~iterator() {}

      const Data& operator * () const { return *_node; }
      iterator& operator ++ () {
         // bucket is end
         // If not last element
         size_t mySize = _buckets[_myBucketNum].size();
         if (mySize != _idxInBucket + 1) {
            // Next element in the bucket
            _node += 1; _idxInBucket += 1; return (*this);
         } else {
            if (_myBucketNum == _numBuckets - 1) {
               // End of all buckets
               _idxInBucket = -1;
               return (*this);
            }
            while (_myBucketNum != _numBuckets - 1) {
               // while not last bucket
               ++_myBucketNum;
               if (_buckets[_myBucketNum].size() != 0) {
                  // if has element
                  _node = &(_buckets[_myBucketNum][0]);
                  _idxInBucket = 0;
                  return (*this);
               }
            }
            // if last bucket
            if (_buckets[_myBucketNum].size() != 0) {
               // if has element
               _node = &(_buckets[_myBucketNum][0]);
               _idxInBucket = 0;
               return (*this);
            } else {
               // End of all buckets
               _idxInBucket = -1;
               return (*this);
            }
         }
      }
      iterator operator ++ (int) { iterator tmp = (*this);  ++(*this); return tmp; }
      iterator& operator -- () { _node -= 1; return (*this); }
      iterator operator -- (int) { iterator tmp = (*this);  --(*this); return tmp; }

      iterator& operator = (const iterator& i) const { return i; }
      bool operator == (const iterator& i) const { return (i._node == _node && i._idxInBucket == _idxInBucket); }
      bool operator != (const iterator& i) const { return !(i._node == _node && i._idxInBucket == _idxInBucket); }
   private:
      Data* _node;
      vector<Data>* _buckets;
      size_t _numBuckets;
      size_t _myBucketNum;
      size_t _idxInBucket;
      size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
      for (size_t i=0; i<_numBuckets; ++i) {
         if (!_buckets[i].empty()) {
            return iterator(&_buckets[i][0], _buckets, _numBuckets, i, 0);
         }
      }
      return iterator();
   }
   // Pass the end
   iterator end() const {
      for (int i=_numBuckets-1; i>=0; --i) {
         if (!_buckets[i].empty()) {
            size_t idx = _buckets[i].size() - 1;
            return iterator(&_buckets[i][idx], _buckets, _numBuckets, _numBuckets-1, -1);
         }
      }
      return iterator();
   }
   // return true if no valid data
   bool empty() const {
      // Done
      for (size_t i=0; i<_numBuckets; ++i) {
         if (!_buckets[i].empty()) {
            return false;
         }
      }
      return true;
   }
   // number of valid data
   size_t size() const {
      // Done
      size_t s = 0;
      for (size_t i=0; i<_numBuckets; ++i) {
         s += (_buckets[i].size());
      }
      return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const {
      // Done
      vector<Data>& myBucket = _buckets[bucketNum(d)];
      for (size_t i=0; i<myBucket.size(); ++i) {
         if (d == myBucket[i]) {
            return true;
         }
      }
      return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const {
      // Done
      vector<Data>& myBucket = _buckets[bucketNum(d)];
      for (size_t i=0; i<myBucket.size(); ++i) {
         if (d == myBucket[i]) {
            d = myBucket[i];
            return true;
         }
      }
      return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) {
      // Done
      vector<Data>& myBucket = _buckets[bucketNum(d)];
      for (size_t i=0; i<myBucket.size(); ++i) {
         if (d == myBucket[i]) {
            myBucket[i] = d;
            return true;
         }
      }
      insert(d);
      return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) {
      // Done
      if (check(d)) {
         return false;
      } else {
         (_buckets[bucketNum(d)]).push_back(d);
         return true;
      }
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) {
      vector<Data>& myBucket = _buckets[bucketNum(d)];
      for (size_t i=0; i<myBucket.size(); ++i) {
         if (d == myBucket[i]) {
            myBucket.erase(myBucket.begin()+i);
            return true;
         }
      }
      return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
