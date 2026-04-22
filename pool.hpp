#pragma once

#include <cstddef>
#include <cstdint>

class MemoryPool {
public:
    MemoryPool(size_t block_size, size_t block_count);
    ~MemoryPool();

    void* allocate();
    void deallocate(void* ptr);

    size_t blocks_in_use() const;

private:
    void*   m_pool;         // pointer to the start of the mmap'd region
    size_t  m_block_size;   // size of each block in bytes
    size_t  m_block_count;  // total number of blocks
    size_t  m_in_use;       // how many blocks are currently allocated

    void**  m_free_list;    // stack of pointers to free blocks
    size_t  m_free_count;   // how many free blocks are on the stack
};