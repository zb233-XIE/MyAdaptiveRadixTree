#ifndef ART_NODE4_HPP
#define ART_NODE4_HPP

#include "art_inner_node.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace art {

template <class T> class Node16;
template <class T> class AdaptiveRadixTreePrinter;

template <class T> class Node4 : public InnerNode<T> {
  friend class Node16<T>;
  friend class AdaptiveRadixTreePrinter<T>;

public:
  Node4() {
    this->nodeType_ = NodeType::Node4;
    memset(key_, 0, sizeof(uint8_t) * MAX);
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  };

  Node4(const Node4<T> &);
  Node4(const char *prefix) : InnerNode<T>(prefix) {
    this->nodeType_ = NodeType::Node4;
    memset(key_, 0, sizeof(uint8_t) * MAX);
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  }
  ~Node4();
  Node<T> *findChild(uint8_t byte) override;
  void addChild(uint8_t byte, Node<T> *child) override;
  void deleteChild(uint8_t byte) override;
  bool isFull() const override;
  bool isLack() const override;
  InnerNode<T> *grow() override;

  /**
    @brief if has only one child, do path compression
    if the child node is not leaf, append current prefix
    and index to head
  */
  Node<T> *shrink() override;

  Node<T> *growChild(uint8_t byte) override;
  Node<T> *shrinkChild(uint8_t byte) override;

private:
  static constexpr int MAX = 4;
  static constexpr int MIN = 1;
  uint8_t size_ = 0;
  uint8_t key_[MAX];
  Node<T> *child_[MAX];
};

template <class T>
Node4<T>::Node4(const Node4<T> &other) : InnerNode<T>(other.prefix_) {
  this->nodeType_ = NodeType::Node4;
  this->size_ = other.size_;
  // set up <k, ptr>
  std::copy(other.key_, other.key_ + other.size_, this->key_);
  std::copy(other.child_, other.child_ + other.size_, this->child_);
}

template <class T> Node4<T>::~Node4() {
  for (int i = 0; i < size_; ++i) {
    delete child_[i];
  }
}

template <class T> Node<T> *Node4<T>::findChild(uint8_t byte) {
  for (int i = 0; i < size_; ++i) {
    if (key_[i] == byte) {
      assert(child_[i] != nullptr);
      return child_[i];
    }
  }
  return nullptr;
}

template <class T> void Node4<T>::addChild(uint8_t byte, Node<T> *child) {
  int idx = 0;
  while (idx < size_ && byte > key_[idx]) {
    idx++;
  }
  if (idx < size_ && key_[idx] == byte) {
    child_[idx] = child;
    return;
  }
  assert(!isFull());
  // move [idx, size_ - 1] back
  size_t n = size_ - idx;
  std::memmove(key_ + idx + 1, key_ + idx, n);
  std::memmove(child_ + idx + 1, child_ + idx, n * sizeof(Node<T> *));
  // install the passed in pointer
  key_[idx] = byte;
  child_[idx] = child;
  size_++;
}

template <class T> void Node4<T>::deleteChild(uint8_t byte) {
  assert(!isLack());
  int idx = 0;
  while (idx < size_ && byte > key_[idx]) {
    idx++;
  }
  if (idx >= size_ || key_[idx] != byte) {
    // no match
    return;
  }
  // key_[idx] == byte, move [idx + 1, size_ - 1] forword
  size_t n = size_ - idx - 1;
  std::memmove(key_ + idx, key_ + idx + 1, n);
  std::memmove(child_ + idx, child_ + idx + 1, n * sizeof(Node<T> *));
  size_--;
}

template <class T> bool Node4<T>::isFull() const { return size_ == MAX; }

template <class T> bool Node4<T>::isLack() const { return size_ <= MIN; }

template <class T> InnerNode<T> *Node4<T>::grow() {
  Node16<T> *newNode = new Node16<T>{this->prefix_};
  newNode->size_ = this->size_;
  for (uint8_t i = 0; i < size_; ++i) {
    newNode->key_[i] = this->key_[i];
    newNode->child_[i] = this->child_[i];
    this->child_[i] = nullptr;
  }
  delete this;
  return newNode;
}

template <class T> Node<T> *Node4<T>::shrink() {
  assert(this->size_ == MIN);
  Node<T> *newNode = this->child_[0];
  if (newNode->type() != NodeType::LeafNode) {
    char *curPrefix = this->prefix_;
    char *childPrefix = const_cast<char *>(newNode->getPrefix());

    int len = this->prefixLen_ + 1 + newNode->getPrefixLen();
    char *newPrefix = new char[len + 1];

    std::copy(curPrefix, curPrefix + this->prefixLen_, newPrefix);
    newPrefix[this->prefixLen_] = this->key_[0]; // index key
    std::copy(childPrefix, childPrefix + newNode->getPrefixLen(),
              newPrefix + this->prefixLen_ + 1);

    newPrefix[len] = '\0';
    newNode->resetPrefix(newPrefix);
    delete[] newPrefix;
  }

  this->child_[0] = nullptr;
  delete this;
  return newNode;
}

template <class T> Node<T> *Node4<T>::growChild(uint8_t byte) {
  for (int i = 0; i < size_; ++i) {
    if (key_[i] == byte) {
      assert(child_[i]->type() != NodeType::LeafNode);
      assert(static_cast<InnerNode<T> *>(child_[i])->isFull());
      child_[i] = static_cast<InnerNode<T> *>(child_[i])->grow();
      return child_[i];
    }
  }
  return nullptr;
}

template <class T> Node<T> *Node4<T>::shrinkChild(uint8_t byte) {
  for (int i = 0; i < size_; ++i) {
    if (key_[i] == byte) {
      assert(child_[i]->type() != NodeType::LeafNode);
      assert(static_cast<InnerNode<T> *>(child_[i])->isLack());
      child_[i] = static_cast<InnerNode<T> *>(child_[i])->shrink();
      return child_[i];
    }
  }
  return nullptr;
}

} // namespace art

#endif