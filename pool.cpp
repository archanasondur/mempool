#include "pool.hpp"

#include <sys/mman.h>
#include <stdexcept>
#include <cassert>

MemoryPool::MemoryPool(size_t block_size, size_t block_count)
    : m_block_size(block_size)
    , m_block_count(block_count)
    , m_in_use(0)
    , m_free_count(0)
{
    size_t total_bytes = block_size * block_count;

    // Ask the OS for a contiguous chunk of memory
    m_pool = mmap(
        nullptr,                        // let OS choose the address
        total_bytes,                    // how much memory we want
        PROT_READ | PROT_WRITE,         // we want to read and write it
        MAP_PRIVATE | MAP_ANONYMOUS,    // not backed by a file, private to this process
        -1,                             // no file descriptor
        0                               // no offset
    );

    if (m_pool == MAP_FAILED) {
        throw std::runtime_error("mmap failed");
    }

    // Allocate the free list separately (this is the only malloc we do, at startup)
    m_free_list = new void*[block_count];

    // Carve the pool into blocks and push each block pointer onto the free list
    for (size_t i = 0; i < block_count; i++) {
        uint8_t* block = static_cast<uint8_t*>(m_pool) + (i * block_size);
        m_free_list[m_free_count++] = block;
    }
}

MemoryPool::~MemoryPool() {
    size_t total_bytes = m_block_size * m_block_count;
    munmap(m_pool, total_bytes);
    delete[] m_free_list;
}

void* MemoryPool::allocate() {
    if (m_free_count == 0) {
        return nullptr; // pool is exhausted
    }
    m_in_use++;
    return m_free_list[--m_free_count]; // pop from free list
}

void MemoryPool::deallocate(void* ptr) {
    assert(ptr != nullptr);
    assert(m_in_use > 0 && "double-free or corrupt pool state");

    m_in_use--;
    m_free_list[m_free_count++] = ptr; // push back onto free list
}

size_t MemoryPool::blocks_in_use() const {
    return m_in_use;
}