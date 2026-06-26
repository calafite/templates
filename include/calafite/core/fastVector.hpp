#pragma once

#include "arena.hpp"
#include "iterator.hpp" // IWYU pragma: keep
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
    namespace core {

        template<typename Type> struct FastVector {
            using value_type = Type;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using reference = Type&;
            using const_reference = const Type&;
            using pointer = Type*;
            using const_pointer = const Type*;

            Type* data = nullptr;
            size_t sizeValue = 0;
            size_t capacityValue = 0;

            FastVector() = default;

            explicit FastVector(size_t count) {
                if (count > 0) {
                    data = static_cast<Type*>(arena::allocate(count * sizeof(Type), alignof(Type)));
                    sizeValue = count;
                    capacityValue = count;
                    if constexpr (!std::is_trivially_default_constructible_v<Type>) {
                        for (size_t index = 0; index < count; ++index) {
                            new (&data[index]) Type();
                        }
                    }
                }
            }

            FastVector(size_t count, const Type& value) {
                if (count > 0) {
                    data = static_cast<Type*>(arena::allocate(count * sizeof(Type), alignof(Type)));
                    sizeValue = count;
                    capacityValue = count;
                    for (size_t index = 0; index < count; ++index) {
                        new (&data[index]) Type(value);
                    }
                }
            }

            FastVector(std::initializer_list<Type> initializer) {
                size_t count = initializer.size();
                if (count > 0) {
                    data = static_cast<Type*>(arena::allocate(count * sizeof(Type), alignof(Type)));
                    sizeValue = count;
                    capacityValue = count;
                    size_t index = 0;
                    for (const Type& value : initializer) {
                        new (&data[index++]) Type(value);
                    }
                }
            }

            FastVector(const FastVector& other) {
                if (other.sizeValue > 0) {
                    data = static_cast<Type*>(arena::allocate(other.sizeValue * sizeof(Type), alignof(Type)));
                    sizeValue = other.sizeValue;
                    capacityValue = other.sizeValue;
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        std::memcpy(data, other.data, sizeValue * sizeof(Type));
                    } else {
                        for (size_t index = 0; index < sizeValue; ++index) {
                            new (&data[index]) Type(other.data[index]);
                        }
                    }
                }
            }

            FastVector(FastVector&& other) noexcept : data(other.data), sizeValue(other.sizeValue), capacityValue(other.capacityValue) {
                other.data = nullptr;
                other.sizeValue = 0;
                other.capacityValue = 0;
            }

            FastVector& operator=(const FastVector& other) {
                if (this == &other) return *this;
                clear();
                if (other.sizeValue > capacityValue) {
                    data = static_cast<Type*>(arena::allocate(other.sizeValue * sizeof(Type), alignof(Type)));
                    capacityValue = other.sizeValue;
                }
                sizeValue = other.sizeValue;
                if (sizeValue > 0) {
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        std::memcpy(data, other.data, sizeValue * sizeof(Type));
                    } else {
                        for (size_t index = 0; index < sizeValue; ++index) {
                            new (&data[index]) Type(other.data[index]);
                        }
                    }
                }
                return *this;
            }

            FastVector& operator=(FastVector&& other) noexcept {
                if (this == &other) return *this;
                clear();
                data = other.data;
                sizeValue = other.sizeValue;
                capacityValue = other.capacityValue;
                other.data = nullptr;
                other.sizeValue = 0;
                other.capacityValue = 0;
                return *this;
            }

            ~FastVector() { clear(); }

            inline void reserve(size_t capacity) {
                if (capacity > capacityValue) {
                    Type* buffer = static_cast<Type*>(arena::allocate(capacity * sizeof(Type), alignof(Type)));
                    if (data && sizeValue > 0) {
                        if constexpr (std::is_trivially_copyable_v<Type>) {
                            std::memcpy(buffer, data, sizeValue * sizeof(Type));
                        } else {
                            for (size_t index = 0; index < sizeValue; ++index) {
                                new (&buffer[index]) Type(std::move(data[index]));
                                if constexpr (!std::is_trivially_destructible_v<Type>) {
                                    data[index].~Type();
                                }
                            }
                        }
                    }
                    data = buffer;
                    capacityValue = capacity;
                }
            }

            inline void grow() {
                size_t newCapacity = capacityValue == 0 ? 4 : capacityValue * 2;
                Type* buffer = static_cast<Type*>(arena::allocate(newCapacity * sizeof(Type), alignof(Type)));
                if (data) {
                    if constexpr (std::is_trivially_copyable_v<Type>) {
                        std::memcpy(buffer, data, sizeValue * sizeof(Type));
                    } else {
                        for (size_t index = 0; index < sizeValue; ++index) {
                            new (&buffer[index]) Type(std::move(data[index]));
                            if constexpr (!std::is_trivially_destructible_v<Type>) {
                                data[index].~Type();
                            }
                        }
                    }
                }
                data = buffer;
                capacityValue = newCapacity;
            }

            inline void pushBack(const Type& value) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&data[sizeValue++]) Type(value);
            }

            inline void pushBack(Type&& value) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&data[sizeValue++]) Type(std::move(value));
            }

            template<typename... Args> inline void emplaceBack(Args&&... arguments) {
                if (CALAFITE_UNLIKELY(sizeValue == capacityValue)) grow();
                new (&data[sizeValue++]) Type(std::forward<Args>(arguments)...);
            }

            inline void popBack() {
                assert(sizeValue > 0);
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    data[sizeValue - 1].~Type();
                }
                --sizeValue;
            }

            inline void clear() {
                if constexpr (!std::is_trivially_destructible_v<Type>) {
                    for (size_t index = 0; index < sizeValue; ++index) {
                        data[index].~Type();
                    }
                }
                sizeValue = 0;
            }

            inline void resize(size_t count) {
                if (count < sizeValue) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) {
                        for (size_t index = count; index < sizeValue; ++index) {
                            data[index].~Type();
                        }
                    }
                } else if (count > sizeValue) {
                    if (count > capacityValue) {
                        Type* buffer = static_cast<Type*>(arena::allocate(count * sizeof(Type), alignof(Type)));
                        if (data && sizeValue > 0) {
                            if constexpr (std::is_trivially_copyable_v<Type>) {
                                std::memcpy(buffer, data, sizeValue * sizeof(Type));
                            } else {
                                for (size_t index = 0; index < sizeValue; ++index) {
                                    new (&buffer[index]) Type(std::move(data[index]));
                                    if constexpr (!std::is_trivially_destructible_v<Type>) {
                                        data[index].~Type();
                                    }
                                }
                            }
                        }
                        data = buffer;
                        capacityValue = count;
                    }
                    if constexpr (!std::is_trivially_default_constructible_v<Type>) {
                        for (size_t index = sizeValue; index < count; ++index) {
                            new (&data[index]) Type();
                        }
                    }
                }
                sizeValue = count;
            }

            inline void assign(size_t count, const Type& value) {
                Type copy = value;
                clear();
                if (count > capacityValue) {
                    data = static_cast<Type*>(arena::allocate(count * sizeof(Type), alignof(Type)));
                    capacityValue = count;
                }
                sizeValue = count;
                for (size_t index = 0; index < count; ++index) {
                    new (&data[index]) Type(copy);
                }
            }

            inline Type& operator[](size_t index) {
                assert(index < sizeValue);
                return data[index];
            }
            
            inline const Type& operator[](size_t index) const {
                assert(index < sizeValue);
                return data[index];
            }

            inline Type& front() {
                assert(sizeValue > 0);
                return data[0];
            }
            
            inline const Type& front() const {
                assert(sizeValue > 0);
                return data[0];
            }
            
            inline Type& back() {
                assert(sizeValue > 0);
                return data[sizeValue - 1];
            }
            
            inline const Type& back() const {
                assert(sizeValue > 0);
                return data[sizeValue - 1];
            }

            inline Type* begin() { return data; }
            inline Type* end() { return data + sizeValue; }
            inline const Type* begin() const { return data; }
            inline const Type* end() const { return data + sizeValue; }

            inline size_t size() const { return sizeValue; }
            inline bool empty() const { return sizeValue == 0; }

            inline void reverse() { std::reverse(begin(), end()); }

            inline size_t find(const Type& value) const {
                for (size_t index = 0; index < sizeValue; ++index) {
                    if (data[index] == value) return index;
                }
                return static_cast<size_t>(-1);
            }

            template<typename Predicate> inline size_t findIf(Predicate&& predicate) const {
                for (size_t index = 0; index < sizeValue; ++index) {
                    if (predicate(data[index])) return index;
                }
                return static_cast<size_t>(-1);
            }

            inline bool contains(const Type& value) const { return find(value) != static_cast<size_t>(-1); }

            inline void replace(const Type& oldValue, const Type& newValue) {
                for (size_t index = 0; index < sizeValue; ++index) {
                    if (data[index] == oldValue) data[index] = newValue;
                }
            }

            template<typename Function> inline auto map(Function&& function) const {
                using ResultType = std::remove_cv_t<std::remove_reference_t<decltype(function(data[0]))>>;
                FastVector<ResultType> result;
                result.reserve(sizeValue);
                for (size_t index = 0; index < sizeValue; ++index) {
                    result.pushBack(function(data[index]));
                }
                return result;
            }

            template<typename Predicate> inline FastVector<Type> filter(Predicate&& predicate) const {
                FastVector<Type> result;
                for (size_t index = 0; index < sizeValue; ++index) {
                    if (predicate(data[index])) result.pushBack(data[index]);
                }
                return result;
            }

            template<typename Accumulator, typename Function> inline Accumulator reduce(Accumulator initial, Function&& function) const {
                Accumulator result = initial;
                for (size_t index = 0; index < sizeValue; ++index) {
                    result = function(result, data[index]);
                }
                return result;
            }

            inline size_t count(const Type& value) const {
                size_t occurrence = 0;
                for (size_t index = 0; index < sizeValue; ++index) {
                    if (data[index] == value) ++occurrence;
                }
                return occurrence;
            }

            inline void unique() {
                if (sizeValue == 0) return;
                sizeValue = std::unique(begin(), end()) - begin();
            }

            inline void sort() {
                if (sizeValue < 2) return;

                if constexpr (std::is_integral_v<Type> && (sizeof(Type) == 4 || sizeof(Type) == 8)) {
                    using UnsignedType = std::make_unsigned_t<Type>;
                    UnsignedType* source = reinterpret_cast<UnsignedType*>(data);

                    UnsignedType* destination = static_cast<UnsignedType*>(std::malloc(sizeValue * sizeof(Type)));
                    if (!destination) {
                        std::sort(begin(), end());
                        return;
                    }

                    constexpr int passes = sizeof(Type);
                    for (int shift = 0; shift < passes * 8; shift += 8) {
                        uint32_t occurrences[256] = {0};
                        bool isLastPass = (shift == (passes - 1) * 8) && std::is_signed_v<Type>;

                        for (size_t index = 0; index < sizeValue; ++index) {
                            uint8_t byte = (source[index] >> shift) & 0xFF;
                            if (isLastPass) byte ^= 128;
                            occurrences[byte]++;
                        }

                        uint32_t positions[256];
                        positions[0] = 0;
                        for (size_t index = 1; index < 256; ++index) {
                            positions[index] = positions[index - 1] + occurrences[index - 1];
                        }

                        for (size_t index = 0; index < sizeValue; ++index) {
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
                return IteratorPipeline<decltype(v)>(std::move(v));
            }

            inline auto iterate() const {
                auto v = std::views::all(*this);
                return IteratorPipeline<decltype(v)>(std::move(v));
            }
            #endif
        };
    }
}
