#pragma once

#include <iostream>
#include <utility>

enum Color { RED, BLACK };

template <typename K, typename V>
struct Node {
    K key;
    V value;
    Node<K, V>* left;
    Node<K, V>* right;
    Node<K, V>* parent;
    Color color;

    Node() : key(0), value(0), left(nullptr), right(nullptr), parent(nullptr), color(Color::RED) {
    }
    Node(const K& key, const V& value)
        : key(key),
          value(value),
          left(nullptr),
          right(nullptr),
          parent(nullptr),
          color(Color::RED) {
    }
    Node(const K& key, const V& value, Color col)
        : key(key), value(value), left(nullptr), right(nullptr), parent(nullptr), color(col) {
    }

    Node<K, V>* getAbsoluteRight() {
        auto node = this;
        while (node->parent) {
            node = node->parent;
        }
        while (node->right) {
            node = node->right;
        }
        return node;
    }
};

template <typename K, typename V>
class Map {
public:
    struct Iterator {
        Iterator() {
        }
        explicit Iterator(Node<K, V>* node) : node_(node), end_(false) {
            updateRes();
        }
        Iterator(Node<K, V>* node, bool is_end) : node_(node), end_(is_end) {
            updateRes();
        }

        const std::pair<K, V>& operator*() const {
            return res_;
        }
        const std::pair<K, V>* operator->() const {
            return &res_;
        }

        Iterator& operator++() {
            if (node_ == node_->getAbsoluteRight()) {
                end_ = true;
                return *this;
            }
            node_ = getRight(node_);
            updateRes();
            return *this;
        }
        Iterator operator++(int) {
            auto b = node_;
            if (node_ == node_->getAbsoluteRight()) {
                end_ = true;
            } else {
                node_ = getRight(node_);
                updateRes();
            }
            return Iterator(b);
        }

        Iterator& operator--() {
            if (end_) {
                end_ = false;
                return *this;
            }
            node_ = getLeft(node_);
            updateRes();
            return *this;
        }
        Iterator operator--(int) {
            auto b = node_;
            if (end_) {
                end_ = false;
            } else {
                node_ = getLeft(node_);
                updateRes();
            }
            return Iterator(b);
        }

        bool operator==(const Iterator& other) const {
            return node_ == other.node_ && end_ == other.end_;
        }
        bool operator!=(const Iterator& other) const {
            return node_ != other.node_ || end_ != other.end_;
        }

    private:
        Node<K, V>* node_{};
        bool end_ = false;
        std::pair<K, V> res_;

        void updateRes() {
            res_.first = node_->key;
            res_.second = node_->value;
        }
    };

    Map() {
    }
    Map(std::initializer_list<std::pair<K, V>> list) {
        for (auto u : list) {
            insert(u.first, u.second);
        }
    }
    Map(const Map& other) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(it->first, it->second);
        }
    }

    Map<K, V>& operator=(const Map& other) {
        clear(root);
        root = nullptr;
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(it->first, it->second);
        }
        return *this;
    }

    ~Map() {
        clear(root);
    }

    void insert(const K& key, const V& value) {
        if (root == nullptr) {
            root = new Node<K, V>(key, value, Color::BLACK);
            ++sz_;
            return;
        }
        Node<K, V>* p = root;
        Node<K, V>* q = nullptr;
        while (p != nullptr) {
            q = p;
            if (p->key < key) {
                p = p->right;
            } else if (key < p->key) {
                p = p->left;
            } else {
                p->value = value;
                return;
            }
        }
        ++sz_;
        auto* t = new Node<K, V>(key, value, Color::RED);
        t->parent = q;
        if (q->key < key) {
            q->right = t;
        } else {
            q->left = t;
        }
        balanceTree(t);
    }

    void erase(const K& key) {
        auto cur = root;
        while (cur) {
            if (cur->key < key) {
                cur = cur->right;
            } else if (key < cur->key) {
                cur = cur->left;
            } else {
                --sz_;
                auto del = cur;
                if (del->left && del->right) {
                    del = del->left;
                    while (del->right) {
                        del = del->right;
                    }
                    cur->key = del->key;
                }
                remove(del);
                break;
            }
        }
    }

    size_t size() const {
        return sz_;
    }
    bool empty() const {
        return sz_ == 0;
    }

    Iterator lowerBound(const K& key) const {
        auto res = lowerBound(root, key);
        if (!res) {
            return end();
        }
        return Iterator(res);
    }
    Iterator find(const K& key) const {
        return find(root, key);
    }

    Iterator begin() const {
        Node<K, V>* node = root;
        while (node && node->left) {
            node = node->left;
        }
        return Iterator(node);
    }
    Iterator end() const {
        Node<K, V>* node = root;
        while (node && node->right) {
            node = node->right;
        }
        return Iterator(node, true);
    }

