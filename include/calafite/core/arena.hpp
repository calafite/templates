#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace calafite {
namespace arena {

struct Chunk {
  Chunk *next;
  size_t size;
  char *data() { return reinterpret_cast<char *>(this + 1); }
};

inline Chunk *head = nullptr;
inline Chunk *current_chunk = nullptr;
inline char *ptr = nullptr;
inline char *end = nullptr;
inline bool active = false;

inline void add_chunk(size_t min_size) {
  size_t new_size = 1024 * 1024; // 1MB
  if (current_chunk)
    new_size = current_chunk->size * 2;
  if (new_size < min_size)
    new_size = min_size;

  Chunk *c = static_cast<Chunk *>(std::malloc(sizeof(Chunk) + new_size));
  c->next = nullptr;
  c->size = new_size;

  if (current_chunk)
    current_chunk->next = c;
  if (!head)
    head = c;
  current_chunk = c;
}

inline void *allocate(size_t size, size_t align = 16) noexcept {
  uintptr_t int_ptr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t aligned_ptr = (int_ptr + align - 1) & ~(align - 1);

  if (aligned_ptr + size > reinterpret_cast<uintptr_t>(end)) {
    if (current_chunk && current_chunk->next &&
        current_chunk->next->size >= size + align) {
      current_chunk = current_chunk->next;
    } else {
      add_chunk(size + align);
    }
    ptr = current_chunk->data();
    end = ptr + current_chunk->size;
    aligned_ptr = (reinterpret_cast<uintptr_t>(ptr) + align - 1) & ~(align - 1);
  }

  ptr = reinterpret_cast<char *>(aligned_ptr + size);
  return reinterpret_cast<void *>(aligned_ptr);
}

inline void reset() noexcept {
  if (!head)
    return;
  current_chunk = head;
  ptr = current_chunk->data();
  end = ptr + current_chunk->size;
}

inline bool contains(void *p) noexcept {
  for (Chunk *c = head; c; c = c->next) {
    if (p >= c->data() && p < c->data() + c->size)
      return true;
  }
  return false;
}

struct ScopedArena {
  ScopedArena() { active = true; }
  ~ScopedArena() {
    active = false;
    reset();
  }
};

} // namespace arena
} // namespace calafite

#if defined(_WIN32)
#include <malloc.h>
#define CALAFITE_ALIGNED_ALLOC(align, size) _aligned_malloc(size, align)
#define CALAFITE_ALIGNED_FREE(ptr) _aligned_free(ptr)
#else
#define CALAFITE_ALIGNED_ALLOC(align, size) std::aligned_alloc(align, size)
#define CALAFITE_ALIGNED_FREE(ptr) std::free(ptr)
#endif

#define CALAFITE_MAKE_ARENA_GLOBAL                                             \
  void *operator new(size_t size) {                                            \
    if (calafite::arena::active)                                               \
      return calafite::arena::allocate(size);                                  \
    return std::malloc(size);                                                  \
  }                                                                            \
  void *operator new[](size_t size) {                                          \
    if (calafite::arena::active)                                               \
      return calafite::arena::allocate(size);                                  \
    return std::malloc(size);                                                  \
  }                                                                            \
  void *operator new(size_t size, std::align_val_t al) {                       \
    if (calafite::arena::active)                                               \
      return calafite::arena::allocate(size, static_cast<size_t>(al));         \
    size_t align = static_cast<size_t>(al);                                    \
    size_t rem = size % align;                                                 \
    return CALAFITE_ALIGNED_ALLOC(align,                                       \
                                  rem == 0 ? size : size + (align - rem));     \
  }                                                                            \
  void *operator new[](size_t size, std::align_val_t al) {                     \
    if (calafite::arena::active)                                               \
      return calafite::arena::allocate(size, static_cast<size_t>(al));         \
    size_t align = static_cast<size_t>(al);                                    \
    size_t rem = size % align;                                                 \
    return CALAFITE_ALIGNED_ALLOC(align,                                       \
                                  rem == 0 ? size : size + (align - rem));     \
  }                                                                            \
  void operator delete(void *p) noexcept {                                     \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    std::free(p);                                                              \
  }                                                                            \
  void operator delete[](void *p) noexcept {                                   \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    std::free(p);                                                              \
  }                                                                            \
  void operator delete(void *p, size_t) noexcept {                             \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    std::free(p);                                                              \
  }                                                                            \
  void operator delete[](void *p, size_t) noexcept {                           \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    std::free(p);                                                              \
  }                                                                            \
  void operator delete(void *p, std::align_val_t) noexcept {                   \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    CALAFITE_ALIGNED_FREE(p);                                                  \
  }                                                                            \
  void operator delete[](void *p, std::align_val_t) noexcept {                 \
    if (!p || calafite::arena::active || calafite::arena::contains(p))         \
      return;                                                                  \
    CALAFITE_ALIGNED_FREE(p);                                                  \
  }
