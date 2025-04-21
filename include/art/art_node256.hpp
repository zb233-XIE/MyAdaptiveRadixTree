#ifndef ART_NODE256_HPP
#define ART_NODE256_HPP

#include "art_inner_node.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace art {

template <class T> class Node48;
template <class T> class AdaptiveRadixTreePrinter;

template <class T> class Node256 : public InnerNode<T> {
  friend class Node48<T>;
  friend class AdaptiveRadixTreePrinter<T>;

public:
  Node256() {
    this->nodeType_ = NodeType::Node256;
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  };
  Node256(const Node256<T> &);
  Node256(const char *prefix) : InnerNode<T>{prefix} {
    this->nodeType_ = NodeType::Node256;
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  }
  ~Node256();
  Node<T> *findChild(uint8_t byte) override;
  void addChild(uint8_t byte, Node<T> *child) override;
  void deleteChild(uint8_t byte) override;
  bool isFull() const override;
  bool isLack() const override;
  InnerNode<T> *grow() override;
  Node<T> *shrink() override;
  Node<T> *growChild(uint8_t byte) override;
  Node<T> *shrinkChild(uint8_t byte) override;

private:
  static constexpr int MAX = 256;
  static constexpr int MIN = 49;
  uint8_t size_ = 0;
  Node<T> *child_[MAX];
};

template <class T> Node<T> *Node256<T>::findChild(uint8_t byte) {
  return child_[byte];
}

template <class T>
Node256<T>::Node256(const Node256<T> &other) : InnerNode<T>{other.prefix_} {
  this->nodeType_ = NodeType::Node256;
  this->size_ = other.size_;
  std::copy(other.child_, other.child_ + MAX, this->child_);
}

template <class T> Node256<T>::~Node256() {
  for (int i = 0; i < MAX; ++i) {
    delete child_[i];
  }
}

template <class T> void Node256<T>::addChild(uint8_t byte, Node<T> *child) {
  auto index = byte;
  if (child_[index] == nullptr) {
    child_[index] = child;
    return;
  }
  assert(!isFull());
  child_[index] = child;
  size_++;
}

template <class T> void Node256<T>::deleteChild(uint8_t byte) {
  assert(!isLack());
  auto index = byte;
  child_[index] = nullptr;
  size_--;
}

template <class T> bool Node256<T>::isFull() const { return size_ == MAX; }

template <class T> bool Node256<T>::isLack() const { return size_ < MIN; }

template <class T> InnerNode<T> *Node256<T>::grow() {
  throw std::runtime_error("Node256 don't grow");
}

template <class T> Node<T> *Node256<T>::shrink() {
  auto newNode = new Node48<T>{this->prefix_};
  newNode->size_ = this->size_;
  uint8_t cnt = 0;
  for (int key = 0; key < MAX && cnt < this->size_; ++key) {
    if (child_[key] != nullptr) {
      newNode->child_[cnt] = child_[key];
      newNode->childIndex_[key] = cnt;
      child_[key] = nullptr;
      cnt++;
    }
  }
  assert(cnt == this->size_);
  delete this;
  return newNode;
}

template <class T> Node<T> *Node256<T>::growChild(uint8_t byte) {
  if (child_[byte] != nullptr) {
    assert(child_[byte]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[byte])->isFull());
    child_[byte] = static_cast<InnerNode<T> *>(child_[byte])->grow();
  }
  return child_[byte];
}

template <class T> Node<T> *Node256<T>::shrinkChild(uint8_t byte) {
  if (child_[byte] != nullptr) {
    assert(child_[byte]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[byte])->isLack());
    child_[byte] = static_cast<InnerNode<T> *>(child_[byte])->shrink();
  }
  return child_[byte];
}

} // namespace art

#endif