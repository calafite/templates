#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#if defined(__GNUC__) || defined(__clang__)
#define CALAFITE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CALAFITE_UNLIKELY(x) (x)
#endif

namespace calafite {
    namespace arena {

        class Chunk {
          public:
            Chunk* next;
            size_t size;
            char* data() { return reinterpret_cast<char*>(this + 1); }
        };

        inline Chunk* head = nullptr;
        inline Chunk* current = nullptr;
        inline char* pointer = nullptr;
        inline char* end = nullptr;
        inline bool active = false;

        inline uintptr_t min_address = UINTPTR_MAX;
        inline uintptr_t max_address = 0;

        [[gnu::noinline]] inline void addChunk(size_t minimum) {
            size_t size = 1024 * 1024;
            if (current) size = current->size * 2;
            if (size < minimum) size = minimum;

            Chunk* chunk = static_cast<Chunk*>(std::malloc(sizeof(Chunk) + size));
            chunk->next = nullptr;
            chunk->size = size;

            if (current) current->next = chunk;
            if (!head) head = chunk;
            current = chunk;

            uintptr_t chunk_start = reinterpret_cast<uintptr_t>(chunk->data());
            uintptr_t chunk_end = chunk_start + size;
            if (chunk_start < min_address) min_address = chunk_start;
            if (chunk_end > max_address) max_address = chunk_end;
        }

        [[gnu::noinline]] inline void* allocate_slow_path(size_t size, size_t alignment) {
            if (current && current->next && current->next->size >= size + alignment) {
                current = current->next;
            } else {
                addChunk(size + alignment);
            }
            pointer = current->data();
            end = pointer + current->size;
            
            uintptr_t aligned = (reinterpret_cast<uintptr_t>(pointer) + alignment - 1) & ~(alignment - 1);
            pointer = reinterpret_cast<char*>(aligned + size);
            return reinterpret_cast<void*>(aligned);
        }

        inline void* allocate(size_t size, size_t alignment = 16) noexcept {
            uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
            uintptr_t aligned = (address + alignment - 1) & ~(alignment - 1);

            if (CALAFITE_UNLIKELY(aligned + size > reinterpret_cast<uintptr_t>(end))) {
                return allocate_slow_path(size, alignment);
            }

            pointer = reinterpret_cast<char*>(aligned + size);
            return reinterpret_cast<void*>(aligned);
        }

        inline void reset() noexcept {
            if (!head) return;
            current = head;
            pointer = current->data();
            end = pointer + current->size;
        }

        inline bool contains(void* target) noexcept {
            uintptr_t addr = reinterpret_cast<uintptr_t>(target);
            if (CALAFITE_UNLIKELY(addr >= min_address && addr < max_address)) {
                for (Chunk* chunk = head; chunk; chunk = chunk->next) {
                    if (target >= chunk->data() && target < chunk->data() + chunk->size) return true;
                }
            }
            return false;
        }

        class ScopedArena {
            ScopedArena() { active = true; }
            ~ScopedArena() {
                active = false;
                reset();
            }
        };
    }
}

#if defined(_WIN32)
#include <malloc.h>
#define CALAFITE_ALIGNED_ALLOC(alignment, size) _aligned_malloc(size, alignment)
#define CALAFITE_ALIGNED_FREE(pointer) _aligned_free(pointer)
#else
#define CALAFITE_ALIGNED_ALLOC(alignment, size) std::aligned_alloc(alignment, size)
#define CALAFITE_ALIGNED_FREE(pointer) std::free(pointer)
#endif

#define CALAFITE_ROUND_UP(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))

#define CALAFITE_MAKE_ARENA_GLOBAL                                             \
  void* operator new(size_t size) {                                            \
    if (calafite::arena::active) return calafite::arena::allocate(size);       \
    return std::malloc(size);                                                  \
  }                                                                            \
  void* operator new[](size_t size) {                                          \
    if (calafite::arena::active) return calafite::arena::allocate(size);       \
    return std::malloc(size);                                                  \
  }                                                                            \
  void* operator new(size_t size, std::align_val_t alignmentValue) {           \
    size_t alignment = static_cast<size_t>(alignmentValue);                    \
    if (calafite::arena::active) return calafite::arena::allocate(size, alignment); \
    return CALAFITE_ALIGNED_ALLOC(alignment, CALAFITE_ROUND_UP(size, alignment)); \
  }                                                                            \
  void* operator new[](size_t size, std::align_val_t alignmentValue) {         \
    size_t alignment = static_cast<size_t>(alignmentValue);                    \
    if (calafite::arena::active) return calafite::arena::allocate(size, alignment); \
    return CALAFITE_ALIGNED_ALLOC(alignment, CALAFITE_ROUND_UP(size, alignment)); \
  }                                                                            \
  void operator delete(void* pointer) noexcept {                               \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    std::free(pointer);                                                        \
  }                                                                            \
  void operator delete[](void* pointer) noexcept {                             \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    std::free(pointer);                                                        \
  }                                                                            \
  void operator delete(void* pointer, size_t) noexcept {                       \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    std::free(pointer);                                                        \
  }                                                                            \
  void operator delete[](void* pointer, size_t) noexcept {                     \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    std::free(pointer);                                                        \
  }                                                                            \
  void operator delete(void* pointer, std::align_val_t) noexcept {             \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    CALAFITE_ALIGNED_FREE(pointer);                                            \
  }                                                                            \
  void operator delete[](void* pointer, std::align_val_t) noexcept {           \
    if (!pointer || calafite::arena::active || calafite::arena::contains(pointer)) return; \
    CALAFITE_ALIGNED_FREE(pointer);                                            \
  }