private:
    int sz_ = 0;
    Node<K, V>* root{};

    void clear(Node<K, V>* node) {
        if (!node) {
            return;
        }
        clear(node->left);
        clear(node->right);
        delete node;
    }

    void leftTurn(Node<K, V>* node) {
        Node<K, V>* b = node->right;
        if (b->left) {
            node->right = b->left;
            b->left->parent = node;
        } else {
            node->right = nullptr;
        }
        b->left = node;
        if (root == node) {
            root = b;
        }
        if (node->parent) {
            if (node->parent->left == node) {
                node->parent->left = b;
            } else {
                node->parent->right = b;
            }
            b->parent = node->parent;
        } else {
            b->parent = nullptr;
        }
        node->parent = b;
    }

    void rightTurn(Node<K, V>* node) {
        Node<K, V>* b = node->left;
        if (b->right) {
            node->left = b->right;
            b->right->parent = node;
        } else {
            node->left = nullptr;
        }
        b->right = node;
        if (node == root) {
            root = b;
        }

        if (node->parent) {
            if (node->parent->left == node) {
                node->parent->left = b;
            } else {
                node->parent->right = b;
            }
            b->parent = node->parent;
        } else {
            b->parent = nullptr;
        }
        node->parent = b;
    }

    void balanceTree(Node<K, V>* node) {
        if (!node) {
            return;
        }
        while (node->parent && node->parent->parent && node->parent->color == Color::RED) {
            auto par = node->parent;
            auto grandad = par->parent;
            if (grandad->left == par) {
                auto uncle = grandad->right;
                if (uncle && uncle->color == Color::RED) {
                    par->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    grandad->color = Color::RED;
                    node = grandad;
                } else {
                    if (par->right == node) {
                        node = par;
                        leftTurn(par);
                        par = par->parent;
                    }
                    par->color = Color::BLACK;
                    grandad->color = Color::RED;
                    rightTurn(grandad);
                }
            } else {
                auto uncle = grandad->left;
                if (uncle && uncle->color == Color::RED) {
                    par->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    grandad->color = Color::RED;
                    node = grandad;
                } else {
                    if (par->left == node) {
                        node = par;
                        rightTurn(node);
                        par = par->parent;
                    }
                    par->color = Color::BLACK;
                    grandad->color = Color::RED;
                    leftTurn(grandad);
                }
            }
        }
        root->color = Color::BLACK;
    }

    static Node<K, V>* getRight(Node<K, V>* node) {
        Node<K, V>* b = nullptr;
        Node<K, V>* q = node;
        if (node->right) {
            b = node->right;
            while (b->left) {
                b = b->left;
            }
        } else {
            while (q->parent) {
                if (q->key < q->parent->key) {
                    b = q->parent;
                    break;
                }
                q = q->parent;
            }
        }
        return b;
    }

    static Node<K, V>* getLeft(Node<K, V>* node) {
        Node<K, V>* b = nullptr;
        Node<K, V>* q = node;
        if (node->left) {
            b = node->left;
            while (b->right) {
                b = b->right;
            }
        } else {
            while (q->parent) {
                if (q->parent->key < q->key) {
                    b = q->parent;
                    break;
                }
                q = q->parent;
            }
        }
        return b;
    }

    bool isBlack(Node<K, V>* node) {
        return !node || node->color == Color::BLACK;
    }

    void removeCorrect(Node<K, V>* node, bool in_left) {
        if (in_left) {
            if (node->right->color == Color::RED) {
                leftTurn(node);
                node->color = Color::RED;
                node->parent->color = Color::BLACK;
                if (node == root) {
                    root = node->parent;
                }
                removeCorrect(node, in_left);
                return;
            }

            if (isBlack(node->right->left) && isBlack(node->right->right)) {
                node->right->color = Color::RED;
                if (node->color == Color::RED) {
                    node->color = Color::BLACK;
                    return;
                }
                if (!node->parent) {
                    return;
                }

                removeCorrect(node->parent, (node->parent->left == node));
                return;
            }

            if (!isBlack(node->right->left) && isBlack(node->right->right)) {
                node->right->color = Color::RED;
                node->right->left->color = Color::BLACK;

                rightTurn(node->right);
                removeCorrect(node, in_left);
                return;
            }

            node->right->color = node->color;
            node->color = Color::BLACK;
            node->right->right->color = Color::BLACK;
            leftTurn(node);
            if (node == root) {
                root = node->parent;
            }
            return;
        }

        if (node->left->color == Color::RED) {
            rightTurn(node);
            node->color = Color::RED;
            node->parent->color = Color::BLACK;
            if (node == root) {
                root = node->parent;
            }
            removeCorrect(node, in_left);
            return;
        }

        if (isBlack(node->left->left) && isBlack(node->left->right)) {
            node->left->color = Color::RED;
            if (node->color == Color::RED) {
                node->color = Color::BLACK;
                return;
            }
            if (!node->parent) {
                return;
            }

            removeCorrect(node->parent, (node->parent->left == node));
            return;
        }

        if (!isBlack(node->left->right) && isBlack(node->left->left)) {
            node->left->color = Color::RED;
            node->left->right->color = Color::BLACK;

            leftTurn(node->left);
            removeCorrect(node, in_left);
            return;
        }

        node->left->color = node->color;
        node->color = Color::BLACK;
        node->left->left->color = Color::BLACK;
        rightTurn(node);
        if (node == root) {
            root = node->parent;
        }
    }

    void remove(Node<K, V>* node) {
        if (node->color == Color::RED) {
            if (node->parent->left == node) {
                node->parent->left = nullptr;
            } else {
                node->parent->right = nullptr;
            }
            delete node;
            return;
        }

        if (node->left) {
            node->key = node->left->key;
            remove(node->left);
            return;
        } else if (node->right) {
            node->key = node->right->key;
            remove(node->right);
            return;
        }
        if (node == root) {
            delete root;
            root = nullptr;
            return;
        }
        auto par = node->parent;
        bool in_left = (par->left == node);
        if (in_left) {
            par->left = nullptr;
        } else {
            par->right = nullptr;
        }
        delete node;
        removeCorrect(par, in_left);
    }

    Iterator find(Node<K, V>* node, const K& val) const {
        if (!node) {
            return end();
        }
        if (val < node->key) {
            return find(node->left, val);
        } else if (node->key < val) {
            return find(node->right, val);
        } else {
            return Iterator(node);
        }
    }

    Node<K, V>* lowerBound(Node<K, V>* node, const K& key) const {
        if (!node) {
            return nullptr;
        }
        Node<K, V>* res;
        if (node->key < key) {
            res = lowerBound(node->right, key);
        } else {
            res = lowerBound(node->left, key);
            if (res == nullptr || node->key < res->key) {
                res = node;
            }
        }
        return res;
    }
};