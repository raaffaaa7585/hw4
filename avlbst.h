#ifndef AVLBST_H
#define AVLBST_H

#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "bst.h"

struct KeyError { };

/**
* A special kind of node for an AVL tree, which adds the balance as a data member, plus
* other additional helper functions. You do NOT need to implement any functionality or
* add additional data members or helper functions.
*/
template <typename Key, typename Value>
class AVLNode : public Node<Key, Value>
{
public:
    // Constructor/destructor.
    AVLNode(const Key& key, const Value& value, AVLNode<Key, Value>* parent);
    virtual ~AVLNode();

    // Getter/setter for the node's height.
    int8_t getBalance () const;
    void setBalance (int8_t balance);
    void updateBalance(int8_t diff);

    // Getters for parent, left, and right. These need to be redefined since they
    // return pointers to AVLNodes - not plain Nodes. See the Node class in bst.h
    // for more information.
    virtual AVLNode<Key, Value>* getParent() const override;
    virtual AVLNode<Key, Value>* getLeft() const override;
    virtual AVLNode<Key, Value>* getRight() const override;

protected:
    int8_t balance_;    // effectively a signed char
};

/*
  -------------------------------------------------
  Begin implementations for the AVLNode class.
  -------------------------------------------------
*/

/**
* An explicit constructor to initialize the elements by calling the base class constructor
*/
template<class Key, class Value>
AVLNode<Key, Value>::AVLNode(const Key& key, const Value& value, AVLNode<Key, Value> *parent) :
    Node<Key, Value>(key, value, parent), balance_(0)
{

}

/**
* A destructor which does nothing.
*/
template<class Key, class Value>
AVLNode<Key, Value>::~AVLNode()
{

}

/**
* A getter for the balance of a AVLNode.
*/
template<class Key, class Value>
int8_t AVLNode<Key, Value>::getBalance() const
{
    return balance_;
}

/**
* A setter for the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::setBalance(int8_t balance)
{
    balance_ = balance;
}

/**
* Adds diff to the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::updateBalance(int8_t diff)
{
    balance_ += diff;
}

/**
* An overridden function for getting the parent since a static_cast is necessary to make sure
* that our node is a AVLNode.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getParent() const
{
    return static_cast<AVLNode<Key, Value>*>(this->parent_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getLeft() const
{
    return static_cast<AVLNode<Key, Value>*>(this->left_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getRight() const
{
    return static_cast<AVLNode<Key, Value>*>(this->right_);
}


/*
  -----------------------------------------------
  End implementations for the AVLNode class.
  -----------------------------------------------
*/


template <class Key, class Value>
class AVLTree : public BinarySearchTree<Key, Value>
{
public:
    virtual void insert (const std::pair<const Key, Value> &new_item); // TODO
    virtual void remove(const Key& key);  // TODO
protected:
    virtual void nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2);

    // Add helper functions here
    void rotateLeft(AVLNode<Key, Value>* x);
    void rotateRight(AVLNode<Key, Value>* z);
    void insertFix(AVLNode<Key, Value>* p, AVLNode<Key, Value>* n);
    void removeFix(AVLNode<Key, Value>* n, int8_t diff);

};

/*
 * Recall: If key is already in the tree, you should 
 * overwrite the current value with the updated value.
 */
template<class Key, class Value>
void AVLTree<Key, Value>::insert (const std::pair<const Key, Value> &new_item)
{
    if (this->root_ == NULL) {
        this->root_ = new AVLNode<Key, Value>(new_item.first, new_item.second, NULL);
        return;
    }

    AVLNode<Key, Value>* curr = static_cast<AVLNode<Key, Value>*>(this->root_);
    AVLNode<Key, Value>* parent = NULL;

    while (curr != NULL) {
        parent = curr;
        if (new_item.first < curr->getKey()) {
            curr = curr->getLeft();
        } else if (new_item.first > curr->getKey()) {
            curr = curr->getRight();
        } else {
            // Key already exists; overwrite value
            curr->setValue(new_item.second);
            return;
        }
    }

    AVLNode<Key, Value>* newNode = new AVLNode<Key, Value>(new_item.first, new_item.second, parent);
    if (new_item.first < parent->getKey()) {
        parent->setLeft(newNode);
        if (parent->getBalance() == 1) {
            parent->setBalance(0);
        } else {
            parent->setBalance(-1);
            insertFix(parent, newNode);
        }
    } else {
        parent->setRight(newNode);
        if (parent->getBalance() == -1) {
            parent->setBalance(0);
        } else {
            parent->setBalance(1);
            insertFix(parent, newNode);
        }
    }
}

