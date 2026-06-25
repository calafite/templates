#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>

namespace calafite {
    namespace ds {

        template<typename Type, typename Compare = std::less<Type>> 
        struct MonotonicStack {
            struct Element {
                Type value;
                size_t count;
            };

            core::FastVector<Element> stack;
            Compare compare;
            Type currentSum;

            MonotonicStack(Compare comp = Compare()) 
                : compare(std::move(comp)), currentSum(Type(0)) {}

            inline void push(const Type& value) {
                size_t count = 1;
                
                while (!stack.empty() && compare(stack.back().value, value)) {
                    currentSum = currentSum - stack.back().value * static_cast<Type>(stack.back().count);
                    count += stack.back().count;
                    stack.popBack();
                }
                
                stack.pushBack({value, count});
                currentSum = currentSum + value * static_cast<Type>(count);
            }

            inline void pop() {
                assert(!stack.empty());
                currentSum = currentSum - stack.back().value * static_cast<Type>(stack.back().count);
                stack.popBack();
            }

            inline const Element& top() const {
                assert(!stack.empty());
                return stack.back();
            }

            inline bool empty() const {
                return stack.empty();
            }

            inline size_t size() const {
                return stack.size();
            }

            inline void reserve(size_t capacity) {
                stack.reserve(capacity);
            }

            inline void clear() {
                stack.clear();
                currentSum = Type(0);
            }
        };

    }
}
