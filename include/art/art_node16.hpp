#ifndef ART_NODE16_HPP
#define ART_NODE16_HPP

#include "art_inner_node.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>

#if defined(__i386__) || defined(__amd64__)
#include <emmintrin.h>
#endif

namespace art {

template <class T> class Node4;
template <class T> class Node48;
template <class T> class AdaptiveRadixTreePrinter;

template <class T> class Node16 : public InnerNode<T> {
  friend class Node4<T>;
  friend class Node48<T>;
  friend class AdaptiveRadixTreePrinter<T>;

public:
  Node16() {
    this->nodeType_ = NodeType::Node16;
    memset(key_, 0, sizeof(uint8_t) * MAX);
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  };
  Node16(const Node16<T> &);
  Node16(const char *prefix) : InnerNode<T>(prefix) {
    this->nodeType_ = NodeType::Node16;
    memset(key_, 0, sizeof(uint8_t) * MAX);
    for (int i = 0; i < MAX; ++i)
      child_[i] = nullptr;
  }
  ~Node16();
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
  static constexpr int MAX = 16;
  static constexpr int MIN = 5;
  uint8_t size_ = 0;
  uint8_t key_[MAX];
  Node<T> *child_[MAX];
};

template <class T>
Node16<T>::Node16(const Node16<T> &other) : InnerNode<T>{other.prefix_} {
  this->nodeType_ = NodeType::Node16;
  // set up <k, ptr>
  this->size_ = other.size_;
  std::copy(other.key_, other.key_ + other.size_, this->key_);
  std::copy(other.child_, other.child_ + other.size_, this->child_);
}

template <class T> Node16<T>::~Node16() {
  for (int i = 0; i < size_; ++i) {
    delete child_[i];
  }
}

template <class T> Node<T> *Node16<T>::findChild(uint8_t byte) {
#if defined(__i386__) || defined(__amd64__)
  __m128i key = _mm_set1_epi8(byte);
  __m128i ndkey = _mm_loadu_si128((__m128i *)key_);
  __m128i mask = _mm_cmpeq_epi8(ndkey, key);
  int bitfield = _mm_movemask_epi8(mask) & ((1 << size_) - 1);
  if (bitfield) {
    return child_[__builtin_ctz(bitfield)];
  } else {
    return nullptr;
  }
#endif

  int index = std::lower_bound(key_, key_ + size_, byte) - key_;
  if (key_[index] == byte) {
    return child_[index];
  }
  return nullptr;
}

template <class T> void Node16<T>::addChild(uint8_t byte, Node<T> *child) {
  int index = size_;

#if defined(__i386__) || defined(__amd64__)
  __m128i key = _mm_set1_epi8(byte);
  __m128i ndkey = _mm_loadu_si128((__m128i *)key_);
  __m128i mask = _mm_cmpgt_epi8(ndkey, key);
  int bitfield = _mm_movemask_epi8(mask) & ((1 << size_) - 1);
  if (bitfield) {
    index = __builtin_ctz(bitfield);
  }
#else
  index = std::upper_bound(key_, key_ + size_, byte) - key_;
#endif

  if (index && key_[index - 1] == byte) {
    // std::cout << "child[" << static_cast<int>(key_[index - 1])
    //           << "] already exist\n";
    child_[index - 1] = child;
    return;
  }
  assert(!isFull());
  // move [index, size_ - 1] back
  size_t n = size_ - index;
  std::memmove(key_ + index + 1, key_ + index, n);
  std::memmove(child_ + index + 1, child_ + index, n * sizeof(Node<T> *));
  // install the passed in pointer
  key_[index] = byte;
  child_[index] = child;
  size_++;
}

template <class T> void Node16<T>::deleteChild(uint8_t byte) {
  assert(!isLack());
  int index = size_;

#if defined(__i386__) || defined(__amd64__)
  __m128i key = _mm_set1_epi8(byte);
  __m128i ndkey = _mm_loadu_si128((__m128i *)key_);
  __m128i mask = _mm_cmpeq_epi8(ndkey, key);
  int bitfield = _mm_movemask_epi8(mask) & ((1 << size_) - 1);
  if (bitfield) {
    index = __builtin_ctz(bitfield);
  }
#else
  index = std::lower_bound(key_, key_ + size_, byte) - key_;
#endif

  if (index == size_ || key_[index] != byte) {
    // no match
    return;
  }
  // key_[index] == byte, move [idx + 1, size_ - 1] forword
  size_t n = size_ - index - 1;
  std::memmove(key_ + index, key_ + index + 1, n);
  std::memmove(child_ + index, child_ + index + 1, n * sizeof(Node<T> *));
  size_--;
}

template <class T> bool Node16<T>::isFull() const { return size_ == MAX; }

template <class T> bool Node16<T>::isLack() const { return size_ < MIN; }

template <class T> InnerNode<T> *Node16<T>::grow() {
  Node48<T> *newNode = new Node48<T>{this->prefix_};
  newNode->size_ = this->size_;

  for (uint8_t i = 0; i < this->size_; ++i) {
    char key = this->key_[i];
    newNode->childIndex_[key] = i;
    newNode->child_[i] = this->child_[i];
    this->child_[i] = nullptr;
  }

  delete this;
  return newNode;
}

template <class T> Node<T> *Node16<T>::shrink() {
  Node4<T> *newNode = new Node4<T>{this->prefix_};
  newNode->size_ = this->size_;
  for (uint8_t i = 0; i < this->size_; ++i) {
    newNode->key_[i] = this->key_[i];
    newNode->child_[i] = this->child_[i];
    this->child_[i] = nullptr;
  }

  delete this;
  return newNode;
}

template <class T> Node<T> *Node16<T>::growChild(uint8_t byte) {
#if defined(__i386__) || defined(__amd64__)
  __m128i key = _mm_set1_epi8(byte);
  __m128i ndkey = _mm_loadu_si128((__m128i *)key_);
  __m128i mask = _mm_cmpeq_epi8(ndkey, key);
  int bitfield = _mm_movemask_epi8(mask) & ((1 << size_) - 1);
  if (bitfield) {
    int index = __builtin_ctz(bitfield);
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isFull());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->grow();
    return child_[index];
  } else {
    return nullptr;
  }
#endif

  int index = std::lower_bound(key_, key_ + size_, byte) - key_;
  if (key_[index] == byte) {
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isFull());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->grow();
    return child_[index];
  }
  return nullptr;
}

template <class T> Node<T> *Node16<T>::shrinkChild(uint8_t byte) {
#if defined(__i386__) || defined(__amd64__)
  __m128i key = _mm_set1_epi8(byte);
  __m128i ndkey = _mm_loadu_si128((__m128i *)key_);
  __m128i mask = _mm_cmpeq_epi8(ndkey, key);
  int bitfield = _mm_movemask_epi8(mask) & ((1 << size_) - 1);
  if (bitfield) {
    int index = __builtin_ctz(bitfield);
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isLack());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->shrink();
    return child_[index];
  } else {
    return nullptr;
  }
#endif

  int index = std::lower_bound(key_, key_ + size_, byte) - key_;
  if (key_[index] == byte) {
    assert(child_[index]->type() != NodeType::LeafNode);
    assert(static_cast<InnerNode<T> *>(child_[index])->isLack());
    child_[index] = static_cast<InnerNode<T> *>(child_[index])->shrink();
    return child_[index];
  }
  return nullptr;
}

} // namespace art

#endif