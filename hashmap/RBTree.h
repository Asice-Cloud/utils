#pragma once
#include <functional>
using namespace std;

enum class Color { RED, BLACK };

template<typename K, typename V>
struct RBNode {
    K key;
    V value;
    Color color;
    RBNode *left, *right, *parent;

    RBNode(K k, V v) : key(k), value(v), color(Color::RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename K, typename V>
class RBTree {
private:
    RBNode<K, V>* root;
    size_t size;

    void leftRotate(RBNode<K, V>* x);
    void rightRotate(RBNode<K, V>* x);
    void insertFixup(RBNode<K, V>* z);
    void deleteFixup(RBNode<K, V>* x);
    RBNode<K, V>* minimum(RBNode<K, V>* x);
    void transplant(RBNode<K, V>* u, RBNode<K, V>* v);
    void destroyTree(RBNode<K, V>* node);
    void inorderTraversalHelper(RBNode<K, V>* node, std::function<void(const K&, const V&)> func) const;

public:
    RBTree() : root(nullptr), size(0) {}
    ~RBTree() { destroyTree(root); }

    void insert(const K& key, const V& value);
    bool find(const K& key, V& value) const;
    bool remove(const K& key);
    size_t getSize() const { return size; }
    void inorderTraversal(std::function<void(const K&, const V&)> func) const;
};

template<typename K, typename V>
void RBTree<K, V>::leftRotate(RBNode<K, V>* x) {
    RBNode<K, V>* y = x->right;
    x->right = y->left;
    if (y->left != nullptr)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == nullptr)
        root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

template<typename K, typename V>
void RBTree<K, V>::rightRotate(RBNode<K, V>* x) {
    RBNode<K, V>* y = x->left;
    x->left = y->right;
    if (y->right != nullptr)
        y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == nullptr)
        root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x;
    x->parent = y;
}

template<typename K, typename V>
void RBTree<K, V>::insertFixup(RBNode<K, V>* z) {
    while (z->parent != nullptr && z->parent->color == Color::RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode<K, V>* y = z->parent->parent->right;
            if (y != nullptr && y->color == Color::RED) {
                z->parent->color = Color::BLACK;
                y->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(z);
                }
                z->parent->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                rightRotate(z->parent->parent);
            }
        } else {
            RBNode<K, V>* y = z->parent->parent->left;
            if (y != nullptr && y->color == Color::RED) {
                z->parent->color = Color::BLACK;
                y->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(z);
                }
                z->parent->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    root->color = Color::BLACK;
}

template<typename K, typename V>
void RBTree<K, V>::deleteFixup(RBNode<K, V>* x) {
    while (x != root && (x == nullptr || x->color == Color::BLACK)) {
        if (x == x->parent->left) {
            RBNode<K, V>* w = x->parent->right;
            if (w->color == Color::RED) {
                w->color = Color::BLACK;
                x->parent->color = Color::RED;
                leftRotate(x->parent);
                w = x->parent->right;
            }
            if ((w->left == nullptr || w->left->color == Color::BLACK) &&
                (w->right == nullptr || w->right->color == Color::BLACK)) {
                w->color = Color::RED;
                x = x->parent;
            } else {
                if (w->right == nullptr || w->right->color == Color::BLACK) {
                    if (w->left != nullptr) w->left->color = Color::BLACK;
                    w->color = Color::RED;
                    rightRotate(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Color::BLACK;
                if (w->right != nullptr) w->right->color = Color::BLACK;
                leftRotate(x->parent);
                x = root;
            }
        } else {
            RBNode<K, V>* w = x->parent->left;
            if (w->color == Color::RED) {
                w->color = Color::BLACK;
                x->parent->color = Color::RED;
                rightRotate(x->parent);
                w = x->parent->left;
            }
            if ((w->right == nullptr || w->right->color == Color::BLACK) &&
                (w->left == nullptr || w->left->color == Color::BLACK)) {
                w->color = Color::RED;
                x = x->parent;
            } else {
                if (w->left == nullptr || w->left->color == Color::BLACK) {
                    if (w->right != nullptr) w->right->color = Color::BLACK;
                    w->color = Color::RED;
                    leftRotate(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Color::BLACK;
                if (w->left != nullptr) w->left->color = Color::BLACK;
                rightRotate(x->parent);
                x = root;
            }
        }
    }
    if (x != nullptr) x->color = Color::BLACK;
}

template<typename K, typename V>
RBNode<K, V>* RBTree<K, V>::minimum(RBNode<K, V>* x) {
    while (x->left != nullptr)
        x = x->left;
    return x;
}

template<typename K, typename V>
void RBTree<K, V>::transplant(RBNode<K, V>* u, RBNode<K, V>* v) {
    if (u->parent == nullptr)
        root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    if (v != nullptr)
        v->parent = u->parent;
}

template<typename K, typename V>
void RBTree<K, V>::destroyTree(RBNode<K, V>* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

template<typename K, typename V>
void RBTree<K, V>::insert(const K& key, const V& value) {
    RBNode<K, V>* z = new RBNode<K, V>(key, value);
    RBNode<K, V>* y = nullptr;
    RBNode<K, V>* x = root;

    while (x != nullptr) {
        y = x;
        if (z->key < x->key)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    if (y == nullptr)
        root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;

    insertFixup(z);
    size++;
}

template<typename K, typename V>
bool RBTree<K, V>::find(const K& key, V& value) const {
    RBNode<K, V>* x = root;
    while (x != nullptr) {
        if (key == x->key) {
            value = x->value;
            return true;
        } else if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    return false;
}

template<typename K, typename V>
bool RBTree<K, V>::remove(const K& key) {
    RBNode<K, V>* z = root;
    while (z != nullptr) {
        if (key == z->key) {
            break;
        } else if (key < z->key) {
            z = z->left;
        } else {
            z = z->right;
        }
    }
    if (z == nullptr)
        return false;

    RBNode<K, V>* y = z;
    RBNode<K, V>* x;
    Color y_original_color = y->color;

    if (z->left == nullptr) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nullptr) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x != nullptr) x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    delete z;
    if (y_original_color == Color::BLACK)
        deleteFixup(x);
    size--;
    return true;
}

template<typename K, typename V>
void RBTree<K, V>::inorderTraversal(std::function<void(const K&, const V&)> func) const {
    inorderTraversalHelper(root, func);
}

template<typename K, typename V>
void RBTree<K, V>::inorderTraversalHelper(RBNode<K, V>* node, std::function<void(const K&, const V&)> func) const {
    if (node != nullptr) {
        inorderTraversalHelper(node->left, func);
        func(node->key, node->value);
        inorderTraversalHelper(node->right, func);
    }
}