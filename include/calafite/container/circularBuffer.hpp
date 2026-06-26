#pragma once

#include "../core/arena.hpp"
#include "../core/iterator.hpp" // IWYU pragma: keep
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <utility>

#if defined(__has_include)
    #if __has_include(<version>)
        #include <version>
    #endif
#endif

#ifndef CALAFITE_UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define CALAFITE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CALAFITE_UNLIKELY(x) (x)
#endif
#endif

namespace calafite {
    namespace container {

        constexpr size_t nextPowerOfTwo(size_t n) {
            if (n == 0) return 1;
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            if constexpr (sizeof(size_t) >= 8) n |= n >> 32;
            return n + 1;
        }

        template<typename Type> struct CircularBuffer {
            using value_type = Type;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using reference = Type&;
            using const_reference = const Type&;
            using pointer = Type*;
            using const_pointer = const Type*;

            Type* data = nullptr;
            size_t headValue = 0;
            size_t tailValue = 0;
            size_t capacityValue = 0;
            size_t maskValue = 0;

            template<bool Const>
            struct IteratorImpl {
                using iterator_category = std::random_access_iterator_tag;
                using value_type = Type;
                using difference_type = ptrdiff_t;
                using pointer = std::conditional_t<Const, const Type*, Type*>;
                using reference = std::conditional_t<Const, const Type&, Type&>;

                Type* bufferData = nullptr;
                size_t maskValue = 0;
                size_t indexValue = 0;

                IteratorImpl() = default;
                IteratorImpl(Type* ptr, size_t mask, size_t idx) : bufferData(ptr), maskValue(mask), indexValue(idx) {}

                inline reference operator*() const { return bufferData[indexValue & maskValue]; }
                inline pointer operator->() const { return &bufferData[indexValue & maskValue]; }

                inline IteratorImpl& operator++() { ++indexValue; return *this; }
                inline IteratorImpl operator++(int) { IteratorImpl tmp = *this; ++indexValue; return tmp; }
                inline IteratorImpl& operator--() { --indexValue; return *this; }
                inline IteratorImpl operator--(int) { IteratorImpl tmp = *this; --indexValue; return tmp; }

                inline IteratorImpl& operator+=(difference_type offset) { indexValue += offset; return *this; }
                inline IteratorImpl operator+(difference_type offset) const { return IteratorImpl(bufferData, maskValue, indexValue + offset); }
                friend inline IteratorImpl operator+(difference_type offset, const IteratorImpl& it) { return it + offset; }

                inline IteratorImpl& operator-=(difference_type offset) { indexValue -= offset; return *this; }
                inline IteratorImpl operator-(difference_type offset) const { return IteratorImpl(bufferData, maskValue, indexValue - offset); }
                inline difference_type operator-(const IteratorImpl& other) const { return indexValue - other.indexValue; }

                inline reference operator[](difference_type offset) const { return bufferData[(indexValue + offset) & maskValue]; }

                inline bool operator==(const IteratorImpl& other) const { return indexValue == other.indexValue; }
                inline bool operator!=(const IteratorImpl& other) const { return indexValue != other.indexValue; }
                inline bool operator<(const IteratorImpl& other) const { return indexValue < other.indexValue; }
                inline bool operator>(const IteratorImpl& other) const { return indexValue > other.indexValue; }
                inline bool operator<=(const IteratorImpl& other) const { return indexValue <= other.indexValue; }
                inline bool operator>=(const IteratorImpl& other) const { return indexValue >= other.indexValue; }
            };

            using iterator = IteratorImpl<false>;
            using const_iterator = IteratorImpl<true>;

            CircularBuffer() = default;

            explicit CircularBuffer(size_t count) {
                if (count > 0) {
                    reserve(count);
                    tailValue = count;
                    if constexpr (!std::is_trivially_default_constructible_v<Type>) {
                        for (size_t index = 0; index < count; ++index) {
                            new (&data[index]) Type();
                        }
                    }
                }
            }

            CircularBuffer(size_t count, const Type& value) {
                if (count > 0) {
                    reserve(count);
                    tailValue = count;
                    for (size_t index = 0; index < count; ++index) {
                        new (&data[index]) Type(value);
                    }
                }
            }

            CircularBuffer(std::initializer_list<Type> initializer) {
                size_t count = initializer.size();
                if (count > 0) {
                    reserve(count);
                    for (const Type& value : initializer) {
                        new (&data[tailValue & maskValue]) Type(value);
                        ++tailValue;
                    }
                }
            }

            CircularBuffer(const CircularBuffer& other) {
                if (other.size() > 0) {
                    reserve(other.size());
                    tailValue = other.size();
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        size_t firstPart = other.capacityValue - (other.headValue & other.maskValue);
                        if (firstPart > tailValue) firstPart = tailValue;
                        std::memcpy(data, other.data + (other.headValue & other.maskValue), firstPart * sizeof(Type));
                        if (firstPart < tailValue) {
                            std::memcpy(data + firstPart, other.data, (tailValue - firstPart) * sizeof(Type));
                        }
                    } else {
                        for (size_t index = 0; index < tailValue; ++index) {
                            new (&data[index]) Type(other[index]);
                        }
                    }
                }
            }

