#ifndef ART_INNER_NODE_HPP
#define ART_INNER_NODE_HPP

#include "art_node.hpp"
#include <algorithm>

namespace art {
/**
    @brief Adaptive Radix tree inner node base class
 */
template <class T> class InnerNode : public Node<T> {
public:
  InnerNode() = default;
  InnerNode(const char *prefix) : Node<T>(prefix){};
  virtual Node<T> *findChild(uint8_t byte) = 0;
  virtual void addChild(uint8_t byte, Node<T> *child) = 0;
  virtual void deleteChild(uint8_t byte) = 0;
  virtual bool isFull() const = 0;
  virtual bool isLack() const = 0;
  virtual InnerNode<T> *grow() = 0;
  virtual Node<T> *shrink() = 0;

  /**
    @brief Check if specific child is full,
      if yes, then do grow operation and renew the pointer
    @param[in] byte the index key
    @return return the child node valid for an insert operation,
      if not exist, return nullptr
  */
  virtual Node<T> *growChild(uint8_t byte) = 0;

  /**
    @brief Check if specific chuld is lack,
      if yes, then do shrink operation and renew the pointer
    @param[in] byte the index key
    @return return the child node, if not exist, return nullptr
  */
  virtual Node<T> *shrinkChild(uint8_t byte) = 0;

  // truncate the first offset bytes off the prefix
  // keep [offset, prefixLen)
  void truncPrefix(int offset) {
    int len = this->prefixLen_ - offset;
    if (len == 0) {
      delete this->prefix_;
      this->prefix_ = nullptr;
      this->prefixLen_ = 0;
      return;
    }
    char *newPrefix = new char[len + 1];
    std::copy(this->prefix_ + offset, this->prefix_ + this->prefixLen_,
              newPrefix);
    newPrefix[len] = '\0'; // for safety
    delete this->prefix_;
    this->prefix_ = newPrefix;
    this->prefixLen_ = len;
  }
};
} // namespace art

#endif