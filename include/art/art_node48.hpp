#ifndef ART_NODE48_HPP
#define ART_NODE48_HPP

#include "art_inner_node.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace art {

template <class T> class Node16;
template <class T> class Node256;
template <class T> class AdaptiveRadixTreePrinter;

template <class T> class Node48 : public InnerNode<T> {
  friend class Node16<T>;
  friend class Node256<T>;
  friend class AdaptiveRadixTreePrinter<T>;

public:
  Node48();
  Node48(const Node48<T> &);
  Node48(const char *prefix) : InnerNode<T>{prefix} {
    this->nodeType_ = NodeType::Node48;
    std::fill(this->childIndex_, this->childIndex_ + CIMAX, (int8_t)-1);
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  }
  ~Node48();
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
  static constexpr int CIMAX = 256;
  static constexpr int MAX = 48;
  static constexpr int MIN = 17;
  // used to index into child_[]
  // -1 means key doesn't exist
  int8_t childIndex_[CIMAX];
  Node<T> *child_[MAX];
  uint8_t size_ = 0;
};

template <class T> Node48<T>::Node48() {
  this->nodeType_ = NodeType::Node48;
  std::fill(this->childIndex_, this->childIndex_ + CIMAX, (int8_t)-1);
  for (int i = 0; i < MAX; ++i)
    child_[i] = nullptr;
}

template <class T>
Node48<T>::Node48(const Node48<T> &other) : InnerNode<T>{other.prefix_} {
  this->nodeType_ = NodeType::Node48;
  this->size_ = other.size_;
  std::copy(other.childIndex_, other.childIndex_ + CIMAX, this->childIndex_);
  std::copy(other.child_, other.child_ + MAX, this->child_);
}

template <class T> Node48<T>::~Node48() {
  for (int i = 0; i < MAX; ++i) {
    if (child_[i] != nullptr) {
      assert(false);
    }
    delete child_[i];
  }
}

template <class T> Node<T> *Node48<T>::findChild(uint8_t byte) {
  auto index = childIndex_[byte];
  if (index >= 0) {
    assert(index < MAX);
    return child_[index];
  }
  return nullptr;
}

template <class T> void Node48<T>::addChild(uint8_t byte, Node<T> *child) {
  if (childIndex_[byte] != -1) {
    child_[childIndex_[byte]] = child;
    return;
  }

  assert(!isFull());
  int index = 0;
  for (; index < MAX; ++index) {
    if (child_[index] == nullptr) {
      break;
    }
  }
  assert(index >= 0 && index < MAX);
  child_[index] = child;
  childIndex_[byte] = index;
  size_++;
}

template <class T> void Node48<T>::deleteChild(uint8_t byte) {
  assert(!isLack());
  if (childIndex_[byte] != -1) {
    child_[childIndex_[byte]] = nullptr;
    childIndex_[byte] = -1;
  }
  size_--;
}

template <class T> bool Node48<T>::isFull() const { return size_ == MAX; }

template <class T> bool Node48<T>::isLack() const { return size_ < MIN; }

template <class T> InnerNode<T> *Node48<T>::grow() {
  Node256<T> *newNode = new Node256<T>{this->prefix_};
  newNode->size_ = this->size_;
  uint8_t cnt = 0;
  for (int key = 0; key < CIMAX && cnt < size_; ++key) {
    int8_t index = childIndex_[key];
    if (index >= 0) {
      newNode->child_[key] = this->child_[index];
      this->child_[index] = nullptr;
      cnt++;
    }
  }
  assert(cnt == this->size_);
  delete this;
  return newNode;
}

template <class T> Node<T> *Node48<T>::shrink() {
  Node16<T> *newNode = new Node16<T>{this->prefix_};
  newNode->size_ = this->size_;
  uint8_t cnt = 0;
  for (int key = 0; key < CIMAX && cnt < this->size_; ++key) {
    int8_t index = childIndex_[key];
    if (index >= 0) {
      newNode->key_[cnt] = key;
      newNode->child_[cnt] = this->child_[index];
      this->child_[index] = nullptr;
      cnt++;
    }
  }
  assert(cnt == this->size_);
  delete this;
  return newNode;
}

template <class T> Node<T> *Node48<T>::growChild(uint8_t byte) {
  auto index = childIndex_[byte];
  if (index >= 0) {
    assert(index < MAX);
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isFull());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->grow();
    return child_[index];
  }
  return nullptr;
}

template <class T> Node<T> *Node48<T>::shrinkChild(uint8_t byte) {
  auto index = childIndex_[byte];
  if (index >= 0) {
    assert(index < MAX);
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isLack());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->shrink();
    return child_[index];
  }
  return nullptr;
}

} // namespace art

#endif