            CircularBuffer(CircularBuffer&& other) noexcept : data(other.data), headValue(other.headValue), tailValue(other.tailValue), capacityValue(other.capacityValue), maskValue(other.maskValue) {
                other.data = nullptr;
                other.headValue = 0;
                other.tailValue = 0;
                other.capacityValue = 0;
                other.maskValue = 0;
            }

            CircularBuffer& operator=(const CircularBuffer& other) {
                if (this == &other) return *this;
                clear();
                if (other.size() > capacityValue) {
                    size_t newCapacity = nextPowerOfTwo(other.size());
                    data = static_cast<Type*>(arena::allocate(newCapacity * sizeof(Type), alignof(Type)));
                    capacityValue = newCapacity;
                    maskValue = newCapacity - 1;
                }
                headValue = 0;
                tailValue = other.size();
                if (tailValue > 0) {
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        size_t firstPart = other.capacityValue - (other.headValue & other.maskValue);
                        if (firstPart > tailValue) firstPart = tailValue;
                        std::memcpy(data, other.data + (other.headValue & other.maskValue), firstPart * sizeof(Type));
                        if (firstPart < tailValue) {
                            std::memcpy(data + firstPart, other.data, (tailValue - firstPart) * sizeof(Type));
                        }
                    } else {
                        for (size_t index = 0; index < tailValue; ++index) {
                            new (&data[index]) Type(other[index]);
                        }
                    }
                }
                return *this;
            }

            CircularBuffer& operator=(CircularBuffer&& other) noexcept {
                if (this == &other) return *this;
                clear();
                data = other.data;
                headValue = other.headValue;
                tailValue = other.tailValue;
                capacityValue = other.capacityValue;
                maskValue = other.maskValue;

                other.data = nullptr;
                other.headValue = 0;
                other.tailValue = 0;
                other.capacityValue = 0;
                other.maskValue = 0;
                return *this;
            }

            ~CircularBuffer() { clear(); }

            inline void reserve(size_t capacity) {
                if (capacity > capacityValue) {
                    size_t newCapacity = nextPowerOfTwo(capacity);
                    size_t newMask = newCapacity - 1;
                    Type* buffer = static_cast<Type*>(arena::allocate(newCapacity * sizeof(Type), alignof(Type)));
                    size_t currentSize = size();
                    
                    if (data && currentSize > 0) {
                        if constexpr (std::is_trivially_copyable_v<Type>) {
                            size_t firstPart = capacityValue - (headValue & maskValue);
                            if (firstPart > currentSize) firstPart = currentSize;
                            std::memcpy(buffer, data + (headValue & maskValue), firstPart * sizeof(Type));
                            if (firstPart < currentSize) {
                                std::memcpy(buffer + firstPart, data, (currentSize - firstPart) * sizeof(Type));
                            }
                        } else {
                            for (size_t index = 0; index < currentSize; ++index) {
                                size_t oldPos = (headValue + index) & maskValue;
                                new (&buffer[index]) Type(std::move(data[oldPos]));
                                if constexpr (!std::is_trivially_destructible_v<Type>) {
                                    data[oldPos].~Type();
                                }
                            }
                        }
                    }
                    data = buffer;
                    capacityValue = newCapacity;
                    maskValue = newMask;
                    headValue = 0;
                    tailValue = currentSize;
                }
            }

            inline void grow() { reserve(capacityValue == 0 ? 4 : capacityValue * 2); }

