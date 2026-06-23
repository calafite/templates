#pragma once

#include "arena.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <utility>

#ifndef CALAFITE_UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define CALAFITE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CALAFITE_UNLIKELY(x) (x)
#endif
#endif

namespace calafite {

template <typename T> struct fvec {
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using iterator = T *;
  using const_iterator = const T *;

  T *data_ptr = nullptr;
  size_t sz = 0;
  size_t cap = 0;

  fvec() = default;

  explicit fvec(size_t n) {
    if (n > 0) {
      data_ptr = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
      sz = cap = n;
      if constexpr (!std::is_trivially_default_constructible_v<T>) {
        for (size_t i = 0; i < n; i++) new (&data_ptr[i]) T();
      }
    }
  }

  fvec(size_t n, const T &val) {
    if (n > 0) {
      data_ptr = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
      sz = cap = n;
      for (size_t i = 0; i < n; i++) new (&data_ptr[i]) T(val);
    }
  }

  fvec(std::initializer_list<T> init) {
    size_t n = init.size();
    if (n > 0) {
      data_ptr = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
      sz = cap = n;
      size_t i = 0;
      for (const T &val : init) new (&data_ptr[i++]) T(val);
    }
  }

  fvec(const fvec &other) {
    if (other.sz > 0) {
      data_ptr = static_cast<T *>(arena::allocate(other.sz * sizeof(T), alignof(T)));
      sz = cap = other.sz;
      if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(data_ptr, other.data_ptr, sz * sizeof(T));
      } else {
        for (size_t i = 0; i < sz; i++) new (&data_ptr[i]) T(other.data_ptr[i]);
      }
    }
  }

  fvec(fvec &&other) noexcept
      : data_ptr(other.data_ptr), sz(other.sz), cap(other.cap) {
    other.data_ptr = nullptr;
    other.sz = other.cap = 0;
  }

