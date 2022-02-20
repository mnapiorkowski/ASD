#include <iostream>
#include <string>
using namespace std;

struct Node {
    char key;
    int weight; // number of descendants of the node (including itself)
    Node* child[2]; // pointers to children of the node
    Node* parent; // pointer to parent of the node
    bool reversed; // if true, all children in the subtree should be treated as swapped
    int longest; // maximum length of consistent substring in subtree, comprising same character
    char neighbour[2]; // previous and next character in string represented by the tree this node is part of
    int end_len[2]; // length of consistent substring in subtree, comprising same character - on the left or right end of string
};

// O(1)
Node* new_node(char key) {
    Node* node = new Node();
    node->key = key;
    node->weight = 1;
    node->child[0] = node->child[1] = nullptr;
    node->parent = nullptr;
    node->reversed = false;
    node->longest = 1;
    node->neighbour[0] = node->neighbour[1] = '\0';
    node->end_len[0] = node->end_len[1] = 1;
    return node;
}

// O(n)
void print_tree_in_order(Node* node, int rev_count) {
    if (node->reversed) {
        rev_count++;
    }
    bool odd = rev_count % 2;
    if (node->child[odd]) {
        print_tree_in_order(node->child[odd], rev_count);
    }
    cout << node->key;
    if (node->child[!odd]) {
        print_tree_in_order(node->child[!odd], rev_count);
    }
}

// O(1)
void swap_dual_fields(Node* node) {
    Node* tmp_child = node->child[0];
    node->child[0] = node->child[1];
    node->child[1] = tmp_child;
    int tmp_end = node->end_len[0];
    node->end_len[0] = node->end_len[1];
    node->end_len[1] = tmp_end;
    char tmp_n = node->neighbour[0];
    node->neighbour[0] = node->neighbour[1];
    node->neighbour[1] = tmp_n;
}

// O(1)
void change_reverse_bit(Node* node) {
    if (node) {
        node->reversed = !node->reversed;
    }
}

// O(1)
void normalize_inversions(Node* node) {
    if (node->reversed) {
        swap_dual_fields(node);
        change_reverse_bit(node->child[0]);
        change_reverse_bit(node->child[1]);
        node->reversed = false;
    }
}

// O(1)
int longest_same_chars(Node* node) {
    return max(
            max(
                    (node->child[0] ? node->child[0]->longest : 0),
                    (node->child[1] ? node->child[1]->longest : 0)
            ),
            (node->key == node->neighbour[0] && node->child[0] ? node->child[0]->end_len[1] : 0) +
            (node->key == node->neighbour[1] && node->child[1] ? node->child[1]->end_len[0] : 0) + 1
    );
}

// O(1)
int end_length(Node* node, int side) {
    if (node->child[side] && node->child[!side]) {
        return node->child[side]->end_len[side] +
               (node->child[side]->end_len[side] == node->child[side]->weight && node->key == node->neighbour[side] ?
                (1 + (node->key == node->neighbour[!side] ? node->child[!side]->end_len[side] : 0)) : 0);
    } else if (node->child[side]) {
        return node->child[side]->end_len[side] +
               (node->child[side]->end_len[side] == node->child[side]->weight && node->key == node->neighbour[side] ? 1 : 0);
    } else if (node->child[!side]) {
        return 1 + (node->key == node->neighbour[!side] ? node->child[!side]->end_len[side] : 0);
    } else {
        return 1;
    }
}

// O(1)
void set_new_fields_values(Node *node) {
    if (node->child[0])
        normalize_inversions(node->child[0]);
    if (node->child[1])
        normalize_inversions(node->child[1]);
    node->weight = (node->child[0] ? node->child[0]->weight : 0) +
                   (node->child[1] ? node->child[1]->weight : 0) + 1;
    node->longest = longest_same_chars(node);
    node->end_len[0] = end_length(node, 0);
    node->end_len[1] = end_length(node, 1);
}

class SplayTree {
private:
    // O(1)
    void rotate_left(Node* node) {
        Node* parent = node->parent;
        Node* r = node->child[1];
        if (!r) {
            return;
        }
        Node* l = r->child[0];
        r->parent = parent;
        if (parent) {
            if (node == parent->child[0]) {
                parent->child[0] = r;
            } else {
                parent->child[1] = r;
            }
        } else {
            root = r;
        }
        node->parent = r;
        r->child[0] = node;
        if (l) {
            l->parent = node;
        }
        node->child[1] = l;

        set_new_fields_values(node);
        set_new_fields_values(r);
    }

    // O(1)
    void rotate_right(Node* node) {
        Node* parent = node->parent;
        Node* l = node->child[0];
        if (!l) {
            return;
        }
        Node* r = l->child[1];
        l->parent = parent;
        if (parent) {
            if (node == parent->child[0]) {
                parent->child[0] = l;
            } else {
                parent->child[1] = l;
            }
        } else {
            root = l;
        }
        node->parent = l;
        l->child[1] = node;
        if (r) {
            r->parent = node;
        }
        node->child[0] = r;

        set_new_fields_values(node);
        set_new_fields_values(l);
    }

public:
    Node* root;
    SplayTree() : root(nullptr) {}

