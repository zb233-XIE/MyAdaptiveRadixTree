#ifndef ART_LEAF_NODE_HPP
#define ART_LEAF_NODE_HPP

#include "art_node.hpp"

namespace art {

template <class T> class AdaptiveRadixTreePrinter;

/**
    @brief Adaptive Radix Tree Leaf node
        store full key in prefix and its corresponding value
 */
template <class T> class LeafNode : public Node<T> {
  friend class AdaptiveRadixTreePrinter<T>;

public:
  LeafNode(const char *prefix, const T &value);
  const T &getValue() const;
  void setValue(const T &value);

  /**
   * @brief For leaf node, given a key, check prefix match
   * key[depth...] and prefix_[depth...]
   * Different from inner node implementation
   * @return the number of matched keys
   */
  int checkPrefix(const char *key, int key_len, int depth) const override;

  bool checkKeyMatch(const char *key, int key_len) const;

private:
  T value_;
};

template <class T>
LeafNode<T>::LeafNode(const char *prefix, const T &value) : Node<T>(prefix) {
  value_ = value;
  this->nodeType_ = NodeType::LeafNode;
}

template <class T> const T &LeafNode<T>::getValue() const { return value_; }

template <class T> void LeafNode<T>::setValue(const T &value) {
  this->value_ = value;
}

template <class T>
int LeafNode<T>::checkPrefix(const char *key, int key_len, int depth) const {
  int i = depth;
  // [depth, i)
  while (i < key_len && i < this->prefixLen_ && key[i] == this->prefix_[i]) {
    i++;
  }
  return i - depth;
}

template <class T>
bool LeafNode<T>::checkKeyMatch(const char *key, int key_len) const {
  if (key_len != this->prefixLen_) {
    return false;
  }
  return this->prefixLen_ == checkPrefix(key, key_len, 0);
}

} // namespace art

#endif