#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <new>

namespace hft {
namespace core {

/**
 * Lock-free memory pool for fast allocation
 * Pre-allocates memory blocks to avoid runtime allocation overhead
 */
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    MemoryPool() {
        expandPool();
    }

    ~MemoryPool() {
        for (auto* block : blocks_) {
            ::operator delete(block);
        }
    }

    // Non-copyable
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    template<typename... Args>
    T* allocate(Args&&... args) {
        if (free_list_ == nullptr) {
            expandPool();
        }

        Node* node = free_list_;
        free_list_ = free_list_->next;

        T* obj = reinterpret_cast<T*>(node);
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    void deallocate(T* ptr) {
        if (ptr == nullptr) return;

        ptr->~T();

        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }

private:
    union Node {
        T data;
        Node* next;

        Node() {}
        ~Node() {}
    };

    void expandPool() {
        size_t block_size = BlockSize * sizeof(Node);
        Node* new_block = static_cast<Node*>(::operator new(block_size));

        blocks_.push_back(new_block);

        for (size_t i = 0; i < BlockSize - 1; ++i) {
            new_block[i].next = &new_block[i + 1];
        }
        new_block[BlockSize - 1].next = free_list_;
        free_list_ = new_block;
    }

    Node* free_list_ = nullptr;
    std::vector<Node*> blocks_;
};

} // namespace core
} // namespace hft
