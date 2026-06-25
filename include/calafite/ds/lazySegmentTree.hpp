#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace calafite {
    namespace ds {

        template<typename Type, typename LazyType, typename CombineNodeOp, typename ApplyLazyOp, typename ComposeLazyOp>
        struct LazySegmentTree {
            size_t sizeValue;
            core::FastVector<Type> tree;
            core::FastVector<LazyType> lazy;
            core::FastVector<uint8_t> hasLazy;

            Type neutralNode;
            LazyType neutralLazy;
            CombineNodeOp combineNode;
            ApplyLazyOp applyLazy;
            ComposeLazyOp composeLazy;

            LazySegmentTree(size_t count, Type neutralNode, LazyType neutralLazy, 
                            CombineNodeOp combineNode, ApplyLazyOp applyLazy, ComposeLazyOp composeLazy)
                : sizeValue(count),
                  tree(count > 0 ? 4 * count : 0, neutralNode),
                  lazy(count > 0 ? 4 * count : 0, neutralLazy),
                  hasLazy(count > 0 ? 4 * count : 0, 0),
                  neutralNode(neutralNode),
                  neutralLazy(neutralLazy),
                  combineNode(std::move(combineNode)),
                  applyLazy(std::move(applyLazy)),
                  composeLazy(std::move(composeLazy)) {}

            LazySegmentTree(const core::FastVector<Type>& values, Type neutralNode, LazyType neutralLazy, 
                            CombineNodeOp combineNode, ApplyLazyOp applyLazy, ComposeLazyOp composeLazy)
                : sizeValue(values.size()),
                  tree(values.size() > 0 ? 4 * values.size() : 0, neutralNode),
                  lazy(values.size() > 0 ? 4 * values.size() : 0, neutralLazy),
                  hasLazy(values.size() > 0 ? 4 * values.size() : 0, 0),
                  neutralNode(neutralNode),
                  neutralLazy(neutralLazy),
                  combineNode(std::move(combineNode)),
                  applyLazy(std::move(applyLazy)),
                  composeLazy(std::move(composeLazy)) {
                if (sizeValue > 0) {
                    buildInternal(1, 0, sizeValue - 1, values);
                }
            }

            void update(size_t left, size_t right, const LazyType& value) {
                if (sizeValue == 0) return;
                assert(left <= right);
                assert(right < sizeValue);
                updateInternal(1, 0, sizeValue - 1, left, right, value);
            }

            Type query(size_t left, size_t right) {
                if (sizeValue == 0) return neutralNode;
                assert(left <= right);
                assert(right < sizeValue);
                return queryInternal(1, 0, sizeValue - 1, left, right);
            }

        private:
            void buildInternal(size_t node, size_t left, size_t right, const core::FastVector<Type>& values) {
                if (left == right) {
                    tree[node] = values[left];
                    return;
                }
                size_t mid = left + (right - left) / 2;
                buildInternal(node << 1, left, mid, values);
                buildInternal(node << 1 | 1, mid + 1, right, values);
                tree[node] = combineNode(tree[node << 1], tree[node << 1 | 1]);
            }

            inline void applyNode(size_t node, const LazyType& value) {
                tree[node] = applyLazy(value, tree[node]);
                if (hasLazy[node]) {
                    lazy[node] = composeLazy(value, lazy[node]);
                } else {
                    lazy[node] = value;
                    hasLazy[node] = 1;
                }
            }

            inline void push(size_t node) {
                if (hasLazy[node]) {
                    applyNode(node << 1, lazy[node]);
                    applyNode(node << 1 | 1, lazy[node]);
                    hasLazy[node] = 0;
                    lazy[node] = neutralLazy;
                }
            }

            void updateInternal(size_t node, size_t left, size_t right, size_t queryLeft, size_t queryRight, const LazyType& value) {
                if (queryLeft <= left && right <= queryRight) {
                    applyNode(node, value);
                    return;
                }
                push(node);
                size_t mid = left + (right - left) / 2;
                if (queryLeft <= mid) {
                    updateInternal(node << 1, left, mid, queryLeft, queryRight, value);
                }
                if (queryRight > mid) {
                    updateInternal(node << 1 | 1, mid + 1, right, queryLeft, queryRight, value);
                }
                tree[node] = combineNode(tree[node << 1], tree[node << 1 | 1]);
            }

            Type queryInternal(size_t node, size_t left, size_t right, size_t queryLeft, size_t queryRight) {
                if (queryLeft <= left && right <= queryRight) {
                    return tree[node];
                }
                push(node);
                size_t mid = left + (right - left) / 2;
                Type leftResult = neutralNode;
                Type rightResult = neutralNode;
                
                if (queryLeft <= mid) {
                    leftResult = queryInternal(node << 1, left, mid, queryLeft, queryRight);
                }
                if (queryRight > mid) {
                    rightResult = queryInternal(node << 1 | 1, mid + 1, right, queryLeft, queryRight);
                }
                
                return combineNode(leftResult, rightResult);
            }
        };

    }
}