/*
 * Recall: The writeup specifies that if a node has 2 children you
 * should swap with the predecessor and then remove.
 */
template<class Key, class Value>
void AVLTree<Key, Value>:: remove(const Key& key)
{
    AVLNode<Key, Value>* target = static_cast<AVLNode<Key, Value>*>(this->internalFind(key));
    if (target == NULL) {
        return;
    }

    // If target has 2 children, swap with predecessor
    if (target->getLeft() != NULL && target->getRight() != NULL) {
        AVLNode<Key, Value>* pred = static_cast<AVLNode<Key, Value>*>(BinarySearchTree<Key, Value>::predecessor(target));
        nodeSwap(target, pred);
    }

    AVLNode<Key, Value>* parent = target->getParent();
    AVLNode<Key, Value>* child = (target->getLeft() != NULL) ? target->getLeft() : target->getRight();
    int8_t diff = 0;

    if (parent != NULL) {
        if (parent->getLeft() == target) {
            diff = 1; // Left subtree shrank
        } else {
            diff = -1; // Right subtree shrank
        }
    }

    if (child != NULL) {
        child->setParent(parent);
    }

    if (parent == NULL) {
        this->root_ = child;
    } else if (parent->getLeft() == target) {
        parent->setLeft(child);
    } else {
        parent->setRight(child);
    }

    delete target;

    if (parent != NULL) {
        removeFix(parent, diff);
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::rotateLeft(AVLNode<Key, Value>* x)
{
    AVLNode<Key, Value>* y = x->getRight();
    AVLNode<Key, Value>* p = x->getParent();
    AVLNode<Key, Value>* b = y->getLeft();

    y->setParent(p);
    if (p == NULL) {
        this->root_ = y;
    } else if (p->getLeft() == x) {
        p->setLeft(y);
    } else {
        p->setRight(y);
    }

    y->setLeft(x);
    x->setParent(y);

    x->setRight(b);
    if (b != NULL) {
        b->setParent(x);
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::rotateRight(AVLNode<Key, Value>* z)
{
    AVLNode<Key, Value>* y = z->getLeft();
    AVLNode<Key, Value>* p = z->getParent();
    AVLNode<Key, Value>* b = y->getRight();

    y->setParent(p);
    if (p == NULL) {
        this->root_ = y;
    } else if (p->getLeft() == z) {
        p->setLeft(y);
    } else {
        p->setRight(y);
    }

    y->setRight(z);
    z->setParent(y);

    z->setLeft(b);
    if (b != NULL) {
        b->setParent(z);
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::insertFix(AVLNode<Key, Value>* p, AVLNode<Key, Value>* n)
{
    if (p == NULL || p->getParent() == NULL) {
        return;
    }

    AVLNode<Key, Value>* g = p->getParent();

    if (g->getLeft() == p) {
        g->updateBalance(-1);
        if (g->getBalance() == 0) {
            return;
        } else if (g->getBalance() == -1) {
            insertFix(g, p);
        } else if (g->getBalance() == -2) {
            if (p->getLeft() == n) {
                // Zig-zig (Left-Left)
                rotateRight(g);
                p->setBalance(0);
                g->setBalance(0);
            } else {
                // Zig-zag (Left-Right)
                rotateLeft(p);
                rotateRight(g);
                if (n->getBalance() == -1) {
                    p->setBalance(0);
                    g->setBalance(1);
                } else if (n->getBalance() == 0) {
                    p->setBalance(0);
                    g->setBalance(0);
                } else {
                    p->setBalance(-1);
                    g->setBalance(0);
                }
                n->setBalance(0);
            }
        }
    } else {
        g->updateBalance(1);
        if (g->getBalance() == 0) {
            return;
        } else if (g->getBalance() == 1) {
            insertFix(g, p);
        } else if (g->getBalance() == 2) {
            if (p->getRight() == n) {
                // Zig-zig (Right-Right)
                rotateLeft(g);
                p->setBalance(0);
                g->setBalance(0);
            } else {
                // Zig-zag (Right-Left)
                rotateRight(p);
                rotateLeft(g);
                if (n->getBalance() == 1) {
                    p->setBalance(0);
                    g->setBalance(-1);
                } else if (n->getBalance() == 0) {
                    p->setBalance(0);
                    g->setBalance(0);
                } else {
                    p->setBalance(1);
                    g->setBalance(0);
                }
                n->setBalance(0);
            }
        }
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::removeFix(AVLNode<Key, Value>* n, int8_t diff)
{
    if (n == NULL) {
        return;
    }

    AVLNode<Key, Value>* p = n->getParent();
    int8_t ndiff = 0;
    if (p != NULL) {
        ndiff = (p->getLeft() == n) ? 1 : -1;
    }

    if (diff == -1) {
        if (n->getBalance() + diff == -2) {
            AVLNode<Key, Value>* c = n->getLeft();
            if (c->getBalance() == -1) {
                rotateRight(n);
                n->setBalance(0);
                c->setBalance(0);
                removeFix(p, ndiff);
            } else if (c->getBalance() == 0) {
                rotateRight(n);
                n->setBalance(-1);
                c->setBalance(1);
            } else if (c->getBalance() == 1) {
                AVLNode<Key, Value>* g = c->getRight();
                rotateLeft(c);
                rotateRight(n);
                if (g->getBalance() == 1) {
                    n->setBalance(0);
                    c->setBalance(-1);
                } else if (g->getBalance() == 0) {
                    n->setBalance(0);
                    c->setBalance(0);
                } else {
                    n->setBalance(1);
                    c->setBalance(0);
                }
                g->setBalance(0);
                removeFix(p, ndiff);
            }
        } else if (n->getBalance() + diff == -1) {
            n->setBalance(-1);
        } else if (n->getBalance() + diff == 0) {
            n->setBalance(0);
            removeFix(p, ndiff);
        }
    } else if (diff == 1) {
        if (n->getBalance() + diff == 2) {
            AVLNode<Key, Value>* c = n->getRight();
            if (c->getBalance() == 1) {
                rotateLeft(n);
                n->setBalance(0);
                c->setBalance(0);
                removeFix(p, ndiff);
            } else if (c->getBalance() == 0) {
                rotateLeft(n);
                n->setBalance(1);
                c->setBalance(-1);
            } else if (c->getBalance() == -1) {
                AVLNode<Key, Value>* g = c->getLeft();
                rotateRight(c);
                rotateLeft(n);
                if (g->getBalance() == -1) {
                    n->setBalance(0);
                    c->setBalance(1);
                } else if (g->getBalance() == 0) {
                    n->setBalance(0);
                    c->setBalance(0);
                } else {
                    n->setBalance(-1);
                    c->setBalance(0);
                }
                g->setBalance(0);
                removeFix(p, ndiff);
            }
        } else if (n->getBalance() + diff == 1) {
            n->setBalance(1);
        } else if (n->getBalance() + diff == 0) {
            n->setBalance(0);
            removeFix(p, ndiff);
        }
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2)
{
    BinarySearchTree<Key, Value>::nodeSwap(n1, n2);
    int8_t tempB = n1->getBalance();
    n1->setBalance(n2->getBalance());
    n2->setBalance(tempB);
}


#endif