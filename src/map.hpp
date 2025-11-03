/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
  class const_iterator;
  struct Node; // forward declaration of internal node
  class iterator {
      private:
      // Iterator holds a pointer to node and owning container for validity checks
      Node *nodePtr = nullptr;
      const map *owner = nullptr;
      public:
      friend class map;
      iterator() = default;

      iterator(const iterator &other) = default;

      // internal constructor
      iterator(struct Node *p, const map *o) : nodePtr(p), owner(o) {}

      // iter++
      iterator operator++(int) {
          iterator tmp = *this;
          ++(*this);
          return tmp;
      }

      // ++iter
      iterator &operator++() {
          if (owner == nullptr) throw invalid_iterator();
          if (nodePtr == nullptr) throw invalid_iterator();
          nodePtr = owner->nextNode(nodePtr);
          if (nodePtr == nullptr) {
              // incrementing the last element should yield end(); allowed
          }
          return *this;
      }

      // iter--
      iterator operator--(int) {
          iterator tmp = *this;
          --(*this);
          return tmp;
      }

      // --iter
      iterator &operator--() {
          if (owner == nullptr) throw invalid_iterator();
          if (nodePtr == nullptr) {
              // --end() should move to the last element if exists
              nodePtr = owner->maxNode(owner->root);
              if (nodePtr == nullptr) throw invalid_iterator();
              return *this;
          }
          struct Node *prev = owner->prevNode(nodePtr);
          if (prev == nullptr) throw invalid_iterator();
          nodePtr = prev;
          return *this;
      }

      // dereference
      value_type &operator*() const {
          if (nodePtr == nullptr) throw invalid_iterator();
          return nodePtr->value;
      }

      bool operator==(const iterator &rhs) const {
          return owner == rhs.owner && nodePtr == rhs.nodePtr;
      }

      bool operator==(const const_iterator &rhs) const { return owner == rhs.owner && nodePtr == rhs.nodePtr; }

      bool operator!=(const iterator &rhs) const {
          return !(*this == rhs);
      }

      bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

      value_type *operator->() const noexcept {
          return &(operator*());
      }
   };
   class const_iterator {
       // it should has similar member method as iterator.
       //  and it should be able to construct from an iterator.
      private:
      Node *nodePtr = nullptr;
      const map *owner = nullptr;
      public:
      friend class map;
      const_iterator() = default;

      const_iterator(const const_iterator &other) = default;

      const_iterator(const iterator &other) : nodePtr(other.nodePtr), owner(other.owner) {}

      // internal constructor
      const_iterator(struct Node *p, const map *o) : nodePtr(p), owner(o) {}

      const value_type &operator*() const {
          if (nodePtr == nullptr) throw invalid_iterator();
          return nodePtr->value;
      }

      const value_type *operator->() const noexcept { return &(operator*()); }

      const_iterator operator++(int) {
          const_iterator tmp = *this;
          ++(*this);
          return tmp;
      }

      const_iterator &operator++() {
          if (owner == nullptr) throw invalid_iterator();
          if (nodePtr == nullptr) throw invalid_iterator();
          nodePtr = owner->nextNode(nodePtr);
          return *this;
      }

      const_iterator operator--(int) {
          const_iterator tmp = *this;
          --(*this);
          return tmp;
      }

      const_iterator &operator--() {
          if (owner == nullptr) throw invalid_iterator();
          if (nodePtr == nullptr) {
              nodePtr = owner->maxNode(owner->root);
              if (nodePtr == nullptr) throw invalid_iterator();
              return *this;
          }
          struct Node *prev = owner->prevNode(nodePtr);
          if (prev == nullptr) throw invalid_iterator();
          nodePtr = prev;
          return *this;
      }

      bool operator==(const const_iterator &rhs) const {
          return owner == rhs.owner && nodePtr == rhs.nodePtr;
      }

      bool operator==(const iterator &rhs) const { return owner == rhs.owner && nodePtr == rhs.nodePtr; }

      bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

      bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
   };

  // no out-of-class iterator cross-operators

   /**
  * TODO two constructors
    */
  map() : root(nullptr), nodeCount(0), cmp(Compare()) {}

  map(const map &other) : root(nullptr), nodeCount(0), cmp(other.cmp) {
      root = cloneSubtree(other.root, nullptr);
      nodeCount = other.nodeCount;
  }

   /**
  * TODO assignment operator
    */
  map &operator=(const map &other) {
      if (this == &other) return *this;
      clear();
      cmp = other.cmp;
      root = cloneSubtree(other.root, nullptr);
      nodeCount = other.nodeCount;
      return *this;
  }

   /**
  * TODO Destructors
    */
  ~map() { clear(); }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
  T &at(const Key &key) {
      Node *n = findNode(key);
      if (n == nullptr) throw index_out_of_bound();
      return n->value.second;
  }

  const T &at(const Key &key) const {
      Node *n = findNode(key);
      if (n == nullptr) throw index_out_of_bound();
      return n->value.second;
  }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
  T &operator[](const Key &key) {
      Node *n = findNode(key);
      if (n) return n->value.second;
      value_type v(key, T());
      auto res = insert(v);
      return (*res.first).second;
  }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
  const T &operator[](const Key &key) const {
      Node *n = findNode(key);
      if (n == nullptr) throw index_out_of_bound();
      return n->value.second;
  }

   /**
  * return a iterator to the beginning
    */
  iterator begin() { return iterator(minNode(root), this); }

  const_iterator cbegin() const { return const_iterator(minNode(root), this); }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
  iterator end() { return iterator(nullptr, this); }

  const_iterator cend() const { return const_iterator(nullptr, this); }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
  bool empty() const { return nodeCount == 0; }

   /**
  * returns the number of elements.
    */
  size_t size() const { return nodeCount; }

   /**
  * clears the contents
    */
  void clear() {
      destroySubtree(root);
      root = nullptr;
      nodeCount = 0;
  }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
  pair<iterator, bool> insert(const value_type &value) {
      // If exists, return iterator
      Node *cur = root;
      Node *parent = nullptr;
      bool isLeft = false;
      while (cur) {
          if (keyEq(value.first, cur->value.first)) {
              return pair<iterator, bool>(iterator(cur, this), false);
          }
          parent = cur;
          if (cmp(value.first, cur->value.first)) {
              cur = cur->left;
              isLeft = true;
          } else {
              cur = cur->right;
              isLeft = false;
          }
      }
      Node *node = new Node(value, parent);
      if (parent == nullptr) {
          root = node;
      } else if (isLeft) {
          parent->left = node;
      } else {
          parent->right = node;
      }
      ++nodeCount;
      rebalanceUp(parent);
      return pair<iterator, bool>(iterator(node, this), true);
  }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
  void erase(iterator pos) {
      if (pos.owner != this) throw invalid_iterator();
      Node *target = pos.nodePtr;
      if (target == nullptr) throw invalid_iterator();
      eraseNode(target);
  }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
  size_t count(const Key &key) const { return findNode(key) ? 1 : 0; }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
  iterator find(const Key &key) { return iterator(findNode(key), this); }

  const_iterator find(const Key &key) const { return const_iterator(findNode(key), this); }
  private:
   Node *root;
   size_t nodeCount;
   Compare cmp;

   // helpers
   bool keyEq(const Key &a, const Key &b) const;
   Node *findNode(const Key &key) const;
   Node *minNode(Node *x) const;
   Node *maxNode(Node *x) const;
   Node *nextNode(Node *n) const;
   Node *prevNode(Node *n) const;
   int heightOf(Node *n) const;
   void update(Node *n);
   int balance(Node *n) const;
   void attachToParent(Node *parent, Node *child, bool asLeft);
   Node *rotateLeft(Node *x);
   Node *rotateRight(Node *y);
   void rebalanceAt(Node *n);
   void rebalanceUp(Node *start);
   void destroySubtree(Node *n);
   Node *cloneSubtree(Node *n, Node *parent);
   void transplant(Node *u, Node *v);
   void eraseNode(Node *z);
};

