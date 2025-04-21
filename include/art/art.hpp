#ifndef ART_IMPL_H
#define ART_IMPL_H

#include "art_inner_node.hpp"
#include "art_leaf_node.hpp"

namespace art {

enum class RC {
  SUCCESS,
  INTERNAL_FAILURE,
  KEY_NOT_EXIST,
};

template <class T> class AdaptiveRadixTreePrinter;
template <class T> class Node4;
template <class T> class Node16;
template <class T> class Node48;
template <class T> class Node256;

template <class T> class AdaptiveRadixTree {
  friend class AdaptiveRadixTreePrinter<T>;

public:
  AdaptiveRadixTree() { root_ = nullptr; }

  /**
    @brief Given key, try to get the corresponding value
    @param[out] val hold the value if key exists
  */
  RC search(const char *key, T &value);

  /**
    @brief Given a <Key, Value> pair, do insert
      if key already exists, do update
  */
  RC insert(const char *key, const T &value);

  /**
    @brief Given key, delete if exists
    @param[out] value hold the value if key exists
  */
  RC remove(const char *key, T &value);

private:
  Node<T> *findChild(Node<T> *node, char byte);

  Node<T> *root_;
};

template <class T> RC AdaptiveRadixTree<T>::search(const char *key, T &val) {
  if (root_ == nullptr) {
    return RC::KEY_NOT_EXIST;
  }
  size_t keyLen = std::strlen(key);
  Node<T> *cur = this->root_;
  int depth = 0;
  while (cur->type() != NodeType::LeafNode) {
    // first check prefix match
    int len = cur->getPrefixLen();
    if (cur->checkPrefix(key, keyLen, depth) != len) {
      return RC::KEY_NOT_EXIST;
    }
    depth += len;
    cur = findChild(cur, key[depth]);
    if (cur == nullptr) {
      return RC::KEY_NOT_EXIST;
    }
    depth++;
  }
  if (static_cast<LeafNode<T> *>(cur)->checkKeyMatch(key, keyLen)) {
    val = static_cast<LeafNode<T> *>(cur)->getValue();
    return RC::SUCCESS;
  }
  return RC::KEY_NOT_EXIST;
}

template <class T>
Node<T> *AdaptiveRadixTree<T>::findChild(Node<T> *node, char byte) {
  switch (node->type()) {
  case NodeType::Node4:
    return static_cast<Node4<T> *>(node)->findChild(static_cast<uint8_t>(byte));
  case NodeType::Node16:
    return static_cast<Node16<T> *>(node)->findChild(
        static_cast<uint8_t>(byte));
  case NodeType::Node48:
    return static_cast<Node48<T> *>(node)->findChild(
        static_cast<uint8_t>(byte));
  case NodeType::Node256:
    return static_cast<Node48<T> *>(node)->findChild(
        static_cast<uint8_t>(byte));
  default:
    return nullptr;
  }
}

template <class T>
RC AdaptiveRadixTree<T>::insert(const char *key, const T &value) {
  // create leaf node
  auto leafNode = new LeafNode<T>{key, value};
  // Cond1: root is empty
  if (root_ == nullptr) {
    root_ = leafNode;
    return RC::SUCCESS;
  }

  int keyLen = strlen(key);
  int depth = 0;
  Node<T> *prev = nullptr;
  uint8_t prevKey = 0;
  Node<T> *cur = root_;

  while (cur != nullptr) {
    int len = cur->getPrefixLen();
    int matchLen = cur->checkPrefix(key, keyLen, depth);
    if (cur->type() == NodeType::LeafNode) {
      // leaf nodes always hold the complete key
      len -= depth;
    }

    // Cond2: prefix mismatch
    if (matchLen != len ||
        (cur->type() == NodeType::LeafNode && matchLen != keyLen - depth)) {
      // create new internal node that holds common prefix
      char *newPrefix = new char[matchLen + 1];
      std::copy(key + depth, key + depth + matchLen, newPrefix);
      newPrefix[matchLen] = '\0';
      auto innerNode = new Node4<T>{newPrefix};
      // get the first unmatched key, use them as index keys
      auto newLeafKey = static_cast<uint8_t>(key[depth + matchLen]);
      auto curNodeKey = static_cast<uint8_t>(cur->getPrefix()[matchLen]);
      // truncate the prefix of the old inner node
      if (cur->type() != NodeType::LeafNode) {
        static_cast<InnerNode<T> *>(cur)->truncPrefix(matchLen + 1);
      } else {
        curNodeKey = static_cast<uint8_t>(cur->getPrefix()[matchLen + depth]);
      }
      static_cast<InnerNode<T> *>(innerNode)->addChild(newLeafKey, leafNode);
      static_cast<InnerNode<T> *>(innerNode)->addChild(curNodeKey, cur);
      if (prev != nullptr) {
        static_cast<InnerNode<T> *>(prev)->addChild(prevKey, innerNode);
      } else if (cur == root_) {
        root_ = innerNode;
      }
      delete[] newPrefix;
      return RC::SUCCESS;
    }

    // Cond3: key already exists, update value
    if (cur->type() == NodeType::LeafNode) {
      static_cast<LeafNode<T> *>(cur)->setValue(value);
      return RC::SUCCESS;
    }

    depth += matchLen;
    Node<T> *nxt = findChild(cur, key[depth]);
    // Cond4: Reach nullptr
    if (nxt == nullptr) {
      if (static_cast<InnerNode<T> *>(cur)->isFull()) {
        if (prev != nullptr) {
          cur = static_cast<InnerNode<T> *>(prev)->growChild(prevKey);
        } else {
          cur = static_cast<InnerNode<T> *>(cur)->grow();
          root_ = cur;
        }
      }
      static_cast<InnerNode<T> *>(cur)->addChild(key[depth], leafNode);
      return RC::SUCCESS;
    }
    prevKey = static_cast<uint8_t>(key[depth]);
    prev = cur;
    cur = nxt;
    depth++;
  }
  return RC::SUCCESS;
}

template <class T> RC AdaptiveRadixTree<T>::remove(const char *key, T &value) {
  if (root_ == nullptr) {
    return RC::KEY_NOT_EXIST;
  }

  // root is leaf node
  if (root_->type() == NodeType::LeafNode) {
    if (static_cast<LeafNode<T> *>(root_)->checkKeyMatch(key,
                                                         std::strlen(key))) {
      value = static_cast<LeafNode<T> *>(root_)->getValue();
      delete root_;
      root_ = nullptr;
      return RC::SUCCESS;
    }
    return RC::KEY_NOT_EXIST;
  }

  Node<T> *prev = nullptr;
  uint8_t prevKey = 0;
  Node<T> *cur = root_;
  size_t keyLen = std::strlen(key);
  int depth = 0;
  while (cur->type() != NodeType::LeafNode) {
    int len = cur->getPrefixLen();
    if (cur->checkPrefix(key, keyLen, depth) != len) {
      return RC::KEY_NOT_EXIST;
    }
    depth += len;
    auto nxt = findChild(cur, key[depth]);
    if (nxt == nullptr) {
      return RC::KEY_NOT_EXIST;
    }
    if (nxt->type() == NodeType::LeafNode) {
      if (static_cast<LeafNode<T> *>(nxt)->checkKeyMatch(key, keyLen)) {
        // nxt is the node to be deleted
        static_cast<InnerNode<T> *>(cur)->deleteChild(key[depth]);
        // shrink node if necessary
        if (static_cast<InnerNode<T> *>(cur)->isLack()) {
          if (prev != nullptr) {
            static_cast<InnerNode<T> *>(prev)->shrinkChild(prevKey);
          } else {
            cur = static_cast<InnerNode<T> *>(cur)->shrink();
            root_ = cur;
          }
        }
        value = static_cast<LeafNode<T> *>(nxt)->getValue();
        delete nxt;
        return RC::SUCCESS;
      }
      return RC::KEY_NOT_EXIST;
    }
    prevKey = key[depth];
    prev = cur;
    cur = nxt;
    depth++;
  }
  return RC::KEY_NOT_EXIST;
}

} // namespace art

#endif