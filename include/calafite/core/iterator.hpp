#pragma once

#if defined(__has_include)
    #if __has_include(<version>)
        #include <version>
    #endif
#endif


#if defined(__cpp_lib_ranges)

namespace calafite {
    namespace core {
        template<typename Type> struct FastVector;
    }

    namespace core {

        template<std::ranges::view View>
        struct IteratorPipeline {
            View view;

            constexpr explicit IteratorPipeline(View v) : view(std::move(v)) {}

            // Lazy Operations

            template<typename Function>
            constexpr auto map(Function&& function) {
                auto next = view | std::views::transform(std::forward<Function>(function));
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            template<typename Predicate>
            constexpr auto filter(Predicate&& predicate) {
                auto next = view | std::views::filter(std::forward<Predicate>(predicate));
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            constexpr auto take(size_t n) {
                auto next = view | std::views::take(n);
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            constexpr auto drop(size_t n) {
                auto next = view | std::views::drop(n);
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            template<typename Predicate>
            constexpr auto take_while(Predicate&& predicate) {
                auto next = view | std::views::take_while(std::forward<Predicate>(predicate));
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            template<typename Predicate>
            constexpr auto drop_while(Predicate&& predicate) {
                auto next = view | std::views::drop_while(std::forward<Predicate>(predicate));
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            constexpr auto reverse() {
                auto next = view | std::views::reverse;
                return IteratorPipeline<decltype(next)>(std::move(next));
            }

            // Eager Operations
            template<template<typename...> typename Container = ::calafite::core::FastVector>
            constexpr auto collect() {
                using RangeRef = std::ranges::range_reference_t<View>;
                using RangeVal = std::remove_cvref_t<RangeRef>;
                
                Container<RangeVal> result;

                if constexpr (requires { result.reserve(std::size_t{}); }) {
                    if constexpr (std::ranges::sized_range<View>) {
                        result.reserve(std::ranges::size(view));
                    }
                }

                for (auto&& item : view) {
                    result.pushBack(std::forward<decltype(item)>(item));
                }

                return result;
            }

            template<typename Accumulator, typename Function>
            constexpr auto fold(Accumulator initial, Function&& function) {
                Accumulator result = std::move(initial);
                for (auto&& item : view) {
                    result = function(std::move(result), std::forward<decltype(item)>(item));
                }
                return result;
            }

            template<typename Function>
            constexpr void forEach(Function&& function) {
                for (auto&& item : view) {
                    function(std::forward<decltype(item)>(item));
                }
            }

            template<typename Predicate>
            constexpr bool any(Predicate&& predicate) {
                for (auto&& item : view) {
                    if (predicate(item)) return true;
                }
                return false;
            }

            template<typename Predicate>
            constexpr bool all(Predicate&& predicate) {
                for (auto&& item : view) {
                    if (!predicate(item)) return false;
                }
                return true;
            }

            constexpr size_t count() {
                if constexpr (std::ranges::sized_range<View>) {
                    return std::ranges::size(view);
                } else {
                    size_t total = 0;
                    for ([[maybe_unused]] auto&& item : view) {
                        ++total;
                    }
                    return total;
                }
            }
        };

    }
}

#endif 