// =================== Implementation details (private) ===================
template<class Key, class T, class Compare>
class map<Key, T, Compare>::Node {
  public:
    value_type value;
    Node *left;
    Node *right;
    Node *parent;
    int height;
    explicit Node(const value_type &v, Node *p) : value(v), left(nullptr), right(nullptr), parent(p), height(1) {}
};

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::minNode(Node *x) const {
    if (x == nullptr) return nullptr;
    while (x->left) x = x->left;
    return x;
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::maxNode(Node *x) const {
    if (x == nullptr) return nullptr;
    while (x->right) x = x->right;
    return x;
}

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::keyEq(const Key &a, const Key &b) const {
    return !cmp(a, b) && !cmp(b, a);
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::findNode(const Key &key) const {
    Node *cur = root;
    while (cur) {
        if (keyEq(key, cur->value.first)) return cur;
        if (cmp(key, cur->value.first)) cur = cur->left; else cur = cur->right;
    }
    return nullptr;
}

template<class Key, class T, class Compare>
int map<Key, T, Compare>::heightOf(Node *n) const { return n ? n->height : 0; }

template<class Key, class T, class Compare>
void map<Key, T, Compare>::update(Node *n) {
    if (!n) return;
    int hl = heightOf(n->left);
    int hr = heightOf(n->right);
    n->height = (hl > hr ? hl : hr) + 1;
}

template<class Key, class T, class Compare>
int map<Key, T, Compare>::balance(Node *n) const {
    return heightOf(n->right) - heightOf(n->left);
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::attachToParent(Node *parent, Node *child, bool asLeft) {
    if (parent == nullptr) {
        root = child;
        if (child) child->parent = nullptr;
    } else {
        if (asLeft) parent->left = child; else parent->right = child;
        if (child) child->parent = parent;
    }
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::rotateLeft(Node *x) {
    Node *y = x->right;
    Node *B = y->left;
    y->left = x;
    x->right = B;
    if (B) B->parent = x;
    Node *p = x->parent;
    y->parent = p;
    x->parent = y;
    if (p == nullptr) root = y; else if (p->left == x) p->left = y; else p->right = y;
    update(x);
    update(y);
    return y;
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::rotateRight(Node *y) {
    Node *x = y->left;
    Node *B = x->right;
    x->right = y;
    y->left = B;
    if (B) B->parent = y;
    Node *p = y->parent;
    x->parent = p;
    y->parent = x;
    if (p == nullptr) root = x; else if (p->left == y) p->left = x; else p->right = x;
    update(y);
    update(x);
    return x;
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::rebalanceAt(Node *n) {
    if (!n) return;
    update(n);
    int bf = balance(n);
    if (bf > 1) {
        if (balance(n->right) < 0) rotateRight(n->right);
        rotateLeft(n);
    } else if (bf < -1) {
        if (balance(n->left) > 0) rotateLeft(n->left);
        rotateRight(n);
    }
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::rebalanceUp(Node *start) {
    Node *cur = start;
    while (cur) {
        rebalanceAt(cur);
        cur = cur->parent;
    }
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::destroySubtree(Node *n) {
    if (!n) return;
    destroySubtree(n->left);
    destroySubtree(n->right);
    delete n;
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::cloneSubtree(Node *n, Node *parent) {
    if (!n) return nullptr;
    Node *m = new Node(n->value, parent);
    m->left = cloneSubtree(n->left, m);
    m->right = cloneSubtree(n->right, m);
    update(m);
    return m;
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::nextNode(Node *n) const {
    if (!n) return nullptr;
    if (n->right) return minNode(n->right);
    Node *p = n->parent;
    while (p && n == p->right) { n = p; p = p->parent; }
    return p;
}

template<class Key, class T, class Compare>
typename map<Key, T, Compare>::Node *
map<Key, T, Compare>::prevNode(Node *n) const {
    if (!n) return nullptr;
    if (n->left) return maxNode(n->left);
    Node *p = n->parent;
    while (p && n == p->left) { n = p; p = p->parent; }
    return p;
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::transplant(Node *u, Node *v) {
    Node *p = u->parent;
    if (p == nullptr) {
        root = v;
    } else if (p->left == u) {
        p->left = v;
    } else {
        p->right = v;
    }
    if (v) v->parent = p;
}

template<class Key, class T, class Compare>
void map<Key, T, Compare>::eraseNode(Node *z) {
    if (!z) return;
    if (z->left == nullptr || z->right == nullptr) {
        Node *child = z->left ? z->left : z->right;
        Node *parent = z->parent;
        transplant(z, child);
        delete z;
        --nodeCount;
        rebalanceUp(parent);
    } else {
        Node *s = minNode(z->right);
        Node *rebalanceStart = nullptr;
        if (s->parent != z) {
            rebalanceStart = s->parent;
            transplant(s, s->right);
            s->right = z->right;
            if (s->right) s->right->parent = s;
        } else {
            rebalanceStart = s; // s is direct child of z
        }
        Node *parent = z->parent;
        transplant(z, s);
        s->left = z->left;
        if (s->left) s->left->parent = s;
        // Update heights bottom-up from where size decreased
        delete z;
        --nodeCount;
        // Heights may be inconsistent; fix starting from rebalanceStart and up
        rebalanceUp(rebalanceStart);
        rebalanceUp(parent);
    }
}


}

#endif