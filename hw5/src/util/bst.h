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
      _root->_parent = _root->_left = _root->_right = _root; // _root is a dummy node
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
      iterator& operator ++ () {
         if (isDummy(_node)) {
            return (*this);
         } else if (!isDummy(_node->_right)) {
            _node = findMin(_node->_right);
            return (*this); 
         } else if (_node == _node->_right->_parent) {
            _node = _node->_right;
            return (*this);
         } else {
            while (true) {
               BSTreeNode<T>* myParent = _node->_parent;
               if (myParent->_left == _node) { _node = myParent; return (*this); }
               else { _node = myParent; }
            }
         }
      }
      iterator operator ++ (int) {
         iterator tmp = *this;
         ++(*this);
         return tmp;
      }
      iterator& operator -- () {
         if (isDummy(_node)) {
            _node = _node->_parent;
            return (*this);
         } else if (!isDummy(_node->_left)) {
            _node = findMax(_node->_left);
            return (*this);
         } else {
            BSTreeNode<T>* tmp = _node;
            while (!isRoot(tmp)) {
               if (tmp->_parent->_data > tmp->_data) {
                  tmp = tmp->_parent;
               } else {
                  _node = tmp->_parent;
                  return (*this);
               }
            }
            tmp = findMax(tmp->_left);
            if (_node == tmp) {
               _node = _node->_left;
            }
            return (*this);
         }
      }
      iterator operator -- (int) {
         iterator tmp = *this;
         --(*this);
         return tmp;
      }

      iterator& operator = (const iterator& i) const { return i; }
      
      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

      BSTreeNode<T>* findMin(BSTreeNode<T>* start) const {
         BSTreeNode<T>* goLeft = start;
         while (!isDummy(goLeft->_left)) {
            goLeft = goLeft->_left;
         }
         return goLeft;
      }
      BSTreeNode<T>* findMax(BSTreeNode<T>* start) const {
         BSTreeNode<T>* goRight = start;
         while (!isDummy(goRight->_right)) {
            goRight = goRight->_right;
         }
         return goRight;
      }

   private:
      BSTreeNode<T>* _node;
      bool isDummy(BSTreeNode<T>* n) const { return (n == n->_left && n == n->_right); }
      bool isRoot(BSTreeNode<T>* n) const { return isDummy(n->_parent); }
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
   if (empty()) {
      BSTreeNode<T>* newNode = new BSTreeNode<T>(x, _root, _root, _root);
      _root = newNode; // Now _root's parent is dummy
      _root->_parent->_parent = _root;
      ++_size;
   } else {
      // Compare to go left or right
      BSTreeNode<T>* toCompare = _root;
      BSTreeNode<T>* dummy = _root->_parent;
      while (true) {
         if (x < toCompare->_data) {
            if (toCompare->_left == dummy) {
               BSTreeNode<T>* newNode = new BSTreeNode<T>(x, toCompare, dummy, dummy);
               toCompare->_left = newNode;
               ++_size;
               dummy->_parent = findMax(_root);
               break;
            } else { toCompare = toCompare->_left; }
         } else {
            if (toCompare->_right == _root->_parent) {
               BSTreeNode<T>* newNode = new BSTreeNode<T>(x, toCompare, dummy, dummy);
               toCompare->_right = newNode;
               ++_size;
               dummy->_parent = findMax(_root);
               break;
            } else { toCompare = toCompare->_right; }
         }
      }
   }
}

void pop_front() {
   if (empty()) { return; }
   BSTreeNode<T>* myFront = findMin(_root);
   if (_size == 1) {
      _root = _root->_parent;
      _root->_parent = _root;
      delete myFront;
      --_size; return; }
   if (myFront == _root) {
      BSTreeNode<T>* dummy = _root->_parent;
      assert(myFront->_left == dummy);
      BSTreeNode<T>* tmp = _root;
      BSTreeNode<T>* myRight = _root->_right;
      _root = myRight;
      _root->_parent = dummy;
      delete tmp;
      --_size;
      return;
   } else if (myFront->_right != _root->_parent) {
      // If there are things inside myFront,
      // Connect myFront's parent and right node
      BSTreeNode<T>* myParent = myFront->_parent;
      BSTreeNode<T>* myRight = myFront->_right;
      myParent->_left = myRight;
      myRight->_parent = myParent;

      delete myFront;
      --_size;

      _root->_parent->_parent = findMax(_root);
      return;
   } else {
      BSTreeNode<T>* myParent = myFront->_parent;
      myParent->_left = _root->_parent;
      _root->_parent->_parent = findMax(_root);
      delete myFront;
      --_size;
      return;
   }
}

