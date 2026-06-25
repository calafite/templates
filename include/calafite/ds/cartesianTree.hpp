#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>

namespace calafite {
    namespace ds {

        template<typename Type, typename Compare = std::less<Type>> 
        struct CartesianTree {
            static constexpr size_t nullNode = static_cast<size_t>(-1);

            size_t sizeValue;
            core::FastVector<size_t> leftChild;
            core::FastVector<size_t> rightChild;
            core::FastVector<size_t> parent;
            size_t root;
            Compare compare;

            CartesianTree() : sizeValue(0), root(nullNode) {}

            CartesianTree(const core::FastVector<Type>& values, Compare comp = Compare())
                : sizeValue(values.size()), 
                  leftChild(values.size(), nullNode), 
                  rightChild(values.size(), nullNode), 
                  parent(values.size(), nullNode), 
                  root(nullNode),
                  compare(std::move(comp)) {
                
                if (sizeValue == 0) return;

                core::FastVector<size_t> stack;
                stack.reserve(sizeValue);

                for (size_t index = 0; index < sizeValue; ++index) {
                    size_t lastPopped = nullNode;
                    
                    while (!stack.empty() && compare(values[index], values[stack.back()])) {
                        lastPopped = stack.back();
                        stack.popBack();
                    }
                    
                    if (lastPopped != nullNode) {
                        leftChild[index] = lastPopped;
                        parent[lastPopped] = index;
                    }
                    
                    if (!stack.empty()) {
                        rightChild[stack.back()] = index;
                        parent[index] = stack.back();
                    }
                    
                    stack.pushBack(index);
                }
                
                root = stack.front();
            }
        };

    }
}