  fvec &operator=(const fvec &other) {
    if (this == &other) return *this;
    clear();
    if (other.sz > cap) {
      data_ptr = static_cast<T *>(arena::allocate(other.sz * sizeof(T), alignof(T)));
      cap = other.sz;
    }
    sz = other.sz;
    if (sz > 0) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(data_ptr, other.data_ptr, sz * sizeof(T));
      } else {
        for (size_t i = 0; i < sz; i++) new (&data_ptr[i]) T(other.data_ptr[i]);
      }
    }
    return *this;
  }

  fvec &operator=(fvec &&other) noexcept {
    if (this == &other) return *this;
    clear();
    data_ptr = other.data_ptr;
    sz = other.sz;
    cap = other.cap;
    other.data_ptr = nullptr;
    other.sz = other.cap = 0;
    return *this;
  }

  ~fvec() { clear(); }

  inline void reserve(size_t n) {
    if (n > cap) {
      T *new_data = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
      if (data_ptr && sz > 0) {
        if constexpr (std::is_trivially_copyable_v<T>) {
          std::memcpy(new_data, data_ptr, sz * sizeof(T));
        } else {
          for (size_t i = 0; i < sz; i++) {
            new (&new_data[i]) T(std::move(data_ptr[i]));
            if constexpr (!std::is_trivially_destructible_v<T>) data_ptr[i].~T();
          }
        }
      }
      data_ptr = new_data;
      cap = n;
    }
  }

  inline void grow() {
    size_t new_cap = cap == 0 ? 4 : cap * 2;
    T *new_data = static_cast<T *>(arena::allocate(new_cap * sizeof(T), alignof(T)));
    if (data_ptr) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(new_data, data_ptr, sz * sizeof(T));
      } else {
        for (size_t i = 0; i < sz; i++) {
          new (&new_data[i]) T(std::move(data_ptr[i]));
          if constexpr (!std::is_trivially_destructible_v<T>) data_ptr[i].~T();
        }
      }
    }
    data_ptr = new_data;
    cap = new_cap;
  }

  inline void push_back(const T &val) {
    if (CALAFITE_UNLIKELY(sz == cap)) grow();
    new (&data_ptr[sz++]) T(val);
  }

  inline void push_back(T &&val) {
    if (CALAFITE_UNLIKELY(sz == cap)) grow();
    new (&data_ptr[sz++]) T(std::move(val));
  }

  template <typename... Args> inline void emplace_back(Args &&...args) {
    if (CALAFITE_UNLIKELY(sz == cap)) grow();
    new (&data_ptr[sz++]) T(std::forward<Args>(args)...);
  }

  inline void pop_back() {
    assert(sz > 0);
    if constexpr (!std::is_trivially_destructible_v<T>) data_ptr[sz - 1].~T();
    sz--;
  }

  inline void clear() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < sz; i++) data_ptr[i].~T();
    }
    sz = 0;
  }

  inline void resize(size_t n) {
    if (n < sz) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (size_t i = n; i < sz; i++) data_ptr[i].~T();
      }
    } else if (n > sz) {
      if (n > cap) {
        T *new_data = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
        if (data_ptr && sz > 0) {
          if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(new_data, data_ptr, sz * sizeof(T));
          } else {
            for (size_t i = 0; i < sz; i++) {
              new (&new_data[i]) T(std::move(data_ptr[i]));
              if constexpr (!std::is_trivially_destructible_v<T>) data_ptr[i].~T();
            }
          }
        }
        data_ptr = new_data;
        cap = n;
      }
      if constexpr (!std::is_trivially_default_constructible_v<T>) {
        for (size_t i = sz; i < n; i++) new (&data_ptr[i]) T();
      }
    }
    sz = n;
  }

  inline void assign(size_t n, const T &val) {
    T copy = val;
    clear();
    if (n > cap) {
      data_ptr = static_cast<T *>(arena::allocate(n * sizeof(T), alignof(T)));
      cap = n;
    }
    sz = n;
    for (size_t i = 0; i < n; i++) new (&data_ptr[i]) T(copy);
  }

  inline T &operator[](size_t i) { assert(i < sz); return data_ptr[i]; }
  inline const T &operator[](size_t i) const { assert(i < sz); return data_ptr[i]; }

  inline T &front() { assert(sz > 0); return data_ptr[0]; }
  inline const T &front() const { assert(sz > 0); return data_ptr[0]; }
  inline T &back() { assert(sz > 0); return data_ptr[sz - 1]; }
  inline const T &back() const { assert(sz > 0); return data_ptr[sz - 1]; }

  inline T *begin() { return data_ptr; }
  inline T *end() { return data_ptr + sz; }
  inline const T *begin() const { return data_ptr; }
  inline const T *end() const { return data_ptr + sz; }

  inline size_t size() const { return sz; }
  inline bool empty() const { return sz == 0; }

  inline void reverse() { std::reverse(begin(), end()); }

  inline void unique() {
    if (sz == 0) return;
    sz = std::unique(begin(), end()) - begin();
  }

  inline void sort() {
    if (sz < 2) return;

    if constexpr (std::is_integral_v<T> && (sizeof(T) == 4 || sizeof(T) == 8)) {
      using U = std::make_unsigned_t<T>;
      U *src = reinterpret_cast<U *>(data_ptr);
      
      U *dst = static_cast<U *>(std::malloc(sz * sizeof(T)));
      if (!dst) {
        std::sort(begin(), end());
        return;
      }

      constexpr int passes = sizeof(T);
      for (int shift = 0; shift < passes * 8; shift += 8) {
        uint32_t count[256] = {0};
        bool is_last_pass = (shift == (passes - 1) * 8) && std::is_signed_v<T>;

        for (size_t i = 0; i < sz; i++) {
          uint8_t byte = (src[i] >> shift) & 0xFF;
          if (is_last_pass) byte ^= 128;
          count[byte]++;
        }

        uint32_t pos[256];
        pos[0] = 0;
        for (int i = 1; i < 256; i++) pos[i] = pos[i - 1] + count[i - 1];

        for (size_t i = 0; i < sz; i++) {
          uint8_t byte = (src[i] >> shift) & 0xFF;
          if (is_last_pass) byte ^= 128;
          dst[pos[byte]++] = src[i];
        }
        std::swap(src, dst);
      }
      std::free(dst);  
    } else {
      std::sort(begin(), end());
    }
  }

  template <typename Compare> inline void sort(Compare comp) {
    std::sort(begin(), end(), comp);
  }
};

} // namespace calafite