            inline void pushBack(const Type& value) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                new (&data[tailValue & maskValue]) Type(value);
                ++tailValue;
            }

            inline void pushBack(Type&& value) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                new (&data[tailValue & maskValue]) Type(std::move(value));
                ++tailValue;
            }

            inline void pushFront(const Type& value) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                --headValue;
                new (&data[headValue & maskValue]) Type(value);
            }

            inline void pushFront(Type&& value) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                --headValue;
                new (&data[headValue & maskValue]) Type(std::move(value));
            }

            template<typename... Args> inline void emplaceBack(Args&&... arguments) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                new (&data[tailValue & maskValue]) Type(std::forward<Args>(arguments)...);
                ++tailValue;
            }

            template<typename... Args> inline void emplaceFront(Args&&... arguments) {
                if (CALAFITE_UNLIKELY(size() == capacityValue)) grow();
                --headValue;
                new (&data[headValue & maskValue]) Type(std::forward<Args>(arguments)...);
            }

            inline void popBack() {
                assert(!empty());
                --tailValue;
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    data[tailValue & maskValue].~Type();
                }
            }

            inline void popFront() {
                assert(!empty());
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    data[headValue & maskValue].~Type();
                }
                ++headValue;
            }

            inline void clear() {
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    size_t currentSize = size();
                    for (size_t index = 0; index < currentSize; ++index) {
                        data[(headValue + index) & maskValue].~Type();
                    }
                }
                headValue = 0;
                tailValue = 0;
            }

            inline void resize(size_t count) {
                size_t currentSize = size();
                if (count < currentSize) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) {
                        for (size_t index = count; index < currentSize; ++index) {
                            data[(headValue + index) & maskValue].~Type();
                        }
                    }
                    tailValue = headValue + count;
                } else if (count > currentSize) {
                    reserve(count);
                    if constexpr (!std::is_trivially_default_constructible_v<Type>) {
                        for (size_t index = currentSize; index < count; ++index) {
                            new (&data[(headValue + index) & maskValue]) Type();
                        }
                    }
                    tailValue = headValue + count;
                }
            }

            inline void assign(size_t count, const Type& value) {
                Type copy = value;
                clear();
                reserve(count);
                tailValue = headValue + count;
                for (size_t index = 0; index < count; ++index) {
                    new (&data[(headValue + index) & maskValue]) Type(copy);
                }
            }
            
            inline void linearize() {
                if (empty() || (headValue & maskValue) == 0) return;
                
                size_t currentSize = size();
                Type* buffer = static_cast<Type*>(arena::allocate(capacityValue * sizeof(Type), alignof(Type)));
                
                if constexpr (std::is_trivially_copyable_v<Type>) {
                    size_t firstPart = capacityValue - (headValue & maskValue);
                    if (firstPart > currentSize) firstPart = currentSize;
                    std::memcpy(buffer, data + (headValue & maskValue), firstPart * sizeof(Type));
                    if (firstPart < currentSize) {
                        std::memcpy(buffer + firstPart, data, (currentSize - firstPart) * sizeof(Type));
                    }
                } else {
                    for (size_t index = 0; index < currentSize; ++index) {
                        size_t oldPos = (headValue + index) & maskValue;
                        new (&buffer[index]) Type(std::move(data[oldPos]));
                        if constexpr (!std::is_trivially_destructible_v<Type>) {
                            data[oldPos].~Type();
                        }
                    }
                }
                data = buffer;
                headValue = 0;
                tailValue = currentSize;
            }

            inline Type& operator[](size_t index) {
                assert(index < size());
                return data[(headValue + index) & maskValue];
            }
            
            inline const Type& operator[](size_t index) const {
                assert(index < size());
                return data[(headValue + index) & maskValue];
            }

            inline Type& front() {
                assert(!empty());
                return data[headValue & maskValue];
            }
            
            inline const Type& front() const {
                assert(!empty());
                return data[headValue & maskValue];
            }
            
            inline Type& back() {
                assert(!empty());
                return data[(tailValue - 1) & maskValue];
            }
            
            inline const Type& back() const {
                assert(!empty());
                return data[(tailValue - 1) & maskValue];
            }

            inline iterator begin() { return iterator(data, maskValue, headValue); }
            inline iterator end() { return iterator(data, maskValue, tailValue); }
            inline const_iterator begin() const { return const_iterator(data, maskValue, headValue); }
            inline const_iterator end() const { return const_iterator(data, maskValue, tailValue); }
            inline const_iterator cbegin() const { return const_iterator(data, maskValue, headValue); }
            inline const_iterator cend() const { return const_iterator(data, maskValue, tailValue); }

            inline size_t size() const { return tailValue - headValue; }
            inline bool empty() const { return headValue == tailValue; }

            inline void reverse() { std::reverse(begin(), end()); }

            inline size_t find(const Type& value) const {
                for (size_t index = 0; index < size(); ++index) {
                    if (data[(headValue + index) & maskValue] == value) return index;
                }
                return static_cast<size_t>(-1);
            }

            template<typename Predicate> inline size_t findIf(Predicate&& predicate) const {
                for (size_t index = 0; index < size(); ++index) {
                    if (predicate(data[(headValue + index) & maskValue])) return index;
                }
                return static_cast<size_t>(-1);
            }

            inline bool contains(const Type& value) const { return find(value) != static_cast<size_t>(-1); }

            inline void replace(const Type& oldValue, const Type& newValue) {
                for (size_t index = 0; index < size(); ++index) {
                    size_t pos = (headValue + index) & maskValue;
                    if (data[pos] == oldValue) data[pos] = newValue;
                }
            }

            template<typename Function> inline auto map(Function&& function) const {
                using ResultType = std::remove_cv_t<std::remove_reference_t<decltype(function(data[0]))>>;
                CircularBuffer<ResultType> result;
                result.reserve(size());
                for (size_t index = 0; index < size(); ++index) {
                    result.pushBack(function(data[(headValue + index) & maskValue]));
                }
                return result;
            }

            template<typename Predicate> inline CircularBuffer<Type> filter(Predicate&& predicate) const {
                CircularBuffer<Type> result;
                for (size_t index = 0; index < size(); ++index) {
                    size_t pos = (headValue + index) & maskValue;
                    if (predicate(data[pos])) result.pushBack(data[pos]);
                }
                return result;
            }

            template<typename Accumulator, typename Function> inline Accumulator reduce(Accumulator initial, Function&& function) const {
                Accumulator result = initial;
                for (size_t index = 0; index < size(); ++index) {
                    result = function(result, data[(headValue + index) & maskValue]);
                }
                return result;
            }

            inline size_t count(const Type& value) const {
                size_t occurrence = 0;
                for (size_t index = 0; index < size(); ++index) {
                    if (data[(headValue + index) & maskValue] == value) ++occurrence;
                }
                return occurrence;
            }

            inline void unique() {
                if (empty()) return;
                auto newEnd = std::unique(begin(), end());
                tailValue = headValue + (newEnd - begin());
            }

            inline void sort() {
                if (size() < 2) return;
                linearize(); 

                if constexpr (std::is_integral_v<Type> && (sizeof(Type) == 4 || sizeof(Type) == 8)) {
                    using UnsignedType = std::make_unsigned_t<Type>;
                    UnsignedType* source = reinterpret_cast<UnsignedType*>(data);

                    UnsignedType* destination = static_cast<UnsignedType*>(std::malloc(size() * sizeof(Type)));
                    if (!destination) {
                        std::sort(begin(), end());
                        return;
                    }

                    constexpr int passes = sizeof(Type);
                    for (int shift = 0; shift < passes * 8; shift += 8) {
                        uint32_t occurrences[256] = {0};
                        bool isLastPass = (shift == (passes - 1) * 8) && std::is_signed_v<Type>;

                        for (size_t index = 0; index < size(); ++index) {
                            uint8_t byte = (source[index] >> shift) & 0xFF;
                            if (isLastPass) byte ^= 128;
                            occurrences[byte]++;
                        }

                        uint32_t positions[256];
                        positions[0] = 0;
                        for (size_t index = 1; index < 256; ++index) {
                            positions[index] = positions[index - 1] + occurrences[index - 1];
                        }

                        for (size_t index = 0; index < size(); ++index) {
                            uint8_t byte = (source[index] >> shift) & 0xFF;
                            if (isLastPass) byte ^= 128;
                            destination[positions[byte]++] = source[index];
                        }
                        std::swap(source, destination);
                    }
                    std::free(destination);
                } else {
                    std::sort(begin(), end());
                }
            }

            template<typename Compare> inline void sort(Compare comparison) {
                std::sort(begin(), end(), comparison);
            }

            #if defined(__cpp_lib_ranges)
            inline auto iterate() {
                auto v = std::views::all(*this);
                return ::calafite::core::IteratorPipeline<decltype(v)>(std::move(v));
            }

            inline auto iterate() const {
                auto v = std::views::all(*this);
                return ::calafite::core::IteratorPipeline<decltype(v)>(std::move(v));
            }
            #endif
        };
    }
}