    /*
     * Moves node up to the root.
     * time complexity: amortised O(log n)
     */
    void splay(Node* node) {
        if (!node) {
            return;
        }
        Node* parent = node->parent;
        while (parent) {
            Node* grandparent = parent->parent;
            if (!grandparent) {
                normalize_inversions(parent);
                normalize_inversions(node);
                /* Zig */
                if (node == parent->child[0]) {
                    rotate_right(parent);
                } else {
                    rotate_left(parent);
                }
            } else {
                normalize_inversions(grandparent);
                normalize_inversions(parent);
                normalize_inversions(node);
                if (node == parent->child[0]) {
                    if (parent == grandparent->child[0]) {
                        /* Zig-zig */
                        rotate_right(grandparent);
                        rotate_right(parent);
                    } else {
                        /* Zig-zag (parent == grandparent.right) */
                        rotate_right(parent);
                        rotate_left(grandparent);
                    }
                } else { /* node == parent->child[1] */
                    if (parent == grandparent->child[1]) {
                        /* Zig-zig */
                        rotate_left(grandparent);
                        rotate_left(parent);
                    } else {
                        /* Zig-zag (parent == grandparent.left) */
                        rotate_left(parent);
                        rotate_right(grandparent);
                    }
                }
            }
            parent = node->parent;
        }
    }

    /*
     * Finds k-th element of the tree and splays it.
     * time complexity: amortised O(log n)
     */
    Node* find(int k) {
        Node* node = root;
        int rev_count = 0;
        while (node) {
            rev_count += node->reversed;
            bool odd = rev_count % 2;
            Node* left = node->child[odd];
            Node* right = node->child[!odd];
            int s = left ? left->weight : 0;
            if (k == s) {
                break;
            } else if (k < s) {
                if (left) {
                    node = left;
                } else {
                    return nullptr;
                }
            } else {
                if (right) {
                    k = k - s - 1;
                    node = right;
                } else {
                    return nullptr;
                }
            }
        }
        splay(node);
        return node;
    }

    /*
     * Joins two trees (this & other -> this).
     * time complexity: amortised O(log n)
     */
    void join(SplayTree* other) {
        if (!root) {
            root = other->root;
            return;
        }
        if (!other || !other->root) {
            return;
        }
        Node* root2 = other->find(0);
        Node* root1 = find(root->weight - 1);
        root2->reversed = root2->reversed != root1->reversed;
        root2->parent = root1;
        root1->child[1] = root2;
        root1->neighbour[!root1->reversed] = root2->key;
        root2->neighbour[root2->reversed] = root1->key;
        set_new_fields_values(root1);
    }

    /*
     * Splits tree into two (this -> this & other).
     * time complexity: amortised O(log n)
     */
    void split(int k, SplayTree* other) {
        other->root = nullptr;
        Node* root1 = find(k);
        if (root1 == nullptr) {
            if (k < 0) {
                other->root = root;
                root = nullptr;
            } else {
                other->root = nullptr;
            }
            return;
        }
        Node* root2 = root1->child[1];
        root1->child[1] = nullptr;
        root1->neighbour[!root1->reversed] = '\0';
        set_new_fields_values(root1);
        if (root2) {
            root2->reversed = root2->reversed != root2->parent->reversed;
            root2->parent = nullptr;
            other->root = root2;
            other->find(0)->neighbour[root2->reversed] = '\0';
        }
    }

    /*
     * Reverses [first, last] substring.
     * time complexity: amortised O(log n)
     */
    void reverse_substr(int first, int last) {
        SplayTree middle;
        SplayTree right;
        split(first - 1, &middle);
        middle.split(last - first, &right);
        change_reverse_bit(middle.root);
        join(&middle);
        join(&right);
    }

    /*
     * Cuts [first, last] substring and pastes it ahead of where-th element of result string.
     * time complexity: amortised O(log n)
     */
    void move_substr(int first, int last, int where) {
        SplayTree middle;
        SplayTree right;
        split(first - 1, &middle);
        middle.split(last - first, &right);
        join(&right);
        split(where - 1, &right);
        join(&middle);
        join(&right);
    }

    /*
     * Returns maximum length of consistent substring of [first, last] substring, comprising same character.
     * time complexity: amortised O(log n)
     */
    int max_same_chars(int first, int last) {
        SplayTree middle;
        SplayTree right;
        split(first - 1, &middle);
        middle.split(last - first, &right);
        int res = middle.root->longest;
        join(&middle);
        join(&right);
        return res;
    }

    /*
     * Inserts next character of the string (use only before any other operations on splay tree).
     * time complexity: O(1)
     */
    void insert_char(char value) {
        Node *node = new_node(value);
        if (root) {
            root->parent = node;
            root->neighbour[1] = value;
            node->neighbour[0] = root->key;
        }
        node->child[0] = root;
        set_new_fields_values(node);
        root = node;
    }

    /*
     * Prints string represented by splay tree.
     * time complexity: O(n)
     */
    void print() const {
        if (!root) {
            cout << "empty tree" << endl;
        } else {
            print_tree_in_order(root, 0);
            cout << endl;
        }
    }
};

int main() {

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    SplayTree code;

    int length, instr;
    cin >> length >> instr;
    char c;
    for (int i = 0; i < length; i++) {
        cin >> c;
        code.insert_char(c);
    }
    char type;
    int first, last, where;
    for (int i = 0; i < instr; i++) {
        cin >> type;
        cin >> first >> last;
        switch (type) {
            case 'O':
                code.reverse_substr(first - 1, last - 1);
                break;
            case 'P': {
                cin >> where;
                code.move_substr(first - 1, last - 1, where - 1);
                break;
            }
            case 'N': {
                cout << code.max_same_chars(first - 1, last - 1) << '\n';
                break;
            }
            default:
                break;
        }
    }
    return 0;
}
