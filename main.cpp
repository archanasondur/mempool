#include "pool.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cassert>

static const size_t BLOCK_SIZE  = 64;
static const size_t BLOCK_COUNT = 10 * 1024 * 1024 / BLOCK_SIZE; // 10MB / 64 = 163840 blocks
static const size_t ITERATIONS  = 100000;

int main() {
    // ── Correctness test ────────────────────────────────────────────
    {
        MemoryPool pool(BLOCK_SIZE, BLOCK_COUNT);

        void* a = pool.allocate();
        void* b = pool.allocate();
        assert(a != nullptr);
        assert(b != nullptr);
        assert(a != b); // two allocations must return different blocks

        assert(pool.blocks_in_use() == 2);

        pool.deallocate(a);
        pool.deallocate(b);

        assert(pool.blocks_in_use() == 0);

        // exhaust the pool
        std::vector<void*> all;
        for (size_t i = 0; i < BLOCK_COUNT; i++) {
            void* p = pool.allocate();
            assert(p != nullptr);
            all.push_back(p);
        }

        // pool is full, next allocation should return nullptr
        assert(pool.allocate() == nullptr);

        // free everything
        for (void* p : all) pool.deallocate(p);
        assert(pool.blocks_in_use() == 0);

        std::cout << "correctness: PASSED\n";
    }

    // ── Benchmark: MemoryPool vs malloc ─────────────────────────────
    {
        MemoryPool pool(BLOCK_SIZE, BLOCK_COUNT);

        // warm up
        for (size_t i = 0; i < 1000; i++) {
            void* p = pool.allocate();
            pool.deallocate(p);
        }

        // benchmark pool
        auto t1 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < ITERATIONS; i++) {
            void* p = pool.allocate();
            pool.deallocate(p);
        }
        auto t2 = std::chrono::high_resolution_clock::now();

        // benchmark malloc
        auto t3 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < ITERATIONS; i++) {
            void* p = malloc(BLOCK_SIZE);
            free(p);
        }
        auto t4 = std::chrono::high_resolution_clock::now();

        auto pool_ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        auto malloc_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();

        std::cout << "pool    total: " << pool_ns   << " ns  |  per-op: " << pool_ns / ITERATIONS   << " ns\n";
        std::cout << "malloc  total: " << malloc_ns  << " ns  |  per-op: " << malloc_ns / ITERATIONS  << " ns\n";
        std::cout << "speedup: " << (double)malloc_ns / (double)pool_ns << "x\n";
    }

    return 0;
}