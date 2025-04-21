#ifndef ART_NODE_HPP
#define ART_NODE_HPP

#include <cstdint>
#include <cstring>

namespace art {

// to avoid dynamic cast
enum class NodeType : uint8_t {
  INVALID = 0,
  LeafNode,
  Node4,
  Node16,
  Node48,
  Node256,
};

/**
    @brief: base class for a node
 */
template <class T> class Node {
public:
  Node() = default;
  Node(const Node<T> &other) = default;
  Node(Node<T> &&other) = default;
  Node(const char *prefix);

  virtual ~Node() { delete this->prefix_; };

  // is leaf or internal node
  NodeType type() const;

  /**
   * @brief Given a key, check prefix match
   * between key[depth...] and prefix_[0...]
   * @return the number of matched keys
  */
  virtual int checkPrefix(const char *key, int key_len, int depth) const;
  int getPrefixLen() const;
  const char *getPrefix() const;
  void resetPrefix(const char *prefix);

protected:
  char *prefix_ = nullptr;
  int prefixLen_ = 0;
  NodeType nodeType_ = NodeType::INVALID;
};

template <class T> Node<T>::Node(const char *prefix) {
  if (prefix != nullptr) {
    int len = std::strlen(prefix);
    this->prefix_ = new char[len + 1];
    std::memmove(this->prefix_, prefix, len);
    this->prefix_[len] = '\0'; // for safety
    this->prefixLen_ = len;
  }
}

template <class T> NodeType Node<T>::type() const { return this->nodeType_; }

template <class T> int Node<T>::getPrefixLen() const { return prefixLen_; }

template <class T> const char *Node<T>::getPrefix() const { return prefix_; }

template <class T> void Node<T>::resetPrefix(const char *prefix) {
  delete this->prefix_;
  int len = std::strlen(prefix);
  this->prefix_ = new char[len + 1];
  std::memmove(this->prefix_, prefix, len);
  this->prefix_[len] = '\0'; // for safety
  this->prefixLen_ = len;
}

template <class T>
int Node<T>::checkPrefix(const char *key, int key_len, int depth) const {
  int ptrk = depth;
  int ptrp = 0;
  // [depth, ptrk)
  while (ptrk < key_len && ptrp < prefixLen_ && key[ptrk] == prefix_[ptrp]) {
    ptrk++, ptrp++;
  }
  return ptrk - depth;
}

} // namespace art

#endif