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
   friend class BSTree<T>::iterator;

   BSTreeNode(const T& d, BSTreeNode<T>* p = 0, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0) :
      _data(d), _parent(p), _left(l), _right(r) {}

   T _data;
   BSTreeNode<T>* _parent;
   BSTreeNode<T>* _left;
   BSTreeNode<T>* _right;
};


template <class T>
class BSTree
{
   // TODO: design your own class!!
public:
   BSTree() {
      _root = new BSTreeNode<T>(T());
      _root->_parent = _root; // _root is a dummy node
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

iterator begin() const { return iterator(findMin(_root)); }
iterator end() const { return iterator(_root->_parent); }
bool empty() const {
   return (_root->_parent == _root);
}

size_t size() const {
   return _size;
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

BSTreeNode<T>* findMin(BSTreeNode<T>* start) const {
   if (empty()) { return 0; }
   BSTreeNode<T>* goingLeft = start;
   while (goingLeft->_left != _root->_parent) {
      goingLeft = goingLeft->_left;
   }
   return goingLeft;
}

BSTreeNode<T>* findMax(BSTreeNode<T>* start) const {
   if (empty()) { return 0; }
   BSTreeNode<T>* goingRight = start;
   while (goingRight->_right != _root->_parent) {
      goingRight = goingRight->_right;
   }
   return goingRight;
}

}

void sort() const {}

void print() const {
   string myBlank = "";
   print(_root, myBlank);
}

void print(BSTreeNode<T>* myRoot, string myBlank) const {
   cout << myBlank << myRoot->_data << endl;
   myBlank = myBlank + "  ";
   if (myRoot->_left != _root->_parent) { print(myRoot->_left, myBlank); }
   else { cout << myBlank << "[0]" << endl; }
   if (myRoot->_right != _root->_parent) { print(myRoot->_right, myBlank); }
   else { cout << myBlank << "[0]" << endl; }
}

private:
   BSTreeNode<T>* _root;
   size_t _size;
};

#endif // BST_H
