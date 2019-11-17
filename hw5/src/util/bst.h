/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree:iterator;

   BSTreeNode(const T& d, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0):
      _data(d), _left(l), _right(r) {}

   T _data;
   BSTreeNode<T>* _left;
   BSTreeNode<T>* _right;
};


template <class T>
class BSTree
{
   // TODO: design your own class!!
   BSTree() {
      _root = new BSTreeNode<T>(T());
      _root->_left = _root->_right = _root;
   }
   ~BSTree() { clear(); delete _root; }

   class iterator
   {
      friend class BSTree;

   public:
      iterator(BSTreeNode<T>* n = 0) : _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {}

      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {}
      iterator operator ++ (int) {}
      iterator& operator -- () {}
      iterator operator -- (int) {}

      iterator& operator = (const iterator& i) const { return i; }
      
      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      BSTreeNode<T>* _node;
   };

iterator begin() const {}
iterator end() const {}
bool empty() const {
   return (_root->_left == _root && _root->_right == _root);
}

size_t size() const {

}

void insert(const T& x) {

}

void pop_front() {

}

void pop_back() {

}

bool erase(iterator pos) {

}

bool erase(const T& x) {

}

iterator find(const T& x) {

}

void clear() {}

void sort() const {}
void print() const {}

private:
   BSTreeNode<T>* _root;
};

#endif // BST_H
