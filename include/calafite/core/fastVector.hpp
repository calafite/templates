#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

#include "arena.hpp" 

#if defined(__has_include)
    #if __has_include(<version>)
        #include <version>
    #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CALAFITE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CALAFITE_UNLIKELY(x) (x)
#endif

namespace calafite {
    namespace core {

        template <typename Type>
        class FastVector {
        public:
            using iterator = Type*;
            using const_iterator = const Type*;

        private:
            Type* pointer = nullptr;
            size_t sizeValue = 0;
            size_t capacityValue = 0;

            [[gnu::noinline]] void grow(size_t minimumCapacity = 0) {
                size_t newCapacity = capacityValue == 0 ? 4 : capacityValue * 2;
                if (newCapacity < minimumCapacity) newCapacity = minimumCapacity;

                if (calafite::arena::active && pointer) {
                    char* allocationTail = reinterpret_cast<char*>(pointer) + (capacityValue * sizeof(Type));
                    if (allocationTail == calafite::arena::pointer) {
                        size_t bytesNeeded = (newCapacity - capacityValue) * sizeof(Type);
                        if (calafite::arena::pointer + bytesNeeded <= calafite::arena::end) {
                            calafite::arena::pointer += bytesNeeded;
                            capacityValue = newCapacity;
                            return;
                        }
                    }
                }

                Type* newPointer = static_cast<Type*>(::operator new[](newCapacity * sizeof(Type)));

                if (pointer) {
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        std::memcpy(newPointer, pointer, sizeValue * sizeof(Type));
                    } else {
                        for (size_t i = 0; i < sizeValue; ++i) {
                            new (&newPointer[i]) Type(std::move(pointer[i]));
                            pointer[i].~Type();
                        }
                    }
                    ::operator delete[](pointer);
                }

                pointer = newPointer;
                capacityValue = newCapacity;
            }

        public:
            FastVector() = default;

            explicit FastVector(size_t initialSize) {
                if (initialSize > 0) {
                    pointer = static_cast<Type*>(::operator new[](initialSize * sizeof(Type)));
                    sizeValue = initialSize;
                    capacityValue = initialSize;
                }
            }

            ~FastVector() {
                if (pointer) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) {
                        for (size_t i = 0; i < sizeValue; ++i) {
                            pointer[i].~Type();
                        }
                    }
                    ::operator delete[](pointer);
                }
            }

            FastVector(const FastVector&) = delete;
            FastVector& operator=(const FastVector&) = delete;

            FastVector(FastVector&& other) noexcept
                : pointer(other.pointer), sizeValue(other.sizeValue), capacityValue(other.capacityValue) {
                other.pointer = nullptr;
                other.sizeValue = 0;
                other.capacityValue = 0;
            }

            FastVector& operator=(FastVector&& other) noexcept {
                if (this != &other) {
                    this->~FastVector();
                    pointer = other.pointer;
                    sizeValue = other.sizeValue;
                    capacityValue = other.capacityValue;
                    other.pointer = nullptr;
                    other.sizeValue = 0;
                    other.capacityValue = 0;
                }
                return *this;
            }

            inline void pushBack(const Type& value) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&pointer[sizeValue++]) Type(value);
            }

            inline void pushBack(Type&& value) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&pointer[sizeValue++]) Type(std::move(value));
            }

            template <typename... Args>
            inline Type& emplaceBack(Args&&... args) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&pointer[sizeValue]) Type(std::forward<Args>(args)...);
                return pointer[sizeValue++];
            }

            inline void reserve(size_t newCapacity) {
                if (newCapacity > capacityValue) grow(newCapacity);
            }

            inline void resize(size_t newSize) {
                if (newSize > capacityValue) grow(newSize);

                if (newSize > sizeValue) {
                    if constexpr (!std::is_trivial_v<Type>) {
                        for (size_t i = sizeValue; i < newSize; ++i) {
                            new (&pointer[i]) Type();
                        }
                    }
                } else {
                    if constexpr (!std::is_trivially_destructible_v<Type>) {
                        for (size_t i = newSize; i < sizeValue; ++i) {
                            pointer[i].~Type();
                        }
                    }
                }
                sizeValue = newSize;
            }

            inline void clear() {
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    for (size_t i = 0; i < sizeValue; ++i) {
                        pointer[i].~Type();
                    }
                }
                sizeValue = 0;
            }

            inline Type& operator[](size_t index) {
                assert(index < sizeValue);
                return pointer[index];
            }

            inline const Type& operator[](size_t index) const {
                assert(index < sizeValue);
                return pointer[index];
            }

            inline Type& back() {
                assert(sizeValue > 0);
                return pointer[sizeValue - 1];
            }

            inline size_t size() const { return sizeValue; }
            inline size_t capacity() const { return capacityValue; }
            inline bool empty() const { return sizeValue == 0; }
            inline Type* data() { return pointer; }

            inline iterator begin() { return iterator(pointer); }
            inline iterator end() { return iterator(pointer + sizeValue); }
            inline const_iterator begin() const { return const_iterator(pointer); }
            inline const_iterator end() const { return const_iterator(pointer + sizeValue); }
        };

    }
}