void pop_back() {
   if (empty()) { return; }
   BSTreeNode<T>* myBack = _root->_parent->_parent;
   if (_size == 1) {
      _root = _root->_parent;
      _root->_parent = _root;
      delete myBack;
      --_size; return; }
   if (myBack == _root) {
      BSTreeNode<T>* dummy = _root->_parent;
      assert(myBack->_right == dummy);
      BSTreeNode<T>* tmp = _root;
      BSTreeNode<T>* myLeft = _root->_left;
      _root = myLeft;
      _root->_parent = dummy;
      delete tmp;
      --_size;
      _root->_parent->_parent = findMax(_root);
      return;
   } else if (myBack->_left != _root->_parent) {
      // If there are things inside myBack,
      // Connect myBack's parent and left node
      BSTreeNode<T>* myParent = myBack->_parent;
      BSTreeNode<T>* myLeft = myBack->_left;
      myParent->_right = myLeft;
      myLeft->_parent = myParent;

      delete myBack;
      --_size;

      _root->_parent->_parent = findMax(_root);
      return;
   } else {
      BSTreeNode<T>* myParent = myBack->_parent;
      myParent->_right = _root->_parent;
      _root->_parent->_parent = findMax(_root);
      delete myBack;
      --_size;
      return;
   }
}

bool erase(iterator pos) {
   // --_size;
   
   // If root?
   if (find(*pos) == end()) {
      // Not found
      return 0;
   } else if (pos._node->_left == _root->_parent && pos._node->_right == _root->_parent) {
      // No child
      if (pos._node == _root) {
         pop_front();
         return true;
      } else if (pos._node == _root->_parent->_parent) {
         pop_back();
         return true;
      } else if (pos._node == pos._node->_parent->_left) {
         pos._node->_parent->_left = _root->_parent;
         delete pos._node;
         --_size;
         return true;
      } else if (pos._node == pos._node->_parent->_right) {
         pos._node->_parent->_right = _root->_parent;
         delete pos._node;
         --_size;
         return true;
      }
   } else if (pos._node->_left == _root->_parent) {
      // One child
      if (pos._node == _root) {
         pop_front();
         return true;
      } else {
         BSTreeNode<T>* myParent = pos._node->_parent;
         BSTreeNode<T>* myRight = pos._node->_right;
         if (myParent->_left == pos._node) {
            myParent->_left = myRight;
         } else {
            myParent->_right = myRight;
         }
         myRight->_parent = myParent;

         delete pos._node;
         --_size;
         _root->_parent->_parent = findMax(_root);
         return true;
      }
   } else if (pos._node->_right == _root->_parent) {
      // One child on the left
      if (pos._node == _root) {
         pop_back();
         return true;
      } else {
         BSTreeNode<T>* myParent = pos._node->_parent;
         BSTreeNode<T>* myLeft = pos._node->_left;
         if (myParent->_left == pos._node) {
            myParent->_left = myLeft;
         } else {
            myParent->_right = myLeft;
         }
         myLeft->_parent = myParent;

         delete pos._node;
         --_size;
         _root->_parent->_parent = findMax(_root);
         return true;
      }
   } else {
      // Two children
      // Move the data of its successor to it
      // And delete the successor node
      BSTreeNode<T>* successor = findMin(pos._node->_right);
      pos._node->_data = successor->_data;
      erase(iterator(successor));
      return true;
   }
   return 0;
}

bool erase(const T& x) {
   return erase(find(x));
}

iterator find(const T& x) {
   if (empty()) { return iterator(_root); }
   BSTreeNode<T>* tmp = _root;
   while (x != tmp->_data) {
      if (tmp == _root->_parent) { return iterator(_root->_parent); }
      else if (x < tmp->_data) { tmp = tmp->_left; }
      else { tmp = tmp->_right; }
   }
   return iterator(tmp);
}

BSTreeNode<T>* findMin(BSTreeNode<T>* start) const {
   if (empty()) { return _root; }
   BSTreeNode<T>* goLeft = start;
   while (goLeft->_left != _root->_parent) {
      goLeft = goLeft->_left;
   }
   return goLeft;
}

BSTreeNode<T>* findMax(BSTreeNode<T>* start) const {
   if (empty()) { return _root; }
   BSTreeNode<T>* goRight = start;
   while (goRight->_right != _root->_parent) {
      goRight = goRight->_right;
   }
   return goRight;
}

void clear() {
   int t = _size;
   while (t > 0) {
      pop_back();
      --t;
   }
   assert(_size == 0);
}

void sort() const {}

void print() const {
   if (_size == 0) { return; }
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
